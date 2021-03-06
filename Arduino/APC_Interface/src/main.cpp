#include "Arduino.h"

/*
  APC Dumb UPS Interface
*/

#define BATTERY_POWER 8
#define MAINS_DOWN 10
#define LOW_BATTERY 9
#define MAINS_UP 11

#define POWER_STATE 12

#define POWER_OFF_TIME 30 * 1000
#define POWER_UP_DELAY 30 * 1000
#define STATUS_CHANGE_UPDATE_WAIT 500
#define STATUS_UPDATE_TIME_MAX 30 * 1000
#define MAX_WATCHDOG_TIME 30 * 1000

unsigned long power_off_start_time;
unsigned long power_on_wait_start_time;
unsigned long last_status_update_time;
unsigned long last_watchdog_message_time;

bool battery_power;
bool mains_down;
bool low_battery;
bool mains_up;
bool powering_off;
bool powering_on;

bool ups_on = true;

bool pending_status_update = false;

#define SERIAL_BUFFER_LENGTH 20

char serialData[SERIAL_BUFFER_LENGTH]; // Allocate some space for the string
byte serialDataIndex = 0; // Index into array; where to store the character

void write_bool(bool value) {
    if (value)
        Serial.write("Yes");
    else
        Serial.write("No");
}

void write_status(unsigned long currentTime) {
    last_status_update_time = currentTime;
    Serial.write("STATUS-");
    Serial.write("BATT:");
    write_bool(battery_power);
    Serial.write(",BATTLOW:");
    write_bool(low_battery);
    Serial.write(",MAINSDWN:");
    write_bool(mains_down);
    Serial.write(",MAINSUP:");
    write_bool(mains_up);
    Serial.write("\n");
}

bool check_digital_inputs() {
    bool state_dirty = false;
    bool temp = digitalRead(BATTERY_POWER) == HIGH;
    if (battery_power != temp) {
        battery_power = temp;
        state_dirty = true;
    }
    temp = digitalRead(MAINS_DOWN) == LOW;
    if (mains_down != temp) {
        mains_down = temp;
        state_dirty = true;
    }
    temp = digitalRead(LOW_BATTERY) == LOW;
    if (low_battery != temp) {
        low_battery = temp;
        state_dirty = true;
    }
    temp = digitalRead(MAINS_UP) == LOW;
    if (mains_up != temp) {
        mains_up = temp;
        state_dirty = true;
    }

    return state_dirty;
}

void set_power_off(bool value) {
    powering_off = value;
    if (powering_off) {
        power_off_start_time = millis();
    }
}

void resume_ac_power() {
    if (mains_up && !ups_on) {
        Serial.write("POWERING_UP\n");
        digitalWrite(POWER_STATE, HIGH);
        powering_off = false;
        ups_on = true;
        delay(500);
        digitalWrite(POWER_STATE, LOW);
    }
}

void check_for_commands(unsigned long loopTime) {
    bool check_command = false;
    if (Serial.available() > 0) {
        char input = Serial.read();
        if (input == '\n' || input == '\r') {
            if (serialDataIndex > 0) check_command = true;
        } else {
            serialData[serialDataIndex] = input;
            serialDataIndex++;
            serialData[serialDataIndex] = '\0';
            if (serialDataIndex > SERIAL_BUFFER_LENGTH - 1) {
                check_command = true;
            }
        }
    }

    if (check_command) {
        if (strcasecmp(serialData, "shut") == 0) {
            last_watchdog_message_time = loopTime;
            set_power_off(true);
        } else if (strcasecmp(serialData, "noshut") == 0) {
            last_watchdog_message_time = loopTime;
            set_power_off(false);
        } else if (strcasecmp(serialData, "ping") == 0) {
            Serial.write("PONG\n");
            last_watchdog_message_time = loopTime;
        } else {
            Serial.write("Unrecognized command: '");
            Serial.write(serialData);
            Serial.write("'\n");
        }

        serialDataIndex = 0;
        serialData[serialDataIndex] = '\0';
    }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(POWER_STATE, OUTPUT);

    pinMode(BATTERY_POWER, INPUT);
    pinMode(MAINS_DOWN, INPUT);
    pinMode(LOW_BATTERY, INPUT);
    pinMode(LOW_BATTERY, INPUT);
    pinMode(LOW_BATTERY, INPUT);

    digitalWrite(POWER_STATE, LOW);
    ups_on = true;
    check_digital_inputs();
    Serial.begin(9600);
    write_status(millis());
    last_watchdog_message_time = millis();
}

// the loop function runs over and over again forever
void loop() {
    unsigned long loopTime = millis();
    bool last_status_updated_exceeded = loopTime - last_status_update_time > STATUS_UPDATE_TIME_MAX;
    if (check_digital_inputs()) {
        pending_status_update = true;
        last_status_update_time = loopTime;
    } else if (last_status_updated_exceeded) {
        write_status(loopTime);
        pending_status_update = false;
    }

    if (pending_status_update) {
        if (loopTime - last_status_update_time > STATUS_CHANGE_UPDATE_WAIT) {
            pending_status_update = false;
            write_status(loopTime);
        }
    }

    check_for_commands(loopTime);
    if (!powering_off && loopTime - last_watchdog_message_time > MAX_WATCHDOG_TIME) {
        Serial.write("Watchdog Expired, shutting down\n");
        powering_off = true;
    }

    if (powering_off) {
        if (mains_up) {
            set_power_off(false);
            return;
        }

        if (loopTime - power_off_start_time > POWER_OFF_TIME) {
            digitalWrite(POWER_STATE, HIGH);
            powering_off = false;
            ups_on = false;
            delay(500);
            digitalWrite(POWER_STATE, LOW);
        }
    }

    if (!ups_on && mains_up) {
        if (!powering_on) {
            powering_on = true;
            power_on_wait_start_time = loopTime;
        } else {
            if (loopTime - power_on_wait_start_time > POWER_UP_DELAY) {
                resume_ac_power();
            }
        }
    }
}
