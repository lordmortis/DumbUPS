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
#define READ_PENDING    1
#define WRITE_PENDING   0

char *writeBuffer;
uint8_t writeBufferIndex;
uint8_t writeBufferLength;

char *readBuffer;
uint8_t readBufferIndex;
uint8_t readBufferLength;

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
    SetBaudRate(baudRate);
    readBufferIndex = 0;
    readBufferLength = 0;
    UCSRA= 0x00;                      // Clear the UASRT status register
    UCSRB= (1<<RXEN) | (1<<TXEN) | (1<< RXCIE);     // Enable Receiver and Transmitter plus Interrupts
    UCSRC= (1<<UCSZ1) | (1<<UCSZ0);   // 8-bit
}

void Serial_SetReadBuffer(char *buffer, uint8_t length) {
    readBuffer = buffer;
    readBufferLength = length;
}

void Serial_WriteString(char *data, uint8_t length) {
    CLEAR_BIT(UCSRB, RXCIE);
    SET_BIT(UCSRB, UDRIE);
    SET_BIT(status, WRITE_PENDING);
    writeBuffer = data;
    writeBufferIndex = 0;
    writeBufferLength = length;
    if (!BIT_IS_SET(UCSRA, UDRE)) {
        TXB = writeBuffer[writeBufferIndex];
        writeBufferIndex++;
    }
}

bool Serial_WritePending() {
    return BIT_IS_SET(status, WRITE_PENDING);
}

bool Serial_ReadPending() {
    return BIT_IS_SET(status, READ_PENDING);
}

void Serial_ResetReadBuffer() {
    CLEAR_BIT(status, READ_PENDING);
    readBufferIndex = 0;
}

ISR(USART_UDRE_vect) {
    if (!BIT_IS_SET(status, WRITE_PENDING)) return;
    TXB = writeBuffer[writeBufferIndex];
    writeBufferIndex++;
    if (writeBufferIndex < writeBufferLength) {
        SET_BIT(status, WRITE_PENDING);
    } else {
        CLEAR_BIT(status, WRITE_PENDING);
        SET_BIT(UCSRB, RXCIE);
        CLEAR_BIT(UCSRB, UDRE);
    }
}

ISR(USART_RX_vect) {
    char temp = RXB;
    if (readBufferIndex >= (readBufferLength - 1)) return;
    SET_BIT(status, READ_PENDING);
    readBuffer[readBufferIndex] = temp;
    readBufferIndex++;
    readBuffer[readBufferIndex] = '\0';
}