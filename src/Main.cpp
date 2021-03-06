/*
 * Copyright © 2015, 2016 - Victor A. Santos <victoraur.santos@gmail.com>
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

#include "Main.h"
#include "Globals.h"
#include "Game.h"
#include "FileIO.h"
#include "openal_wrapper.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "gamegl.h"
#include "MacCompatibility.h"
extern "C" {
#include <zlib.h>
#include <png.h>
}

using namespace std;

unsigned int resolutionDepths[8][2] = {{0}};

// Menu defs
enum { kFileQuit = 1 };

enum { kForegroundSleep = 10, kBackgroundSleep = 10000 };

GLuint gFontList;
char gcstrMode[256] = "";

UInt32 gSleepTime = kForegroundSleep;
bool gDone = false, gfFrontProcess = true;

void ShutdownDSp()
{
}

void DrawGL(Ragaru::Game& game)
{
    game.DrawGLScene();
}

static void sdlEventProc(const SDL_Event& e, Ragaru::Game& game)
{
    bool skipkey = false;

    switch(e.type) {
    case SDL_MOUSEMOTION:
        game.deltah = e.motion.xrel;
        game.deltav = e.motion.yrel;

        return;

    case SDL_KEYDOWN:
        if(!skipkey) keysdown.push_back(e.key.keysym.scancode);

        return;

    case SDL_KEYUP:
        keysdown.erase(std::remove(keysdown.begin(), keysdown.end(), e.key.keysym.scancode));

        return;
    }
}

void print_resolutions()
{
    for(int i = 0; i < SDL_GetNumDisplayModes(0); i++) {
        SDL_DisplayMode mode;

        SDL_GetDisplayMode(0, i, &mode);
        std::cout << "Format: " << mode.format << std::endl;
        std::cout << "Witdh: " << mode.w << std::endl;
        std::cout << "Height: " << mode.h << std::endl;
        std::cout << "Refresh Rate: " << mode.refresh_rate << std::endl;
        std::cout << std::string(80, '-') << std::endl;
    }
}

void DoFrameRate(int update)
{
    static long frames = 0;

    static AbsoluteTime time = { 0, 0 };
    static AbsoluteTime frametime = { 0, 0 };
    AbsoluteTime currTime = UpTime();
    double deltaTime = (float)AbsoluteDeltaToDuration(currTime, frametime);

    if(0 > deltaTime) // if negative microseconds
        deltaTime /= -1000000.0;
    else // else milliseconds
        deltaTime /= 1000.0;

    multiplier = deltaTime;
    if(multiplier < .001)
        multiplier = .001;
    if(multiplier > 10)
        multiplier = 10;
    if(update)
        frametime = currTime; // reset for next time interval

    deltaTime = (float)AbsoluteDeltaToDuration(currTime, time);

    if(0 > deltaTime) // if negative microseconds
        deltaTime /= -1000000.0;
    else // else milliseconds
        deltaTime /= 1000.0;
    frames++;
    if(0.001 <= deltaTime) // has update interval passed
    {
        if(update) {
            time = currTime; // reset for next time interval
            frames = 0;
        }
    }
}

void DoUpdate(Ragaru::Game& game)
{
    static float sps = 200;
    static int count;
    static float oldmult;

    DoFrameRate(1);
    if(multiplier > .6)
        multiplier = .6;

    game.fps = 1 / multiplier;

    count = multiplier * sps;
    if(count < 2)
        count = 2;

    realmultiplier = multiplier;
    multiplier *= config.game_speed;
    if(config.game_difficulty == 1)
        multiplier *= .9;
    if(config.game_difficulty == 0)
        multiplier *= .8;

    if(game.loading == 4)
        multiplier *= .00001;
        // multiplier * .9;

    if(slomo && !mainmenu)
        multiplier *= slomospeed;
        // if(freeze) multiplier *= 0.00001;

    oldmult = multiplier;
    multiplier /= (float)count;

    game.DoMouse();

    game.TickOnce();

    for(int i = 0; i < count; i++) {
        game.Tick();
    }
    multiplier = oldmult;

    game.TickOnceAfter();
    /* - Debug code to test how many channels were active on average per frame
            static long frames = 0;

            static AbsoluteTime start = {0,0};
            AbsoluteTime currTime = UpTime ();
            static int num_channels = 0;

            num_channels += OPENAL_GetChannelsPlaying();
            double deltaTime = (float) AbsoluteDeltaToDuration (currTime, start);

            if (0 > deltaTime)	// if negative microseconds
                    deltaTime /= -1000000.0;
            else				// else milliseconds
                    deltaTime /= 1000.0;

            ++frames;

            if (deltaTime >= 1)
            {
                    start = currTime;
                    float avg_channels = (float)num_channels / (float)frames;

                    ofstream opstream("log.txt",ios::app);
                    opstream << "Average frame count: ";
                    opstream << frames;
                    opstream << " frames - ";
                    opstream << avg_channels;
                    opstream << " per frame.\n";
                    opstream.close();

                    frames = 0;
                    num_channels = 0;
            }
    */
    DrawGL(game);
}

int main(int argc, char** argv)
{
    LOG = new Ragaru::Log(true, true);

    Glib::init();

    Glib::OptionContext context;
    context.set_main_group(options);

    try {
        context.parse(argc, argv);
    } catch(const Glib::Error& ex) {
        LOG->ERR("Exception: %s", ex.what().data());
    }

    try {
        Ragaru::Game game;
        pgame = &game;

        if(!game.Setup())
            return 42;

        while(!gDone && !game.quit && (!game.tryquit)) {
            game.deltah = 0;
            game.deltav = 0;
            SDL_Event e;

            // message pump
            while(SDL_PollEvent(&e)) {
                if(e.type == SDL_QUIT) {
                    gDone = true;
                    break;
                }
                sdlEventProc(e, game);
            }
            // game
            DoUpdate(game);
        }

        pgame = 0;

        game.CleanUp();

        return 0;
    } catch(const std::exception& error) {
        SDL_Quit();

        LOG->ERR(error.what());
    }

    return 0;
}


extern int channels[100];
extern OPENAL_SAMPLE* samp[100];
extern OPENAL_STREAM* strm[20];

extern "C" void PlaySoundEx(int chan, OPENAL_SAMPLE* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused)
{
    const OPENAL_SAMPLE* currSample = OPENAL_GetCurrentSample(channels[chan]);
    if(currSample && currSample == samp[chan]) {
        if(OPENAL_GetPaused(channels[chan])) {
            OPENAL_StopSound(channels[chan]);
            channels[chan] = OPENAL_FREE;
        } else if(OPENAL_IsPlaying(channels[chan])) {
            int loop_mode = OPENAL_GetLoopMode(channels[chan]);
            if(loop_mode & OPENAL_LOOP_OFF) {
                channels[chan] = OPENAL_FREE;
            }
        }
    } else {
        channels[chan] = OPENAL_FREE;
    }

    channels[chan] = OPENAL_PlaySoundEx(channels[chan], sptr, dsp, startpaused);
    if(channels[chan] < 0) {
        channels[chan] = OPENAL_PlaySoundEx(OPENAL_FREE, sptr, dsp, startpaused);
    }
}

extern "C" void PlayStreamEx(int chan, OPENAL_STREAM* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused)
{
    const OPENAL_SAMPLE* currSample = OPENAL_GetCurrentSample(channels[chan]);
    if(currSample && currSample == OPENAL_Stream_GetSample(sptr)) {
        OPENAL_StopSound(channels[chan]);
        OPENAL_Stream_Stop(sptr);
    } else {
        OPENAL_Stream_Stop(sptr);
        channels[chan] = OPENAL_FREE;
    }

    channels[chan] = OPENAL_Stream_PlayEx(channels[chan], sptr, dsp, startpaused);
    if(channels[chan] < 0) {
        channels[chan] = OPENAL_Stream_PlayEx(OPENAL_FREE, sptr, dsp, startpaused);
    }
}

/* Screenshot code
    png_bytep screenshot = new png_byte[pgame->screenWidth * pgame->screenHeight * 3];

    glGetError();
    glReadPixels(0, 0, pgame->screenWidth, pgame->screenHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshot);
    if(glGetError() != GL_NO_ERROR)
        goto save_png_done;
*/