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

#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <SDL.h>

static std::vector<SDL_Scancode> keysdown;

bool IsKeyDown(SDL_Scancode key, int mod = 0);
bool IsButtonPressed(Uint8 button);
char *getButtonName(Uint8 button);
//void InitMouse();
//void MoveMouse(int xcoord, int ycoord, Point* mouseloc);
//void RefreshMouse(Point* mouseloc);
//void DisposeMouse();
//bool Button();

#endif /* INPUT_H */
