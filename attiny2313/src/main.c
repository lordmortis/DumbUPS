#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "serialIO.h"

#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)
#define BIT_IS_SET(val, bitIndex) (val & (1 << bitIndex))
#define TOGGLE_BIT(val, bitIndex) val ^= (1 << bitIndex)

#define POWER_OFF_TIME_S 30
#define POWER_ON_TIME_S 30
#define STATUS_CHANGE_UPDATE_WAIT_IN_MS 100
#define STATUS_UPDATE_INTERVAL_IN_S 30
#define MAX_WATCHDOG_TIME_IN_S 30

#define POWER_UP_FLASH_DELAY_MS 250
#define POWER_DOWN_FLASH_DELAY_MS 125
#define MAINS_DOWN_FLASH_DELAY_MS 50

#define POWER_STATE_BIT PA0 // Power state is Port A 0 (Output)
#define STATUS_LED_BIT PA1 // Status LED is Port A 1 (Output)

#define BATTERY_POWER_BIT PB0 // Battery Power is Port B 0 (Input)
#define MAINS_DOWN_BIT PB1 // Mains Down is Port B 1 (Input)
#define LOW_BATTERY_BIT PB2 // Low Battery is Port B 2 (Input)
#define MAINS_UP_BIT PB3 // Mains up is Port B 3 (input)

const char status_text[] PROGMEM = "STATUS-";
char const battery_text[] PROGMEM = "BATT:";
char const battery_low_text[] PROGMEM = ",BATTLOW:";
char const mains_down_text[] PROGMEM = ",MAINSDWN:";
char const mains_up_text[] PROGMEM = ",MAINSUP:";
char const yes_text[] PROGMEM = "Yes";
char const no_text[] PROGMEM = "No";
const char power_up_status_text[] PROGMEM = "POWERING UP";
const char watchdog_expired_text[] PROGMEM = "Watchdog Expired, shutting down";
char const new_line[] PROGMEM = "\r\n";
char const power_off_command_text[] PROGMEM = "shut";
char const no_power_off_command_text[] PROGMEM = "noshut";
char const ping_command_text[] PROGMEM = "ping";
char const ping_response_text[] PROGMEM = "PONG";
char const unknown_command_text[] PROGMEM = "unknown command: ";

uint16_t countMS;
uint16_t countPowerChangeFinalizeMS;
uint8_t countStatusChangeDelayMS;
uint8_t flashDelayMS;
uint8_t countStatusUpdateS;
uint8_t countStatusWatchdogS;
uint8_t countPowerChangeS;

bool sendStatusUpdate;
bool checkInputs;

char outputBuffer[60];
char inputBuffer[10];

bool upsOn;
bool poweringOff;
bool poweringOn;

bool batteryPower;
bool mainsDown;
bool mainsUp;
bool lowBattery;

bool write_status() {
    if (Serial_WritePending()) return false;
    uint8_t bufIndex = 0;
    strcpy_P(outputBuffer, status_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, battery_text);
    bufIndex = strlen(outputBuffer);
    if (batteryPower) {
        strcpy_P(outputBuffer + bufIndex, yes_text);
    } else {
        strcpy_P(outputBuffer + bufIndex, no_text);
    }
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, battery_low_text);
    bufIndex = strlen(outputBuffer);
    if (lowBattery) {
        strcpy_P(outputBuffer + bufIndex, yes_text);
    } else {
        strcpy_P(outputBuffer + bufIndex, no_text);
    }
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, mains_down_text);
    bufIndex = strlen(outputBuffer);
    if (mainsDown) {
        strcpy_P(outputBuffer + bufIndex, yes_text);
    } else {
        strcpy_P(outputBuffer + bufIndex, no_text);
    }
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, mains_up_text);
    bufIndex = strlen(outputBuffer);
    if (mainsUp) {
        strcpy_P(outputBuffer + bufIndex, yes_text);
    } else {
        strcpy_P(outputBuffer + bufIndex, no_text);
    }
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, new_line);
    Serial_WriteString(outputBuffer, strlen(outputBuffer));
    return true;
}

void check_command() {
    uint8_t length = strlen(inputBuffer);
    if (length == 0) return; // Not enough data in buffer
    if (inputBuffer[length - 1] != '\n' && inputBuffer[length - 1] != '\r') return; //not a proper command;
    Serial_ResetReadBuffer();
    for(int i = 0; i < length; i++) {
        if (inputBuffer[i] != '\n' && inputBuffer[i] != '\r') continue;
        inputBuffer[i] = '\0';
    }

    if (strcasecmp_P(inputBuffer, power_off_command_text) == 0) {
        poweringOff = true;
    } else if (strcasecmp_P(inputBuffer, no_power_off_command_text) == 0) {
        poweringOff = false;
    } else if (strcasecmp_P(inputBuffer, ping_command_text) == 0) {
        countStatusWatchdogS = 0;
        strcpy_P(outputBuffer, ping_response_text);
        strcpy_P(outputBuffer + strlen(outputBuffer), new_line);
        Serial_WriteString(outputBuffer, strlen(outputBuffer));
    } else {
        strcpy_P(outputBuffer, unknown_command_text);
        strcpy(outputBuffer + strlen(outputBuffer), inputBuffer);
        strcpy_P(outputBuffer + strlen(outputBuffer), new_line);
        Serial_WriteString(outputBuffer, strlen(outputBuffer));
    }
}

// This timer is configured to pulse every 1ms
ISR(TIMER0_OVF_vect) {
    countMS++;
    if (poweringOn || poweringOff || mainsDown) {
        flashDelayMS++;
    }

    //This is the power change low set timer system.
    if (countPowerChangeFinalizeMS > 0) {
        countPowerChangeFinalizeMS--;
        if (countPowerChangeFinalizeMS == 0) {
            PORTA = 0; // This clears the LED status _and_ drops the power state
        }
    }

    if (countStatusChangeDelayMS > 0) {
        countStatusChangeDelayMS--;
        if (countStatusChangeDelayMS == 0) {
            sendStatusUpdate = true;
            countStatusChangeDelayMS = 0;
        }
    }

    if (countMS > 999) {
        countMS = 0;
        countStatusUpdateS++;
        countStatusWatchdogS++;

        if (poweringOff || poweringOn) countPowerChangeS++;
    }

    if (countStatusUpdateS >= STATUS_UPDATE_INTERVAL_IN_S) {
        countStatusChangeDelayMS = 0;
        sendStatusUpdate = true;
    }

    checkInputs = true;
    TCNT0 = 192;
}

bool updateInputs() {
    bool stateDirty = false;
    uint8_t state = PINB;
    bool temp = BIT_IS_SET(state, BATTERY_POWER_BIT) > 0;
    if (temp != batteryPower) {
        batteryPower = temp;
        stateDirty = true;
    }

    //These are inverted due to the open collector design.
    temp = BIT_IS_SET(state, MAINS_DOWN_BIT) == 0;
    if (temp != mainsDown) {
        mainsDown = temp;
        stateDirty = true;
    }

    temp = BIT_IS_SET(state, MAINS_UP_BIT) == 0;
    if (temp != mainsUp) {
        mainsUp = temp;
        stateDirty = true;
    }

    temp = BIT_IS_SET(state, LOW_BATTERY_BIT) == 0;
    if (temp != lowBattery) {
        lowBattery = temp;
        stateDirty = true;
    }

    return stateDirty;
}

int main(void)
{
    Serial_Init(38400);
    Serial_SetReadBuffer(inputBuffer, 10, true);
    countMS = 0;
    upsOn = true;
    poweringOff = false;
    DDRA = (1<<DDRA0) | (1<<DDRA1); // Output pins A0 and A1
    PORTA = 0;
    DDRB = 0; // Input pins on B0-3, may as well set them all to input
    PORTB = 0; // Disable pullups

    _NOP();

    updateInputs();
    write_status();
    countStatusUpdateS = 0;
    countStatusWatchdogS = 0;

    TCNT0 = 192;
    TCCR0A = 0x00;
    TCCR0B = (1<<CS01) | (1<<CS00);
    TIMSK = (1 << TOIE0);
    sei();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(1)
    {
        if (sendStatusUpdate) {
            if (write_status()) {
                countStatusUpdateS = 0;
                sendStatusUpdate = false;
            }
            continue;
        }

        if (checkInputs) {
            if (Serial_ReadPending() && !Serial_WritePending()) {
                check_command();
                checkInputs = false;
                continue;
            }

            if (updateInputs()) countStatusChangeDelayMS = STATUS_CHANGE_UPDATE_WAIT_IN_MS;

            checkInputs = false;
        }

        if (countStatusWatchdogS > MAX_WATCHDOG_TIME_IN_S) {
            strcpy_P(outputBuffer, watchdog_expired_text);
            strcpy_P(outputBuffer + strlen(outputBuffer), new_line);
            Serial_WriteString(outputBuffer, strlen(outputBuffer));
            poweringOff = true;
            countPowerChangeS = 0;
            countStatusWatchdogS = 0;
            continue;
        }

        if (poweringOff) {
            if (mainsUp) {
                poweringOff = false;
                CLEAR_BIT(PORTA, STATUS_LED_BIT);
                continue;
            }

            if (flashDelayMS >= POWER_DOWN_FLASH_DELAY_MS) {
                TOGGLE_BIT(PORTA, STATUS_LED_BIT);
                flashDelayMS = 0;
            }

            if (countPowerChangeS > POWER_OFF_TIME_S) {
                //We need to go high and then low after 500 MS. Here we set it high,
                //and then start a counter for the ISR timer routine to set low
                SET_BIT(PORTA, POWER_STATE_BIT);
                countPowerChangeFinalizeMS = 500;
                countPowerChangeS = 0;
                poweringOff = false;
                upsOn = false;
                continue;
            }
        }

        if (!upsOn && mainsUp) {
            if (!poweringOn) {
                poweringOn = true;
                countPowerChangeS = 0;
            } else {
                if (flashDelayMS >= POWER_UP_FLASH_DELAY_MS) {
                    TOGGLE_BIT(PORTA, STATUS_LED_BIT);
                    flashDelayMS = 0;
                }
                if (countPowerChangeS >= POWER_ON_TIME_S) {
                    if (mainsUp && !upsOn) {
                        strcpy_P(outputBuffer, power_up_status_text);
                        strcpy_P(outputBuffer + strlen(outputBuffer), new_line);
                        Serial_WriteString(outputBuffer, strlen(outputBuffer));
                        //We need to go high and then low after 500 MS. Here we set it high,
                        //and then start a counter for the ISR timer routine to set low
                        SET_BIT(PORTA, POWER_STATE_BIT);
                        poweringOff = false;
                        poweringOn = false;
                        upsOn = true;
                        countPowerChangeFinalizeMS = 500;
                    }
                }
                continue;
            }

            if (mainsDown && flashDelayMS >= MAINS_DOWN_FLASH_DELAY_MS) {
                TOGGLE_BIT(PORTA, STATUS_LED_BIT);
                flashDelayMS = 0;
            }
        }
    }
#pragma clang diagnostic pop

    return 0;
}