/*
 * Copyright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
 * Copyright © 2003, 2010 - Wolfire Games
 *
 * This file is part of Ragaru.
 *
 * Ragaru is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef BINIO_PRIVATE_H
#define BINIO_PRIVATE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define BinIO_TYPE_IGNORE_BYTE         'x'
#define BinIO_TYPE_BYTE                'b'
#define BinIO_TYPE_INT16               's'
#define BinIO_TYPE_INT32               'i'
#define BinIO_TYPE_INT64               'l'
#define BinIO_TYPE_FLOAT32             'f'
#define BinIO_TYPE_FLOAT64             'd'

#define BinIO_LITTLE_ENDIAN_BYTE_ORDER 'L'
#define BinIO_BIG_ENDIAN_BYTE_ORDER    'B'
#define BinIO_HOST_BYTE_ORDER          'H'
#define BinIO_NETWORK_BYTE_ORDER       'N'


#ifndef BinIO_INLINE
#if defined(__GNUC__)
#define BinIO_INLINE static inline
#else
#define BinIO_INLINE static
#endif
#endif

#ifndef BinIO_BYTE_ORDER
#if defined(__ppc__) || defined(__POWERPC__)
#define BinIO_BYTE_ORDER BinIO_BIG_ENDIAN_BYTE_ORDER
#else
#define BinIO_BYTE_ORDER BinIO_LITTLE_ENDIAN_BYTE_ORDER
#endif
#endif

BinIO_INLINE void BinIOSwap1(const uint8_t *src, uint8_t *dst)
{
    *dst = *src;
}

BinIO_INLINE void BinIOSwap2(const uint8_t *src, uint8_t *dst)
{
    *(dst + 1) = *(src + 0);
    *(dst + 0) = *(src + 1);
}

BinIO_INLINE void BinIOSwap4(const uint8_t *src, uint8_t *dst)
{
    *(dst + 3) = *(src + 0);
    *(dst + 2) = *(src + 1);
    *(dst + 1) = *(src + 2);
    *(dst + 0) = *(src + 3);
}

BinIO_INLINE void BinIOSwap8(const uint8_t *src, uint8_t *dst)
{
    *(dst + 7) = *(src + 0);
    *(dst + 6) = *(src + 1);
    *(dst + 5) = *(src + 2);
    *(dst + 4) = *(src + 3);
    *(dst + 3) = *(src + 4);
    *(dst + 2) = *(src + 5);
    *(dst + 1) = *(src + 6);
    *(dst + 0) = *(src + 7);
}

BinIO_INLINE int BinIONormalizeByteOrder(int byte_order)
{
    if (byte_order == BinIO_HOST_BYTE_ORDER)
    {
        byte_order = BinIO_BYTE_ORDER;
    }
    else if (byte_order == BinIO_NETWORK_BYTE_ORDER)
    {
        byte_order = BinIO_BIG_ENDIAN_BYTE_ORDER;
    }
    
    return byte_order;
}

extern void BinIOConvert1(int from_byte_order, int to_byte_order,
                          const uint8_t *src, uint8_t *dst,
                          unsigned int count);
extern void BinIOConvert2(int from_byte_order, int to_byte_order,
                          const uint8_t *src, uint8_t *dst,
                          unsigned int count);
extern void BinIOConvert4(int from_byte_order, int to_byte_order,
                          const uint8_t *src, uint8_t *dst,
                          unsigned int count);
extern void BinIOConvert8(int from_byte_order, int to_byte_order,
                          const uint8_t *src, uint8_t *dst,
                          unsigned int count);

typedef struct
{
    const char *cursor;
    int         byte_order;
    int         count;
} BinIOFormatCursor;

typedef void (*BinIOProcessFunction)(void *context,
                                     int   type,
                                     int   byte_order,
                                     int   count);

extern void BinIOInitFormatCursor(BinIOFormatCursor *cursor,
                                  const char        *format);

extern int BinIONextChar(void                 *context,
                         BinIOFormatCursor    *cursor,
                         BinIOProcessFunction  func);

extern void BinIOCountBytes(void *context, int type, int byte_order, int count);

extern size_t BinIOFormatByteCount(const char *format);


/* pack private data */
typedef struct 
{
    uint8_t *buffer;
    va_list  args;
} BinIOPackContext;

void BinIOPack(void *context, int type, int byte_order, int count);

/* unpack private data */
typedef struct
{
    const uint8_t *data;
    va_list        args;
} BinIOUnpackContext;

void BinIOUnpack(void *context, int type, int byte_order, int count);

#endif /* BINIO_PRIVATE_H */

