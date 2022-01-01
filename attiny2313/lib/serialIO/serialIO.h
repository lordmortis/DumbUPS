//
// Created by Brendan Ragan on 1/1/2022.
//

#ifndef ATTINY2313_SERIALIO_H
#define ATTINY2313_SERIALIO_H

#include "stdint.h"
#include "stdbool.h"

void Serial_Init(uint32_t baudRate);

void Serial_WriteString(char *data, uint8_t length);
bool Serial_WritePending();

void Serial_SetReadBuffer(char *buffer, uint8_t length);
void Serial_ResetReadBuffer();
bool Serial_ReadPending();
void Serial_LockRead();
void Serial_UnlockRead();

#endif //ATTINY2313_SERIALIO_H
