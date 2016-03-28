/*
 * Copyright © 2015, 2016 - Victor A. Santos <victoraur.santos@gmail.com>
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

#ifndef MAIN_H
#define MAIN_H

#include "Globals.h"
#include "Log.h"
#include "Options.h"
#include <glibmm.h>

static bool load_image(const char* fname, TGAImageRec& tex);
static bool load_png(const char* fname, TGAImageRec& tex);
static bool save_image(const char* fname);
static bool save_png(const char* fname);
void ShutdownDSp();
void DrawGL(Ragaru::Game& game);
void CreateGLWindow(void);
void DoUpdate(Ragaru::Game& game);
void DoEvent(void);
void print_resolutions();

Ragaru::Log* LOG;
Ragaru::Config config;
SDL_Window *window;
MainGroup options;

#endif /* MAIN_H */