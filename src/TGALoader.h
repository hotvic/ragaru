/*
 * Copruright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
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

#ifndef TGA_LOADER_H
#define TGA_LOADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gamegl.h"

typedef struct TGAImageRec
{
    GLubyte* data; // Image Data (Up To 32 Bits)
    GLuint bpp;    // Image Color Depth In Bits Per Pixel.
    GLuint sizeX;
    GLuint sizeY;
} TGAImageRec;

#endif /* TGA_LOADER_H */
