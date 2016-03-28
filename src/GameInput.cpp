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

#include "Game.h"

#include "Globals.h"

namespace Ragaru
{
    bool Game::isMouseHoverRect(int sx, int ex, int sy, int ey)
    {
        bool isHover = true;
        int mousex = ((double) mousecoordh / screenWidth) * 640;
        int mousey = 480 - ((double) mousecoordv / screenHeight) * 480;

        if (mousex < sx || mousex > ex)
            isHover = false;

        if (mousey < sy || mousey > ey)
            isHover = false;

        return isHover;
    }

    void Game::DoMouse()
    {
        if (mainmenu || (abs(deltah) < 10 * realmultiplier * 1000 && abs(deltav) < 10 * realmultiplier * 1000))
        {
            deltah      *= (config.input_mouse_speed / 2.5f);
            deltav      *= (config.input_mouse_speed / 2.5f);
            mousecoordh += deltah;
            mousecoordv += deltav;

            if (mousecoordh < 0) mousecoordh = 0;
            if (mousecoordh >= screenWidth) mousecoordh = screenWidth - 1;

            if (mousecoordv < 0) mousecoordv = 0;
            if (mousecoordv >= screenHeight) mousecoordv = screenHeight - 1;
        }
    }
}