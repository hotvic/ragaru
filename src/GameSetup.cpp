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
    bool Game::Setup()
    {
        Uint8 sdlflags = 0;

        slomospeed = 0.25;
        slomofreq = 8012;

        config.load();

        keys.crouch  = config.input_key_crouch;
        keys.jump    = config.input_key_jump;
        keys.left    = config.input_key_left;
        keys.forward = config.input_key_forward;
        keys.back    = config.input_key_back;
        keys.right   = config.input_key_right;
        keys.draw    = config.input_key_draw;
        keys.throww  = config.input_key_throw;
        keys.chat    = config.input_key_chat;
        keys.attack  = config.input_button_attack;

        numplayers = 1;

        // Initialize SDL
        if(SDL_Init( SDL_INIT_VIDEO ) < 0) {
            LOG->ERR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return false;
        }

        if(SDL_GL_LoadLibrary(NULL) == -1) {
            LOG->ERR("SDL_GL_LoadLibrary() failed: %s\n", SDL_GetError());
            SDL_Quit();
            return false;
        }

        if(options.listresolutions) {
            /*print_resolutions();*/
            SDL_Quit();
            return false;
        }

        //if(!options.windowed)
        //    sdlflags |= SDL_WINDOW_FULLSCREEN;

        window = SDL_CreateWindow("Ragaru", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | sdlflags);
        if(window == NULL) {
            LOG->ERR("Can't initalize video...");

            return false;
        }

        this->glcontext = SDL_GL_CreateContext(window);
        if(this->glcontext == NULL) {
            LOG->ERR("Can't create GL context...");

            return false;
        }

        // if we are using fullscreen desktop then we need know window size
        if (config.video_resw == 0 || config.video_resh == 0)
        {
            SDL_GetWindowSize(window, &screenWidth, &screenHeight);
        }
        else
        {
            screenWidth = config.video_resw;
            screenHeight = config.video_resh;
        }

        // well... use vsync
        //SDL_GL_SetSwapInterval(1);

        //if(!options.nomousegrab)
            SDL_SetRelativeMouseMode(SDL_TRUE);

        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);

        // clear all states
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        // glDisable(GL_DITHER);
        glDisable(GL_FOG);
        glDisable(GL_LIGHTING);
        glDisable(GL_LOGIC_OP);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
        glPixelTransferi(GL_RED_SCALE, 1);
        glPixelTransferi(GL_RED_BIAS, 0);
        glPixelTransferi(GL_GREEN_SCALE, 1);
        glPixelTransferi(GL_GREEN_BIAS, 0);
        glPixelTransferi(GL_BLUE_SCALE, 1);
        glPixelTransferi(GL_BLUE_BIAS, 0);
        glPixelTransferi(GL_ALPHA_SCALE, 1);
        glPixelTransferi(GL_ALPHA_BIAS, 0);

        // set initial rendering states
        glShadeModel(GL_SMOOTH);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        // glDepthRange(FRONT_CLIP, BACK_CLIP);
        glEnable(GL_DEPTH_TEST);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_LIGHTING);
        // glEnable(GL_LIGHT_MODEL_AMBIENT);
        glEnable(GL_DITHER);
        glEnable(GL_COLOR_MATERIAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glAlphaFunc(GL_GREATER, 0.5f);

        screenWidth = 640;
        screenHeight = 480;

        this->newdetail = config.video_detail;
        this->newscreenwidth = screenWidth;
        this->newscreenheight = screenHeight;



        this->InitGame();

        config.save();

        return true;
    }

    void Game::CleanUp()
    {
        SDL_GL_DeleteContext(this->glcontext);
        SDL_Quit();
    }
}