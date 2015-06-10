/*
 * This file is part of Ragaru.
 *
 * Copyright © 2015 Victor A. Santos <victoraur.santos@gmail.com>
 *
 * Kugaru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kugaru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kugaru.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef MAPMETA_H
#define MAPMETA_H

typedef struct
{
    float x;
    float y;
    float z;
} XYZ;

typedef struct
{
    int texture;

    float r;
    float g;
    float b;

    float lr;
    float lg;
    float lb;
} Skybox;


#endif /* MAPMETA_H */
