//
// Created by Brendan Ragan on 1/1/2022.
//

#include "serialIO.h"
#include <avr/io.h>

void SetBaudRate(uint32_t baudRate) {
    uint16_t regValue = (F_CPU / (baudRate * 16UL)) - 1;
    if (regValue > 7777 || regValue < 1) {
        regValue = (F_CPU / (9600 * 16UL)) - 1;
    }
    UBRRH = (uint8_t)regValue >> 8;
    UBRRL = (uint8_t)regValue & 0xFF;
}

void Serial_Init(uint32_t baudRate) {
    SetBaudRate(baudRate);
    UCSRA= 0x00;                      // Clear the UASRT status register
    UCSRB= (1<<RXEN) | (1<<TXEN);     // Enable Receiver and Transmitter
    UCSRC= (1<<UCSZ1) | (1<<UCSZ0);   // 8-bit
}

void Serial_WriteChar(char data) {
    while (!( UCSRA & (1<<UDRE)));
    UDR = data;
}