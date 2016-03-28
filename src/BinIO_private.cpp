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


#include "BinIO_private.h"
#include <string.h>


void
BinIOPack(void *context, int type, int byte_order, int count) {
    BinIOPackContext *ctx = (BinIOPackContext *) context;
    if (count == -1) {
        switch (type) {
            case BinIO_TYPE_IGNORE_BYTE:
                {
                    ctx->buffer += 1;
                }
                break;
            case BinIO_TYPE_BYTE:
                {
                    uint8_t value = va_arg(ctx->args, int);
                    BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 1;
                }
                break;
            case BinIO_TYPE_INT16:
                {
                    uint16_t value = va_arg(ctx->args, int);
                    BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 2;
                }
                break;
            case BinIO_TYPE_INT32:
                {
                    int value = va_arg(ctx->args, int);
                    BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 4;
                }
                break;
            case BinIO_TYPE_INT64:
                {
                    uint64_t value = va_arg(ctx->args, uint64_t);
                    BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 8;
                }
                break;
            case BinIO_TYPE_FLOAT32:
                {
                    float value = (float) va_arg(ctx->args, double);
                    BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 4;
                }
                break;
            case BinIO_TYPE_FLOAT64:
                {
                    double value = va_arg(ctx->args, double);
                    BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 8;
                }
                break;
        }
    }
    else
    {
        switch (type) {
            case BinIO_TYPE_IGNORE_BYTE:                                                                                                     ctx->buffer += 1 * count; break;
            case BinIO_TYPE_BYTE:        BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 1 * count; break;
            case BinIO_TYPE_INT16:       BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 2 * count; break;
            case BinIO_TYPE_INT32:       BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 4 * count; break;
            case BinIO_TYPE_INT64:       BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 8 * count; break;
            case BinIO_TYPE_FLOAT32:     BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 4 * count; break;
            case BinIO_TYPE_FLOAT64:     BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 8 * count; break;
        }
    }
}

void
BinIOUnpack(void *context, int type, int byte_order, int count) {
    BinIOUnpackContext *ctx = (BinIOUnpackContext *) context;

    if (count == -1) {
        count = 1;
    }

    switch (type) {
        case BinIO_TYPE_IGNORE_BYTE:                                                                                                   ctx->data += 1 * count; break;
        case BinIO_TYPE_BYTE:        BinIOConvert1(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 1 * count; break;
        case BinIO_TYPE_INT16:       BinIOConvert2(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 2 * count; break;
        case BinIO_TYPE_INT32:       BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 4 * count; break;
        case BinIO_TYPE_INT64:       BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 8 * count; break;
        case BinIO_TYPE_FLOAT32:     BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 4 * count; break;
        case BinIO_TYPE_FLOAT64:     BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 8 * count; break;
    }
}

/* convert functions */
void
BinIOConvert1(int            from_byte_order, int      to_byte_order,
              const uint8_t *src,             uint8_t *dst,
              unsigned int   count)
{
    if (BinIONormalizeByteOrder(from_byte_order) !=
        BinIONormalizeByteOrder(to_byte_order))
    {
        unsigned int i;
        for (i = 0; i < count; ++i)
        {
            BinIOSwap1(src, dst);
            src += 1;
            dst += 1;
        }
    }
    else
    {
        memcpy(dst, src, 1 * count);
    }
}

void
BinIOConvert2(int            from_byte_order, int      to_byte_order,
              const uint8_t *src,             uint8_t *dst,
              unsigned int   count)
{
    if (BinIONormalizeByteOrder(from_byte_order) != BinIONormalizeByteOrder(to_byte_order))
    {
        unsigned int i;
        for (i = 0; i < count; ++i)
        {
            BinIOSwap2(src, dst);
            src += 2;
            dst += 2;
        }
    }
    else
    {
        memcpy(dst, src, 2 * count);
    }
}

void
BinIOConvert4(int            from_byte_order, int      to_byte_order,
              const uint8_t *src,             uint8_t *dst,
              unsigned int   count)
{
    if (BinIONormalizeByteOrder(from_byte_order) != BinIONormalizeByteOrder(to_byte_order))
    {
        unsigned int i;
        for (i = 0; i < count; ++i)
        {
            BinIOSwap4(src, dst);
            src += 4;
            dst += 4;
        }
    }
    else
    {
        memcpy(dst, src, 4 * count);
    }
}

void
BinIOConvert8(int            from_byte_order, int      to_byte_order,
              const uint8_t *src,             uint8_t *dst,
              unsigned int   count)
{
    if (BinIONormalizeByteOrder(from_byte_order) != BinIONormalizeByteOrder(to_byte_order))
    {
        unsigned int i;
        for (i = 0; i < count; ++i)
        {
            BinIOSwap8(src, dst);
            src += 8;
            dst += 8;
        }
    }
    else
    {
        memcpy(dst, src, 8 * count);
    }
}

void
BinIOInitFormatCursor(BinIOFormatCursor *cursor,
                      const char        *format)
{
    cursor->cursor = format;
    cursor->byte_order = BinIO_HOST_BYTE_ORDER;
    cursor->count = -1;
}

int
BinIONextChar(void                 *context,
              BinIOFormatCursor    *cursor,
              BinIOProcessFunction  func)
{
    int count, value;
    int c;
    switch (c = *(cursor->cursor)++)
    {
        case BinIO_LITTLE_ENDIAN_BYTE_ORDER:
        case BinIO_BIG_ENDIAN_BYTE_ORDER:
        case BinIO_HOST_BYTE_ORDER:
        case BinIO_NETWORK_BYTE_ORDER:
            cursor->byte_order = c;
            break;
        
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            count = cursor->count;
            value = c - '0';
            if (count == -1)
            {
                cursor->count = value;
            }
            else
            {
                cursor->count = (count * 10) + value;
            }
            break;
            
        case BinIO_TYPE_IGNORE_BYTE:
        case BinIO_TYPE_BYTE:
        case BinIO_TYPE_INT16:
        case BinIO_TYPE_INT32:
        case BinIO_TYPE_INT64:
        case BinIO_TYPE_FLOAT32:
        case BinIO_TYPE_FLOAT64:
            func(context, c, cursor->byte_order, cursor->count);
            cursor->byte_order = BinIO_HOST_BYTE_ORDER;
            cursor->count = -1;
            break;
            
        case ' ': case '\t': case '\r': case '\n':
            break;
            
        default:
            return 0;
    }
    
    return 1;
}

void
BinIOCountBytes(void *context, int type, int byte_order, int count)
{
    size_t type_size = 0;
    
    if (count == -1)
    {
        count = 1;
    }
    
    switch (type)
    {
        case BinIO_TYPE_IGNORE_BYTE: type_size = 1; break;
        case BinIO_TYPE_BYTE:        type_size = 1; break;
        case BinIO_TYPE_INT16:       type_size = 2; break;
        case BinIO_TYPE_INT32:       type_size = 4; break;
        case BinIO_TYPE_INT64:       type_size = 8; break;
        case BinIO_TYPE_FLOAT32:     type_size = 4; break;
        case BinIO_TYPE_FLOAT64:     type_size = 8; break;
    }
    
    *(size_t *) context += type_size * count;
}

size_t
BinIOFormatByteCount(const char *format)
{
    BinIOFormatCursor cursor;
    size_t n_bytes = 0;

    BinIOInitFormatCursor(&cursor, format);
    
    while (BinIONextChar(&n_bytes, &cursor, BinIOCountBytes)) {}
    
    return n_bytes;
}
