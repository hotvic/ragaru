/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**> HEADER FILES <**/
#include "Game.h"
#include "TGALoader.h"
#include <iostream>

extern float texdetail;
extern TGAImageRec texture;
extern short vRefNum;
extern long dirID;
extern bool visibleloading;
extern Game * pgame;
extern int loadscreencolor;

extern bool LoadImage(const char * fname, TGAImageRec & tex);
/********************> LoadTGA() <*****/
bool upload_image(const char *fileName, bool hasalpha)
{
	if(visibleloading) {
		loadscreencolor = 1;
		pgame->LoadingScreen();
	}

    return LoadImage(fileName, texture);
}
