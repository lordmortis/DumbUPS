#include <avr/io.h>
#include <util/delay_basic.h>

#include "serialIO.h"

int main(void)
{
    Serial_Init(4800);
    uint8_t led_on = 0;
    DDRB = 1;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(1)
    {
        for(int i = 0; i < 250; i++) {
            _delay_loop_2(250);
        }

        if (led_on == 0) {
            led_on = 1;
        } else {
            led_on = 0;
        }

        PORTB ^= led_on;

        if (led_on > 0) {
            Serial_WriteChar('O');
            Serial_WriteChar('N');
        } else {
            Serial_WriteChar('O');
            Serial_WriteChar('F');
            Serial_WriteChar('F');
        }
        Serial_WriteChar('\r');
        Serial_WriteChar('\n');
    }
#pragma clang diagnostic pop

    return 0;
}