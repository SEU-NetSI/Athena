#ifndef _LIBDW3000_SPI_H_
#define _LIBDW3000_SPI_H_

#include <stddef.h>

void spiRead(const void *header, size_t headerLength, void *data, size_t dataLength);

void spiWrite(const void *header, size_t headerLength, const void *data, size_t dataLength);

#endif
