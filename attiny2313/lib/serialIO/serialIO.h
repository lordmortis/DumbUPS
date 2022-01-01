//
// Created by Brendan Ragan on 1/1/2022.
//

#ifndef ATTINY2313_SERIALIO_H
#define ATTINY2313_SERIALIO_H

#include "stdint.h"

void Serial_Init(uint32_t baudRate);
void Serial_WriteChar(char data);

#endif //ATTINY2313_SERIALIO_H
