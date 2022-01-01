#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "serialIO.h"

char const status_text[] PROGMEM = "STATUS-";
char const battery_text[] PROGMEM = "BATT:";
char const battery_low_text[] PROGMEM = "BATTLOW";
char const mains_down_text[] PROGMEM = "MAINSDWN:";
char const mains_up_text[] PROGMEM = "MAINSUP:";
char const on_text[] PROGMEM = "LED on";
char const off_text[] PROGMEM = "No LED";
char const new_line[] PROGMEM = "\r\n";

uint8_t led_count;
uint8_t led_on;

char outputBuffer[60];
char inputBuffer[10];

// This timer is configured to pulse every 1ms
ISR(TIMER0_OVF_vect) {
    led_count++;

    if (led_count > 200) {
        if (led_on == 0) {
            led_on = 1;
        } else {
            led_on = 0;
        }

        PORTB ^= led_on;
        if (!Serial_WritePending()) {
            if (led_on > 0) {
                strcpy_P(outputBuffer, on_text);
            } else {
                strcpy_P(outputBuffer, off_text);
            }

            strcpy_P((outputBuffer + strlen(outputBuffer)), new_line);
            Serial_WriteString(outputBuffer, strlen(outputBuffer));
        }
        led_count = 0;
    }

    TCNT0 = 192;
}

int main(void)
{
    Serial_Init(38400);
    led_on = 0;
    led_count = 0;
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