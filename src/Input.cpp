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

#include "Input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

bool IsKeyDown(SDL_Scancode key, int mod)
{
    bool iskey = false;

    if(std::find(keysdown.begin(), keysdown.begin(), key) != keysdown.end()) iskey = true;

    if (mod != 0)
        if ((SDL_GetModState() ^ mod) && iskey) return true;

    return iskey;
}

bool IsButtonPressed(Uint8 button)
{
    SDL_PumpEvents();
    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button)) return true;

    return false;
}

char *getButtonName(Uint8 button)
{
    if(button == SDL_BUTTON_LEFT) return   (char *) "Left Button";
    if(button == SDL_BUTTON_MIDDLE) return (char *) "Middle Button";
    if(button == SDL_BUTTON_RIGHT) return  (char *) "Right Button";

    return (char *) "Unknown";
}