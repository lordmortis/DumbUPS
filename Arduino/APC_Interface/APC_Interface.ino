#include "Arduino.h"

/*
  APC Dumb UPS Interface
*/

#define BATTERY_POWER 3
#define MAINS_DOWN 4
#define LOW_BATTERY 5
#define MAINS_UP 6

#define POWER_STATE 12

#define POWER_OFF_TIME 5 * 1000

unsigned long power_off_start_time;
bool battery_power;
bool mains_down;
bool low_battery;
bool mains_up;
bool powering_off;

bool ups_on = true;

#define SERIAL_BUFFER_LENGTH 20

char serialData[SERIAL_BUFFER_LENGTH]; // Allocate some space for the string
byte serialDataIndex = 0; // Index into array; where to store the character

void write_bool(bool value) {
    if (value)
        Serial.write("Yes");
    else
        Serial.write("No");
}

void write_status() {
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
        digitalWrite(POWER_STATE, HIGH);
        powering_off = false;
        ups_on = true;
    }
}

void check_for_commands() {
    bool check_command = false;
    if (Serial.available() > 0) {
        char input = Serial.read();
        if (input == '\n' || input == '\r') {
            if (serialDataIndex > 0) check_command = true;
        } else {
            serialData[serialDataIndex] = input;
            serialDataIndex++;
            serialData[serialDataIndex] = '\0';
            if (serialDataIndex >= SERIAL_BUFFER_LENGTH) {
                check_command = true;
            }
        }
    }

    if (check_command) {
        if (strcasecmp(serialData, "shut") == 0) {
            set_power_off(true);;
        } else if (strcasecmp(serialData, "noshut") == 0) {
            set_power_off(false);
        } else if (strcasecmp(serialData, "acresume") == 0) {
            resume_ac_power();
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

    digitalWrite(POWER_STATE, HIGH);
    ups_on = true;
    check_digital_inputs();
    Serial.begin(9600);
    Serial.write("Device Starting\n");
    write_status();
}

// the loop function runs over and over again forever
void loop() {
    if (check_digital_inputs()) write_status();
    check_for_commands();
    if (powering_off) {
        if (mains_up) {
            set_power_off(false);
            return;
        }

        unsigned long timer = millis() - power_off_start_time;
        if (millis() - power_off_start_time > POWER_OFF_TIME) {
            Serial.write("Powering Off.\n");
            digitalWrite(POWER_STATE, LOW);
            powering_off = false;
            ups_on = false;
        }
    }
}
