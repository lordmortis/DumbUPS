#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "serialIO.h"

// Power state is Port A 0 (Output)

// Battery Power is Port B 0 (Input)
// Mains Down is Port B 1 (Input)
// Low Battery is Port B 2 (Input)
// Mains up is Port B 3 (input)

#define POWER_OFF_TIME_S = 30
#define POWER_UP_DELAY_S = 30
#define STATUS_CHANGE_UPDATE_WAIT_IN_MS 500
#define STATUS_UPDATE_TIME_MAX_IN_S 30
#define STATUS_UPDATE_INTERVAL_IN_S 5

char const status_text[] PROGMEM = "STATUS-";
char const battery_text[] PROGMEM = "BATT:";
char const battery_low_text[] PROGMEM = ",BATTLOW:";
char const mains_down_text[] PROGMEM = ",MAINSDWN:";
char const mains_up_text[] PROGMEM = ",MAINSUP:";
char const yes_text[] PROGMEM = "Yes";
char const no_text[] PROGMEM = "No";
char const new_line[] PROGMEM = "\r\n";
char const power_off_command_text[] PROGMEM = "shut";
char const no_power_off_command_text[] PROGMEM = "noshut";
char const ping_command_text[] PROGMEM = "ping";
char const unknown_command_text[] PROGMEM = "unknown command";

uint16_t ms_count;
uint8_t status_update_s_count;

char outputBuffer[60];
char inputBuffer[10];

bool write_status() {
    if (Serial_WritePending()) return false;
    uint8_t bufIndex = 0;
    strcpy_P(outputBuffer, status_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, battery_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, yes_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, battery_low_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, yes_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, mains_down_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, yes_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, mains_up_text);
    bufIndex = strlen(outputBuffer);
    strcpy_P(outputBuffer + bufIndex, yes_text);
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
        //TODO: handle power off
        strcpy_P(outputBuffer, power_off_command_text);
    } else if (strcasecmp_P(inputBuffer, no_power_off_command_text) == 0) {
        //TODO: handle power on
        strcpy_P(outputBuffer, no_power_off_command_text);
    } else if (strcasecmp_P(inputBuffer, ping_command_text) == 0) {
        status_update_s_count = STATUS_UPDATE_INTERVAL_IN_S;
        return;
    } else {
        strcpy_P(outputBuffer, unknown_command_text);
    }

    strcpy_P(outputBuffer + strlen(outputBuffer), new_line);
    Serial_WriteString(outputBuffer, strlen(outputBuffer));
}

// This timer is configured to pulse every 1ms
ISR(TIMER0_OVF_vect) {
    ms_count++;
    if (ms_count > 999) {
        ms_count = 0;
        status_update_s_count++;
    }

    if (status_update_s_count >= STATUS_UPDATE_INTERVAL_IN_S) {
        if (write_status()) status_update_s_count = 0;
    }

    if (Serial_ReadPending() && !Serial_WritePending()) {
        check_command();
    }

    TCNT0 = 192;
}

int main(void)
{
    Serial_Init(38400);
    Serial_SetReadBuffer(inputBuffer, 10);
    ms_count = 0;
    status_update_s_count = 0;
    DDRB = 1;

    TCNT0 = 192;
    TCCR0A = 0x00;
    TCCR0B = (1<<CS01) | (1<<CS00);
    TIMSK = (1 << TOIE0);
    sei();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(1)
    {

    }
#pragma clang diagnostic pop

    return 0;
}