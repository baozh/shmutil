/*
 * common.h
 *
 *      Created on: 2017.5.10
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#ifndef SHMUTIL_COMMON_H
#define SHMUTIL_COMMON_H

#if (BYTE_ORDER == LITTLE_ENDIAN)
#define IS_LITTLE_ENDIAN true
#else
#define IS_LITTLE_ENDIAN false
#endif

#define NEED_MD5_VALUE_LEN 1024
#define MD5_STR_LEN        16

#include <stdlib.h>
#include <string.h>

inline void encodeValueLen(char* buf, uint32_t len)
{
    if (IS_LITTLE_ENDIAN)
    {
        memcpy(buf, &len, sizeof(uint32_t));
    }
    else
    {
        for (size_t i = 0; i < sizeof(uint32_t); ++i)
            *(buf + i) = (len >> i * 8) & 0xff;
    }
}

inline uint32_t decodeValueLen(const char* buf)
{
    uint32_t len = 0;
    if (IS_LITTLE_ENDIAN)
        memcpy(&len, buf, sizeof(uint32_t));
    else
    {
        for (size_t i = 0; i < sizeof(uint32_t); ++i)\
            len += (*reinterpret_cast<const uint8_t*>(buf + i) << i * 8);
    }
    return len;
}

#endif //SHMUTIL_COMMON_H
