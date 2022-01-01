//
// Created by Brendan Ragan on 1/1/2022.
//

#include "serialIO.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)
#define TOGGLE_BIT(val, bitIndex) val ^= (1 << bitIndex)
#define BIT_IS_SET(val, bitIndex) (val & (1 << bitIndex))

//STATUS BITS
#define WRITE_PENDING    0

char *writeBuffer;
uint8_t writeBufferIndex;
uint8_t writeBufferLength;
uint8_t status;

void SetBaudRate(uint32_t baudRate) {
    uint16_t regValue = (F_CPU / (baudRate * 16UL)) - 1;
    if (regValue > 7777 || regValue < 1) {
        regValue = (F_CPU / (9600 * 16UL)) - 1;
    }
    UBRRH = (uint8_t)regValue >> 8;
    UBRRL = (uint8_t)regValue & 0xFF;
}

void Serial_Init(uint32_t baudRate) {
    status = 0;
    writeBufferIndex = 0;
    writeBufferLength = 0;
    SetBaudRate(baudRate);
    UCSRA= 0x00;                      // Clear the UASRT status register
    UCSRB= (1<<RXEN) | (1<<TXEN) | (1<<TXCIE);     // Enable Receiver and Transmitter and Transmit Interrupt
    UCSRC= (1<<UCSZ1) | (1<<UCSZ0);   // 8-bit
}

void Serial_WriteString(char *data, uint8_t length) {
    status = 1;
    writeBuffer = data;
    writeBufferIndex = 0;
    writeBufferLength = length;
    if (!BIT_IS_SET(UCSRA, TXC)) {
        UDR = writeBuffer[writeBufferIndex];
        writeBufferIndex++;
    }
}

bool Serial_WritePending() {
    return (status != 0);
}

ISR(USART_TX_vect) {
    if (status == 0) return;
    UDR = writeBuffer[writeBufferIndex];
    writeBufferIndex++;
    if (writeBufferIndex < writeBufferLength) {
        SET_BIT(status, WRITE_PENDING);
    } else {
        CLEAR_BIT(status, WRITE_PENDING);
    }
}