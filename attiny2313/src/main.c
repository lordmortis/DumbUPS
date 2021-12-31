#include <avr/io.h>

//10 Mhz CPU
#define 	F_CPU   10000000UL

#include <util/delay.h>

int main(void)
{
    DDRB = 1;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(1)
    {
        _delay_ms(500);
        PORTB ^= 1;
    }
#pragma clang diagnostic pop

    return 0;
}