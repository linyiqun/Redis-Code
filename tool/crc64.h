#ifndef CRC64_H
#define CRC64_H

#include <stdint.h>

/* Crc64循环冗余运算算法，crc:基础值0，s：传入的内容，l:内容长度 */
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

#endif
