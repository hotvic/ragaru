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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <ctime>
#include "Game.h"
#include "openal_wrapper.h"
#include "Globals.h"
#include "FileIO.h"

using namespace std;

static int music1;

static const char* rabbitskin[] = { "fur3.jpg",  "fur.jpg",       "fur2.jpg",
                                    "lynx.jpg",  "otter.jpg",     "opal.jpg",
                                    "sable.jpg", "chocolate.jpg", "bw2.jpg",
                                    "wb2.jpg" };

static const char* wolfskin[] = { "wolf.jpg", "darkwolf.jpg", "snowwolf.jpg" };

#define STATIC_ASSERT(x) extern int s_a_dummy[2 * (!!(x)) - 1];
STATIC_ASSERT(rabbittype == 0 && wolftype == 1)

static const char** creatureskin[] = { rabbitskin, wolfskin };

extern OPENAL_STREAM* strm[20];
extern "C" void PlaySoundEx(int channel, OPENAL_SAMPLE* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused);
extern "C" void PlayStreamEx(int chan, OPENAL_STREAM* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused);

typedef void (*console_handler)(Ragaru::Game* game, const char* args);

extern void ScreenShot(const char* fname);
void Screenshot(void)
{
    char temp[1024];
    time_t t = time(NULL);
    struct tm* tme = localtime(&t);
    sprintf(temp,
            "Screenshots\\Screenshot_%04d_%02d_%02d--%02d_%02d_%02d.png",
            tme->tm_year + 1900,
            tme->tm_mon + 1,
            tme->tm_mday,
            tme->tm_hour,
            tme->tm_min,
            tme->tm_sec);

    mkdir("Screenshots", S_IRWXU);

    LOG->ERR("Not Implemented yet! Screenshot\n");
    /* ScreenShot(temp); */
}

namespace Ragaru
{
    void Game::SetUpLighting()
    {
        if(environment == snowyenvironment) {
            light.color[0] = .65;
            light.color[1] = .65;
            light.color[2] = .7;
            light.ambient[0] = .4;
            light.ambient[1] = .4;
            light.ambient[2] = .44;
        }
        if(environment == desertenvironment) {
            light.color[0] = .95;
            light.color[1] = .95;
            light.color[2] = .95;
            light.ambient[0] = .4;
            light.ambient[1] = .35;
            light.ambient[2] = .3;
        }

        if(environment == grassyenvironment) {
            light.color[0] = .95;
            light.color[1] = .95;
            light.color[2] = 1;
            light.ambient[0] = .4;
            light.ambient[1] = .4;
            light.ambient[2] = .44;
        }
        if(!skyboxtexture) {
            light.color[0] = 1;
            light.color[1] = 1;
            light.color[2] = 1;
            light.ambient[0] = .4;
            light.ambient[1] = .4;
            light.ambient[2] = .4;
        }
        float average;
        average = (skyboxlightr + skyboxlightg + skyboxlightb) / 3;
        light.color[0] *= (skyboxlightr + average) / 2;
        light.color[1] *= (skyboxlightg + average) / 2;
        light.color[2] *= (skyboxlightb + average) / 2;
        light.ambient[0] = light.ambient[0] * (skyboxlightr + average) / 2 * 1;
        light.ambient[1] = light.ambient[1] * (skyboxlightg + average) / 2 * 1;
        light.ambient[2] = light.ambient[2] * (skyboxlightb + average) / 2 * 1;
        /*
        light.ambient[0]=0;
        light.ambient[1]=0;
        light.ambient[2]=0;	*/
    }

    int Game::findPathDist(int start, int end)
    {
        int i, j, k, smallestcount, count, connected;
        int last, last2, last3, last4;
        int closest;

        smallestcount = 1000;
        for(i = 0; i < 50; i++) {
            count = 0;
            last = start;
            last2 = -1;
            last3 = -1;
            last4 = -1;
            while(last != end && count < 30) {
                closest = -1;
                for(j = 0; j < numpathpoints; j++) {
                    if(j != last && j != last2 && j != last3 && j != last4) {
                        connected = 0;
                        if(numpathpointconnect[j])
                            for(k = 0; k < numpathpointconnect[j]; k++) {
                                if(pathpointconnect[j][k] == last)
                                    connected = 1;
                            }
                        if(!connected)
                            if(numpathpointconnect[last])
                                for(k = 0; k < numpathpointconnect[last]; k++) {
                                    if(pathpointconnect[last][k] == j)
                                        connected = 1;
                                }
                        if(connected)
                            if(closest == -1 || rand() % 2 == 0) {
                                closest = j;
                            }
                    }
                }
                last4 = last3;
                last3 = last2;
                last2 = last;
                last = closest;
                count++;
            }
            if(count < smallestcount)
                smallestcount = count;
        }
        return smallestcount;
    }

    int Game::checkcollide(XYZ startpoint, XYZ endpoint)
    {
        static XYZ colpoint, colviewer, coltarget;
        static float minx, minz, maxx, maxz, miny, maxy;
        static int i;

        // startpoint.y+=.7;
        // endpoint.y+=.7;
        // startpoint.y-=.1;
        // endpoint.y-=.1;

        minx = startpoint.x;
        if(minx > endpoint.x)
            minx = endpoint.x;
        miny = startpoint.y;
        if(miny > endpoint.y)
            miny = endpoint.y;
        minz = startpoint.z;
        if(minz > endpoint.z)
            minz = endpoint.z;

        maxx = startpoint.x;
        if(maxx < endpoint.x)
            maxx = endpoint.x;
        maxy = startpoint.y;
        if(maxy < endpoint.y)
            maxy = endpoint.y;
        maxz = startpoint.z;
        if(maxz < endpoint.z)
            maxz = endpoint.z;

        minx -= 1;
        miny -= 1;
        minz -= 1;
        maxx += 1;
        maxy += 1;
        maxz += 1;

        for(i = 0; i < objects.numobjects; i++) {
            if(objects.position[i].x > minx - objects.model[i].boundingsphereradius &&
               objects.position[i].x < maxx + objects.model[i].boundingsphereradius &&
               objects.position[i].y > miny - objects.model[i].boundingsphereradius &&
               objects.position[i].y < maxy + objects.model[i].boundingsphereradius &&
               objects.position[i].z > minz - objects.model[i].boundingsphereradius &&
               objects.position[i].z < maxz + objects.model[i].boundingsphereradius) {
                if(objects.type[i] != treeleavestype && objects.type[i] != bushtype && objects.type[i] != firetype) {
                    colviewer = startpoint;
                    coltarget = endpoint;
                    if(objects.model[i].LineCheck(
                           &colviewer, &coltarget, &colpoint, &objects.position[i], &objects.rotation[i]) != -1)
                        return i;
                }
            }
        }

        // if(terrain.lineTerrain(startpoint,endpoint,&colpoint)!=-1)return 1000;

        return -1;
    }

    int Game::checkcollide(XYZ startpoint, XYZ endpoint, int what)
    {
        static XYZ colpoint, colviewer, coltarget;
        static float minx, minz, maxx, maxz, miny, maxy;
        static int i;

        // startpoint.y+=.7;
        // endpoint.y+=.7;
        // startpoint.y-=.1;
        // endpoint.y-=.1;

        minx = startpoint.x;
        if(minx > endpoint.x)
            minx = endpoint.x;
        miny = startpoint.y;
        if(miny > endpoint.y)
            miny = endpoint.y;
        minz = startpoint.z;
        if(minz > endpoint.z)
            minz = endpoint.z;

        maxx = startpoint.x;
        if(maxx < endpoint.x)
            maxx = endpoint.x;
        maxy = startpoint.y;
        if(maxy < endpoint.y)
            maxy = endpoint.y;
        maxz = startpoint.z;
        if(maxz < endpoint.z)
            maxz = endpoint.z;

        minx -= 1;
        miny -= 1;
        minz -= 1;
        maxx += 1;
        maxy += 1;
        maxz += 1;

        if(what != 1000) {
            if(objects.position[what].x > minx - objects.model[what].boundingsphereradius &&
               objects.position[what].x < maxx + objects.model[what].boundingsphereradius &&
               objects.position[what].y > miny - objects.model[what].boundingsphereradius &&
               objects.position[what].y < maxy + objects.model[what].boundingsphereradius &&
               objects.position[what].z > minz - objects.model[what].boundingsphereradius &&
               objects.position[what].z < maxz + objects.model[what].boundingsphereradius) {
                if(objects.type[what] != treeleavestype && objects.type[what] != bushtype &&
                   objects.type[what] != firetype) {
                    colviewer = startpoint;
                    coltarget = endpoint;
                    if(objects.model[what].LineCheck(
                           &colviewer, &coltarget, &colpoint, &objects.position[what], &objects.rotation[what]) != -1)
                        return i;
                }
            }
        }

        if(what == 1000)
            if(terrain.lineTerrain(startpoint, endpoint, &colpoint) != -1)
                return 1000;

        return -1;
    }

    void Game::Setenvironment(int which)
    {
        LOG->LOG(" Setting environment...");

        float temptexdetail;
        environment = which;
        /*
                OPENAL_SetPaused(channels[music1snow], true);
                OPENAL_SetPaused(channels[music1grass], true);
                OPENAL_SetPaused(channels[music1desert], true);
                OPENAL_SetPaused(channels[wind], true);
                OPENAL_SetPaused(channels[desertambient], true);
        */
        OPENAL_SetPaused(channels[stream_music1snow], true);
        OPENAL_SetPaused(channels[stream_music1grass], true);
        OPENAL_SetPaused(channels[stream_music1desert], true);
        OPENAL_SetPaused(channels[stream_wind], true);
        OPENAL_SetPaused(channels[stream_desertambient], true);

        if(environment == snowyenvironment) {
            windvector = 0;
            windvector.z = 3;
            if(config.sound_ambient) {
                // PlaySoundEx( wind, samp[wind], NULL, true);
                PlayStreamEx(stream_wind, strm[stream_wind], 0, true);
                OPENAL_SetPaused(channels[stream_wind], false);
                OPENAL_SetVolume(channels[stream_wind], 256);
            }

            LoadTexture("snowtree.png", &objects.treetextureptr, 0, 1);
            LoadTexture("bushsnow.png", &objects.bushtextureptr, 0, 1);
            LoadTexture("bouldersnow.jpg", &objects.rocktextureptr, 1, 0);
            LoadTexture("snowbox.jpg", &objects.boxtextureptr, 1, 0);

            OPENAL_Sample_Free(samp[footstepsound]);
            OPENAL_Sample_Free(samp[footstepsound2]);
            OPENAL_Sample_Free(samp[footstepsound3]);
            OPENAL_Sample_Free(samp[footstepsound4]);
            samp[footstepsound] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound2] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound3] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepstone1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound4] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepstone2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound2], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound3], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound4], 4.0f, 1000.0f);

            LoadTexture("snow.jpg", &terraintexture, 1, 0);

            LoadTexture("rock.jpg", &terraintexture2, 1, 0);

            // LoadTexture("detailgrain.png",&terraintexture3,1);

            temptexdetail = texdetail;
            if(texdetail > 1)
                texdetail = 4;
            skybox.load("skyboxsnowfront.jpg",
                        "skyboxsnowleft.jpg",
                        "skyboxsnowback.jpg",
                        "skyboxsnowright.jpg",
                        "skyboxsnowup.jpg",
                        "skyboxsnowdown.jpg",
                        "skyboxsnowcloud.jpg",
                        "skyboxsnowreflect.jpg");

            texdetail = temptexdetail;
        }
        if(environment == desertenvironment) {
            windvector = 0;
            windvector.z = 2;
            LoadTexture("deserttree.png", &objects.treetextureptr, 0, 1);
            LoadTexture("bushdesert.png", &objects.bushtextureptr, 0, 1);
            LoadTexture("boulderdesert.jpg", &objects.rocktextureptr, 1, 0);
            LoadTexture("desertbox.jpg", &objects.boxtextureptr, 1, 0);

            if(config.sound_ambient) {
                // PlaySoundEx( desertambient, samp[desertambient], NULL, true);
                PlayStreamEx(stream_desertambient, strm[stream_desertambient], NULL, true);
                OPENAL_SetPaused(channels[stream_desertambient], false);
                OPENAL_SetVolume(channels[stream_desertambient], 256);
            }

            OPENAL_Sample_Free(samp[footstepsound]);
            OPENAL_Sample_Free(samp[footstepsound2]);
            OPENAL_Sample_Free(samp[footstepsound3]);
            OPENAL_Sample_Free(samp[footstepsound4]);
            samp[footstepsound] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound2] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound3] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound4] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepsnow2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound2], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound3], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound4], 4.0f, 1000.0f);

            LoadTexture("sand.jpg", &terraintexture, 1, 0);

            LoadTexture("sandslope.jpg", &terraintexture2, 1, 0);

            // LoadTexture("detailgrain.png",&terraintexture3,1);

            temptexdetail = texdetail;
            if(texdetail > 1)
                texdetail = 4;
            skybox.load("skyboxsandfront.jpg",
                        "skyboxsandleft.jpg",
                        "skyboxsandback.jpg",
                        "skyboxsandright.jpg",
                        "skyboxsandup.jpg",
                        "skyboxsanddown.jpg",
                        "skyboxsandcloud.jpg",
                        "skyboxsandreflect.jpg");

            texdetail = temptexdetail;
        }
        if(environment == grassyenvironment) {
            windvector = 0;
            windvector.z = 2;
            LoadTexture("tree.png", &objects.treetextureptr, 0, 1);
            LoadTexture("bush.png", &objects.bushtextureptr, 0, 1);
            LoadTexture("boulder.jpg", &objects.rocktextureptr, 1, 0);
            LoadTexture("grassbox.jpg", &objects.boxtextureptr, 1, 0);

            if(config.sound_ambient) {
                PlayStreamEx(stream_wind, strm[stream_wind], NULL, true);
                OPENAL_SetPaused(channels[stream_wind], false);
                OPENAL_SetVolume(channels[stream_wind], 100);
            }

            OPENAL_Sample_Free(samp[footstepsound]);
            OPENAL_Sample_Free(samp[footstepsound2]);
            OPENAL_Sample_Free(samp[footstepsound3]);
            OPENAL_Sample_Free(samp[footstepsound4]);
            samp[footstepsound] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepgrass1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound2] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepgrass2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound3] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepstone1.ogg").c_str(), OPENAL_HW3D, 0, 0);
            samp[footstepsound4] =
                OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "footstepstone2.ogg").c_str(), OPENAL_HW3D, 0, 0);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound2], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound3], 4.0f, 1000.0f);
            OPENAL_Sample_SetMinMaxDistance(samp[footstepsound4], 4.0f, 1000.0f);

            LoadTexture("grassdirt.jpg", &terraintexture, 1, 0);

            LoadTexture("mossrock.jpg", &terraintexture2, 1, 0);

            // LoadTexture("detail.png",&terraintexture3,1);

            temptexdetail = texdetail;
            if(texdetail > 1)
                texdetail = 4;
            skybox.load("skyboxgrassfront.jpg",
                        "skyboxgrassleft.jpg",
                        "skyboxgrassback.jpg",
                        "skyboxgrassright.jpg",
                        "skyboxgrassup.jpg",
                        "skyboxgrassdown.jpg",
                        "skyboxgrasscloud.jpg",
                        "skyboxgrassreflect.jpg");

            texdetail = temptexdetail;
        }
        temptexdetail = texdetail;
        texdetail = 1;
        terrain.load("heightmap.png");

        texdetail = temptexdetail;
    }

    void Game::Loadlevel(int which)
    {
        stealthloading = 0;

        switch(which) {
        case -1:
            tutoriallevel = -1;
            Loadlevel("tutorial");
            break;
        case 0:
            Loadlevel("map1");
            break;
        case 1:
            Loadlevel("map2");
            break;
        case 2:
            Loadlevel("map3");
            break;
        case 3:
            Loadlevel("map4");
            break;
        case 4:
            Loadlevel("map5");
            break;
        case 5:
            Loadlevel("map6");
            break;
        case 6:
            Loadlevel("map7");
            break;
        case 7:
            Loadlevel("map8");
            break;
        case 8:
            Loadlevel("map9");
            break;
        case 9:
            Loadlevel("map10");
            break;
        case 10:
            Loadlevel("map11");
            break;
        case 11:
            Loadlevel("map12");
            break;
        case 12:
            Loadlevel("map13");
            break;
        case 13:
            Loadlevel("map14");
            break;
        case 14:
            Loadlevel("map15");
            break;
        case 15:
            Loadlevel("map16");
            break;
        default:
            Loadlevel("mapsave");
            break;
        }

        whichlevel = which;
    }

    void Game::Loadlevel(const char* filename)
    {
        static int oldlevel;

        LOG->LOG("Loading level... %s", filename);

        if(!gameon)
            visibleloading = 1;

        if(stealthloading)
            visibleloading = 0;

        if(!stillloading)
            loadtime = 0;
        gamestarted = 1;

        numenvsounds = 0;
        // visibleloading=1;
        if(tutoriallevel != -1)
            tutoriallevel = 0;
        else
            tutoriallevel = 1;

        if(tutoriallevel == 1)
            tutorialstage = 0;
        if(tutorialstage == 0) {
            tutorialstagetime = 0;
            tutorialmaxtime = 1;
        }
        loadingstuff = 1;
        if(!firstload) {
            oldlevel = 50;
        }
        OPENAL_SetPaused(channels[whooshsound], true);
        OPENAL_SetPaused(channels[stream_firesound], true);

        std::string name = locateDataFile("maps", filename);

        int mapvers;
        FILE* tfile;
        tfile = fopen(name.c_str(), "rb");

        if(tfile) {
            OPENAL_SetPaused(channels[stream_firesound], true);

            scoreadded = 0;
            windialogue = 0;

            hostiletime = 0;

            won = 0;

            // campaign=0;
            animation[bounceidleanim].Load("idle.anim", middleheight, neutral);

            numdialogues = 0;

            for(int i = 0; i < 20; i++) {
                dialoguegonethrough[i] = 0;
            }

            indialogue = -1;
            cameramode = 0;

            damagedealt = 0;
            damagetaken = 0;

            if(accountactive != -1)
                config.game_difficulty = accountdifficulty[accountactive];

            if(config.game_difficulty != 2)
                minimap = 1;
            else
                minimap = 0;

            numhotspots = 0;
            currenthotspot = -1;
            bonustime = 1;

            skyboxtexture = 1;
            skyboxr = 1;
            skyboxg = 1;
            skyboxb = 1;

            freeze = 0;
            winfreeze = 0;

            for(int i = 0; i < 100; i++) {
                bonusnum[i] = 0;
            }

            numfalls = 0;
            numflipfail = 0;
            numseen = 0;
            numstaffattack = 0;
            numswordattack = 0;
            numknifeattack = 0;
            numunarmedattack = 0;
            numescaped = 0;
            numflipped = 0;
            numwallflipped = 0;
            numthrowkill = 0;
            numafterkill = 0;
            numreversals = 0;
            numattacks = 0;
            maxalarmed = 0;
            numresponded = 0;

            bonustotal = startbonustotal;
            bonus = 0;
            gameon = 1;
            changedelay = 0;
            if(console) {
                PlaySoundEx(consolesuccesssound, samp[consolesuccesssound], NULL, true);
                OPENAL_SetVolume(channels[consolesuccesssound], 256);
                OPENAL_SetPaused(channels[consolesuccesssound], false);
                freeze = 0;
                console = 0;
            }

            if(!stealthloading) {
                terrain.numdecals = 0;
                sprites.numsprites = 0;
                for(int i = 0; i < objects.numobjects; i++) {
                    objects.model[i].numdecals = 0;
                }

                int j = objects.numobjects;
                for(int i = 0; i < j; i++) {
                    objects.DeleteObject(0);
                    if(visibleloading) {
                        loadscreencolor = 4;
                        LoadingScreen();
                    }
                }

                for(int i = 0; i < subdivision; i++) {
                    for(int j = 0; j < subdivision; j++) {
                        terrain.patchobjectnum[i][j] = 0;
                    }
                }
                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }
            }

            weapons.numweapons = 0;

            funpackf(tfile, "Bi", &mapvers);
            funpackf(tfile, "Bi", &indemo);
            funpackf(tfile, "Bi", &maptype);
            funpackf(tfile, "Bi", &hostile);
            funpackf(tfile, "Bi", &environment);
            funpackf(tfile, "Bf", &viewdistance);
            funpackf(tfile, "Bf", &fadestart);
            funpackf(tfile, "Bf", &mapradius);
            funpackf(tfile, "Bi", &skyboxtexture);
            funpackf(tfile, "Bf Bf Bf", &skyboxr, &skyboxg, &skyboxb);
            funpackf(tfile, "Bf Bf Bf", &skyboxlightr, &skyboxlightg, &skyboxlightb);
            funpackf(tfile, "Bf Bf Bf", &mapcenter.x, &mapcenter.y, &mapcenter.z);

            funpackf(tfile, "Bi", &numdialogues);

            for(int i = 0; i < numdialogues; i++) {
                funpackf(tfile, "Bi", &dialoguetype[i]);

                for(int j = 0; j < 10; j++) {
                    funpackf(tfile, "Bf", &participantrotation[i][j]);
                    funpackf(tfile,
                             "Bf Bf Bf",
                             &participantlocation[i][j].x,
                             &participantlocation[i][j].y,
                             &participantlocation[i][j].z);
                }

                funpackf(tfile, "Bi", &numdialogueboxes[i]);

                for(int j = 0; j < numdialogueboxes[j]; j++) {
                    funpackf(tfile, "Bi", &dialogueboxlocation[i][j]);
                    funpackf(tfile, "Bi", &dialogueboxsound[i][j]);
                    funpackf(
                        tfile, "Bf Bf Bf", &dialogueboxcolor[i][j][0], &dialogueboxcolor[i][j][1], &dialogueboxcolor[i][j][2]);
                    funpackf(tfile, "Bf Bf Bf", &dialoguecamera[i][j].x, &dialoguecamera[i][j].y, &dialoguecamera[i][j].z);
                    funpackf(tfile, "Bf", &dialoguecamerarotation[i][j]);
                    funpackf(tfile, "Bf", &dialoguecamerarotation2[i][j]);
                    funpackf(tfile, "Bi", &participantfocus[i][j]);
                    funpackf(tfile, "Bi", &participantaction[i][j]);

                    for(int k = 0;; k++) {
                        funpackf(tfile, "Bb", &dialoguename[i][j][k]);
                        if(dialoguename[i][j][k] == '\0') break;
                    }

                    for(int k = 0;; k++) {
                        funpackf(tfile, "Bb", &dialoguetext[i][j][k]);
                        if(dialoguetext[i][j][k] == '\0') break;
                    }

                    for(int k = 0; k < 10; k++) {
                        funpackf(tfile,
                                 "Bf Bf Bf",
                                 &participantfacing[i][j][k].x,
                                 &participantfacing[i][j][k].y,
                                 &participantfacing[i][j][k].z);
                    }
                }
            }

            funpackf(tfile, "Bi", &objects.numobjects);
            for(int i = 0; i < objects.numobjects; i++) {
                funpackf(tfile, "Bi", &objects.type[i]);
                funpackf(tfile, "Bf", &objects.rotation[i]);
                funpackf(tfile, "Bf", &objects.rotation2[i]);
                funpackf(tfile, "Bf", &objects.scale[i]);
                funpackf(tfile, "Bf Bf Bf", &objects.position[i].x, &objects.position[i].y, &objects.position[i].z);
            }

            funpackf(tfile, "Bi", &numhotspots);
            for(int i = 0; i < numhotspots; i++) {
                funpackf(tfile, "Bi", &hotspottype[i]);
                funpackf(tfile, "Bf", &hotspotsize[i]);
                funpackf(tfile, "Bf Bf Bf", &hotspot[i].x, &hotspot[i].y, &hotspot[i].z);

                for(int j = 0;; j++) {
                    funpackf(tfile, "Bb", &hotspottext[i][j]);
                    if(hotspottext[i][j] == '\0') break;
                }
            }

            funpackf(tfile, "Bi", &numpathpoints);
            for(int i = 0; i < numpathpoints; i++) {
                funpackf(tfile, "Bf Bf Bf", &pathpoint[i].x, &pathpoint[i].y, &pathpoint[i].z);

                funpackf(tfile, "Bi", &numpathpointconnect[i]);
                for(int j = 0; j < numpathpointconnect[i]; j++) {
                    funpackf(tfile, "Bi", &pathpointconnect[i][j]);
                }
            }

            int tmplen;
            funpackf(tfile, "Bi", &tmplen);
            numplayers = tmplen - 1; // remove the main character from the count

            for(int i = 0; i < tmplen; i++) {
                funpackf(tfile, "Bf Bf Bf", &player[i].coords.x, &player[i].coords.y, &player[i].coords.z);
                funpackf(tfile, "Bf", &player[i].rotation);
                funpackf(tfile, "Bf", &player[i].targetrotation);
                funpackf(tfile, "Bf Bf Bf", &player[i].armorhead, &player[i].armorhigh, &player[i].armorlow);
                funpackf(tfile, "Bf Bf Bf", &player[i].protectionhead, &player[i].protectionhigh, &player[i].protectionlow);
                funpackf(tfile, "Bf Bf Bf", &player[i].metalhead, &player[i].metalhigh, &player[i].metallow);
                funpackf(tfile, "Bf Bf", &player[i].power, &player[i].speedmult);
                funpackf(tfile, "Bi Bi", &player[i].whichskin, &player[i].creature);
                funpackf(tfile, "Bf", &player[i].scale);
                funpackf(tfile, "Bi", &player[i].howactive);
                funpackf(tfile, "Bb", &player[i].immobile);
                funpackf(tfile, "Bf Bf Bf Bf", &player[i].headprop, &player[i].bodyprop, &player[i].armprop, &player[i].legprop);

                funpackf(tfile, "Bi", &player[i].numclothes);
                for(int j = 0; j < player[i].numclothes; j++) {
                    funpackf(
                        tfile, "Bf Bf Bf", &player[i].clothestintr[j], &player[i].clothestintb[j], &player[i].clothestintg[j]);

                    for(int k = 0;; k++) {
                        funpackf(tfile, "Bb", &player[i].clothes[j][k]);
                        if(player[i].clothes[j][k] == '\0') break;
                    }
                }

                funpackf(tfile, "Bi", &player[i].numweapons);
                for(int j = 0; j < player[i].numweapons; j++)
                    funpackf(tfile, "Bi", &player[i].weaponids[j]);

                funpackf(tfile, "Bi", &player[i].numwaypoints);
                for(int j = 0; j < player[i].numwaypoints; j++) {
                    funpackf(tfile, "Bi", &player[i].waypointtype[j]);
                    funpackf(
                        tfile, "Bf Bf Bf", &player[i].waypoints[j].x, &player[i].waypoints[j].y, &player[i].waypoints[j].z);
                }
            }

            // if(!stealthloading)funpackf(tfile, "Bf Bf Bf Bf Bf Bi",
            // &player[0].coords.x,&player[0].coords.y,&player[0].coords.z,&player[0].rotation,&player[0].targetrotation,
            // &player[0].numweapons);
            // if(stealthloading)funpackf(tfile, "Bf Bf Bf Bf Bf Bi",
            // &lamefloat,&lamefloat,&lamefloat,&lamefloat,&lamefloat, &player[0].numweapons);
            player[0].originalcoords = player[0].coords;

            if(visibleloading) {
                loadscreencolor = 4;
                LoadingScreen();
            }

            player[0].lastattack = -1;
            player[0].lastattack2 = -1;
            player[0].lastattack3 = -1;

            if(visibleloading) {
                loadscreencolor = 4;
                LoadingScreen();
            }

            if(!stealthloading) {
                objects.center = 0;
                for(int i = 0; i < objects.numobjects; i++) {
                    objects.center += objects.position[i];
                }
                objects.center /= objects.numobjects;

                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }

                float maxdistance = 0;
                float tempdist;
                int whichclosest;
                for(int i = 0; i < objects.numobjects; i++) {
                    tempdist = findDistancefast(&objects.center, &objects.position[i]);
                    if(tempdist > maxdistance) {
                        whichclosest = i;
                        maxdistance = tempdist;
                    }
                }
                objects.radius = fast_sqrt(maxdistance);
            }

            if(visibleloading) {
                loadscreencolor = 4;
                LoadingScreen();
            }

            SetUpLighting();
            if(environment != oldenvironment)
                Setenvironment(environment);
            oldenvironment = environment;

            if(!stealthloading) {
                int j = objects.numobjects;
                objects.numobjects = 0;
                for(int i = 0; i < j; i++) {
                    // if(objects.type[i]!=spiketype)
                    objects.MakeObject(
                        objects.type[i], objects.position[i], objects.rotation[i], objects.rotation2[i], objects.scale[i]);
                    if(visibleloading) {
                        loadscreencolor = 4;
                        LoadingScreen();
                    }
                }

                // if(skyboxtexture){
                terrain.DoShadows();
                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }
                objects.DoShadows();
                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }
                /*}
                else terrain.DoLighting();
                */
            }

            fclose(tfile);

            oldlevel = whichlevel;

            if(numplayers > maxplayers - 1)
                numplayers = maxplayers - 1;
            for(int i = 0; i < numplayers; i++) {
                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }
                player[i].burnt = 0;
                player[i].bled = 0;
                player[i].onfire = 0;
                if(i == 0 || player[i].scale < 0)
                    player[i].scale = .2;
                player[i].skeleton.free = 0;
                player[i].skeleton.id = i;
                // if(rand()%2==0)player[i].creature=wolftype;
                // else player[i].creature=rabbittype;
                if(i == 0 && mapvers < 9)
                    player[i].creature = rabbittype;
                if(player[i].creature != wolftype)
                    player[i].skeleton.Load("basic_figure.skl",
                                            "basic_figurelow.skl",
                                            "rabbitbelt.skl",
                                            "body.solid",
                                            "body2.solid",
                                            "body3.solid",
                                            "body4.solid",
                                            "body5.solid",
                                            "body6.solid",
                                            "body7.solid",
                                            "bodylow.solid",
                                            "belt.solid",
                                            0);
                else {
                    if(player[i].creature != wolftype) {
                        player[i].skeleton.Load("basic_figure.skl",
                                                "basic_figurelow.skl",
                                                "rabbitbelt.skl",
                                                "body.solid",
                                                "body2.solid",
                                                "body3.solid",
                                                "body4.solid",
                                                "body5.solid",
                                                "body6.solid",
                                                "body7.solid",
                                                "bodylow.solid",
                                                "belt.solid",
                                                1);
                        LoadTexture("belt.png", &player[i].skeleton.drawmodelclothes.textureptr, 1, 1);
                    }
                    if(player[i].creature == wolftype) {
                        player[i].skeleton.Load("basic_figure_wolf.skl",
                                                "Basic_figure_wolf_low.skl",
                                                "rabbitbelt.skl",
                                                "wolf.solid",
                                                "wolf2.solid",
                                                "wolf3.solid",
                                                "wolf4.solid",
                                                "wolf5.solid",
                                                "wolf6.solid",
                                                "wolf7.solid",
                                                "wolflow.solid",
                                                "welt.solid",
                                                0);
                    }
                }

                int texsize;
                texsize = 512 * 512 * 3 / texdetail / texdetail;
                // if(!player[i].loaded)player[i].skeleton.skinText = new GLubyte[texsize];
                // player[i].skeleton.skinText.resize(texsize);

                LoadTextureSave(creatureskin[player[i].creature][player[i].whichskin],
                                &player[i].skeleton.drawmodel.textureptr,
                                1,
                                &player[i].skeleton.skinText[0],
                                &player[i].skeleton.skinsize);

                if(player[i].numclothes) {
                    for(int j = 0; j < player[i].numclothes; j++) {
                        tintr = player[i].clothestintr[j];
                        tintg = player[i].clothestintg[j];
                        tintb = player[i].clothestintb[j];
                        AddClothes((char*)player[i].clothes[j],
                                   0,
                                   1,
                                   &player[i].skeleton.skinText[0],
                                   &player[i].skeleton.skinsize);
                    }
                    player[i].DoMipmaps(5, 0, 0, player[i].skeleton.skinsize, player[i].skeleton.skinsize);
                }

                player[i].currentanimation = bounceidleanim;
                player[i].targetanimation = bounceidleanim;
                player[i].currentframe = 0;
                player[i].targetframe = 1;
                player[i].target = 0;
                player[i].speed = 1 + (float)(rand() % 100) / 1000;
                if(config.game_difficulty == 0)
                    player[i].speed -= .2;
                if(config.game_difficulty == 1)
                    player[i].speed -= .1;

                player[i].velocity = 0;
                player[i].oldcoords = player[i].coords;
                player[i].realoldcoords = player[i].coords;

                player[i].id = i;
                player[i].skeleton.id = i;
                player[i].updatedelay = 0;
                player[i].normalsupdatedelay = 0;

                player[i].aitype = passivetype;
                player[i].aitarget = 0;
                player[i].madskills = 0;

                if(i == 0) {
                    player[i].proportionhead = 1.2;
                    player[i].proportionbody = 1.05;
                    player[i].proportionarms = 1.00;
                    player[i].proportionlegs = 1.1;
                    player[i].proportionlegs.y = 1.05;
                }
                player[i].headless = 0;
                player[i].currentoffset = 0;
                player[i].targetoffset = 0;
                /*player[i].armorhead=1;
                player[i].armorhigh=1;
                player[i].armorlow=1;
                player[i].protectionhead=1;
                player[i].protectionhigh=1;
                player[i].protectionlow=1;
                player[i].metalhead=1;
                player[i].metalhigh=1;
                player[i].metallow=1;
                player[i].power=1;
                player[i].speedmult=1;*/

                player[i].damagetolerance = 200;

                if(player[i].creature == wolftype) {
                    /*player[i].proportionhead=1.1;
                    player[i].proportionbody=1.1;
                    player[i].proportionarms=1.1;
                    player[i].proportionlegs=1.1;
                    player[i].proportionlegs.y=1.1;*/
                    if(i == 0 || player[i].scale < 0)
                        player[i].scale = .23;

                    player[i].damagetolerance = 300;
                }

                if(visibleloading) {
                    loadscreencolor = 4;
                    LoadingScreen();
                }
                if(cellophane) {
                    player[i].proportionhead.z = 0;
                    player[i].proportionbody.z = 0;
                    player[i].proportionarms.z = 0;
                    player[i].proportionlegs.z = 0;
                }

                player[i].tempanimation.Load((char*)"animations/tempanim.anim", 0, 0);

                player[i].headmorphness = 0;
                player[i].targetheadmorphness = 1;
                player[i].headmorphstart = 0;
                player[i].headmorphend = 0;

                player[i].pausetime = 0;

                player[i].dead = 0;
                player[i].jumppower = 5;
                player[i].damage = 0;
                player[i].permanentdamage = 0;
                player[i].superpermanentdamage = 0;

                player[i].forwardkeydown = 0;
                player[i].leftkeydown = 0;
                player[i].backkeydown = 0;
                player[i].rightkeydown = 0;
                player[i].jumpkeydown = 0;
                player[i].crouchkeydown = 0;
                player[i].throwkeydown = 0;

                player[i].collided = -10;
                player[i].loaded = 1;
                player[i].bloodloss = 0;
                player[i].weaponactive = -1;
                player[i].weaponstuck = -1;
                player[i].bleeding = 0;
                player[i].deathbleeding = 0;
                player[i].stunned = 0;
                player[i].hasvictim = 0;
                player[i].wentforweapon = 0;
            }

            player[0].aitype = playercontrolled;
            player[0].weaponactive = -1;

            if(config.game_difficulty == 1) {
                // player[0].speedmult=1/.9;
                player[0].power = 1 / .9;
            }

            if(config.game_difficulty == 0) {
                // player[0].speedmult=1/.8;
                player[0].power = 1 / .8;
            }

            // player[0].weaponstuck=1;

            if(config.game_difficulty == 1)
                player[0].damagetolerance = 250;
            if(config.game_difficulty == 0) {
                player[0].damagetolerance = 300;
                player[0].armorhead *= 1.5;
                player[0].armorhigh *= 1.5;
                player[0].armorlow *= 1.5;
            }
            cameraloc = player[0].coords;
            cameraloc.y += 5;
            rotation = player[0].rotation;

            hawkcoords = player[0].coords;
            hawkcoords.y += 30;

            if(visibleloading) {
                loadscreencolor = 4;
                LoadingScreen();
            }
            // weapons.numweapons=numplayers;
            for(int i = 0; i < weapons.numweapons; i++) {
                weapons.bloody[i] = 0;
                weapons.blooddrip[i] = 0;
                weapons.blooddripdelay[i] = 0;
                weapons.onfire[i] = 0;
                weapons.flamedelay[i] = 0;
                weapons.damage[i] = 0;
                // weapons.type[i]=sword;
                if(weapons.type[i] == sword) {
                    weapons.mass[i] = 1.5;
                    weapons.tipmass[i] = 1;
                    weapons.length[i] = .8;
                }
                if(weapons.type[i] == staff) {
                    weapons.mass[i] = 2;
                    weapons.tipmass[i] = 1;
                    weapons.length[i] = 1.5;
                }
                if(weapons.type[i] == knife) {
                    weapons.mass[i] = 1;
                    weapons.tipmass[i] = 1.2;
                    weapons.length[i] = .25;
                }
                weapons.position[i] = -1000;
                weapons.tippoint[i] = -1000;
            }

            /*		for(i=0;i<32;i++){
                                    //if(i<16||i>20)
                                    OPENAL_StopSound(i);
                            }
            */
            LOG->LOG("Starting background music...");

            OPENAL_StopSound(OPENAL_ALL);
            if(environment == snowyenvironment) {
                if(config.sound_ambient) {
                    PlayStreamEx(stream_wind, strm[stream_wind], NULL, true);
                    OPENAL_SetPaused(channels[stream_wind], false);
                    OPENAL_SetVolume(channels[stream_wind], 256);
                }
            } else if(environment == desertenvironment) {
                if(config.sound_ambient) {
                    // PlaySoundEx(desertambient,
                    //	samp[desertambient], NULL, true);
                    PlayStreamEx(stream_desertambient, strm[stream_desertambient], NULL, true);
                    OPENAL_SetPaused(channels[stream_desertambient], false);
                    OPENAL_SetVolume(channels[stream_desertambient], 256);
                }
            } else if(environment == grassyenvironment) {
                if(config.sound_ambient) {
                    // PlaySoundEx(wind, samp[wind], NULL, true);
                    PlayStreamEx(stream_wind, strm[stream_wind], NULL, true);
                    OPENAL_SetPaused(channels[stream_wind], false);
                    OPENAL_SetVolume(channels[stream_wind], 100);
                }
            }
            oldmusicvolume[0] = 0;
            oldmusicvolume[1] = 0;
            oldmusicvolume[2] = 0;
            oldmusicvolume[3] = 0;

            /*LoadTexture("cloud.png",&sprites.cloudtexture,1,1);
            LoadTexture("cloudimpact.png",&sprites.cloudimpacttexture,1,1);
            LoadTexture("bloodparticle.png",&sprites.bloodtexture,1,1);
            LoadTexture("snowflake.png",&sprites.snowflaketexture,1,1);
            LoadTexture("flame.png",&sprites.flametexture,1,1);
            LoadTexture("bloodflame.png",&sprites.bloodflametexture,1,1);
            LoadTexture("smoke.png",&sprites.smoketexture,1,1);
            LoadTexture("shine.png",&sprites.shinetexture,1,0);
            */

            if(!firstload) {
                firstload = 1;
            }
        }
        leveltime = 0;
        loadingstuff = 0;
        visibleloading = 0;
    }

    void Game::Tick()
    {
        static int i, k, j, l;
        static XYZ facing, flatfacing, absflatfacing;
        static XYZ rotatetarget;
        static bool oldkey;
        static float oldtargetrotation;
        static int target;
        static XYZ tempcoords1, tempcoords2;
        static XYZ test;
        static XYZ test2;
        static XYZ lowpoint, lowpointtarget, lowpoint2, lowpointtarget2, lowpoint3, lowpointtarget3, lowpoint4,
            lowpointtarget4, lowpoint5, lowpointtarget5, lowpoint6, lowpointtarget6, lowpoint7, lowpointtarget7, colpoint,
            colpoint2;
        static int whichhit;
        static bool oldjumpkeydown;

        float headprop, bodyprop, armprop, legprop;

        for(i = 0; i < 15; i++) {
            displaytime[i] += multiplier;
        }

        static bool mainmenutogglekeydown;
        if(!console) {
            if(mainmenu && endgame == 1)
                mainmenu = 10;
            if(IsKeyDown(SDL_SCANCODE_ESCAPE) && !mainmenutogglekeydown && (mainmenu == 7 && entername)) {
                for(j = 0; j < 255; j++) {
                    displaytext[0][j] = ' ';
                }
                displaychars[0] = 0;
                displayselected = 0;
                entername = 0;
                mainmenutogglekeydown = 1;
            }
            if((IsKeyDown(SDL_SCANCODE_ESCAPE) ||
                (mainmenu == 0 && ((IsKeyDown(SDL_SCANCODE_ESCAPE) || IsKeyDown(SDL_SCANCODE_SPACE) || (campaign))) &&
                 !oldjumpkeydown && campaign && winfreeze)) &&
               !mainmenutogglekeydown && (!mainmenu || gameon || mainmenu == 3 || mainmenu == 4 || mainmenu == 5 ||
                                          mainmenu == 6 || (mainmenu == 7 && !entername) || mainmenu == 9 ||
                                          mainmenu == 11 || mainmenu == 13 || mainmenu == 17 || mainmenu == 10)) {
                selected = -1;
                if(mainmenu == 1 || mainmenu == 2 || mainmenu == 0) {
                    if(mainmenu == 0 && !winfreeze)
                        mainmenu = 2;
                    else if(mainmenu == 0 && winfreeze && (campaignchoosenext[campaignchoicewhich[whichchoice]]) == 1)
                        mainmenu = 100;
                    else if(mainmenu == 0 && winfreeze) {
                        /*	if(campaignchoosenext[campaignchoicewhich[whichchoice]]==2)
                        stealthloading=1;
                        else stealthloading=0;

                        if(!stealthloading){
                        float gLoc[3]={0,0,0};
                        float vel[3]={0,0,0};
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                        PlaySoundEx( firestartsound, samp[firestartsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                        OPENAL_SetVolume(channels[firestartsound], 256);
                        OPENAL_SetPaused(channels[firestartsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                        flashr=1;
                        flashg=0;
                        flashb=0;
                        flashamount=1;
                        flashdelay=1;
                        }

                        startbonustotal=0;

                        for(i=0;i<campaignnumlevels;i++){
                        levelvisible[i]=0;
                        levelhighlight[i]=0;
                        }

                        levelorder[0]=0;
                        levelvisible[0]=1;
                        if(accountcampaignchoicesmade[accountactive])
                        for(i=0;i<accountcampaignchoicesmade[accountactive];i++){
                        levelorder[i+1]=campaignnextlevel[levelorder[i]][accountcampaignchoices[accountactive][i]];
                        levelvisible[levelorder[i+1]]=1;
                        }
                        int whichlevelstart;
                        whichlevelstart=accountcampaignchoicesmade[accountactive]-1;
                        if(whichlevelstart<0){
                        campaignchoicenum=1;
                        campaignchoicewhich[0]=0;
                        }
                        else
                        {
                        campaignchoicenum=campaignnumnext[levelorder[whichlevelstart]];
                        if(campaignchoicenum)
                        for(i=0;i<campaignchoicenum;i++){
                        campaignchoicewhich[i]=campaignnextlevel[levelorder[whichlevelstart]][i];
                        levelvisible[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
                        levelhighlight[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
                        }
                        }

                        loading=2;
                        loadtime=0;
                        targetlevel=7;
                        if(firstload)TickOnceAfter();
                        if(!firstload)LoadStuff();
                        //else {
                        for(i=0;i<255;i++){
                        mapname[i]='\0';
                        }
                        mapname[0]=':';
                        mapname[1]='D';
                        mapname[2]='a';
                        mapname[3]='t';
                        mapname[4]='a';
                        mapname[5]=':';
                        mapname[6]='M';
                        mapname[7]='a';
                        mapname[8]='p';
                        mapname[9]='s';
                        mapname[10]=':';

                        //accountcampaignchoices[accountactive][accountcampaignchoicesmade[accountactive]]=whichchoice;
                        //accountcampaignchoicesmade[accountactive]++;


                        strcat(mapname,campaignmapname[campaignchoicewhich[0]]);
                        whichchoice=0;
                        visibleloading=1;
                        stillloading=1;
                        Loadlevel(mapname);
                        campaign=1;
                        mainmenu=0;
                        gameon=1;
                        OPENAL_SetPaused(channels[music3], true);

                        stealthloading=0;*/
                    } else if(mainmenu == 1 || mainmenu == 2)
                        mainmenu = 0;
                    if(mainmenu && config.sound_music) {
                        if(mainmenu == 1 || mainmenu == 2 || mainmenu == 100) {
                            OPENAL_SetFrequency(OPENAL_ALL, 1);
                            PlayStreamEx(stream_music3, strm[stream_music3], NULL, true);
                            OPENAL_SetPaused(channels[stream_music3], false);
                            OPENAL_SetVolume(channels[stream_music3], 256);
                            OPENAL_SetPaused(channels[music1], true);
                        }
                    }
                    if(!mainmenu) {
                        OPENAL_SetPaused(channels[stream_music3], true);
                        OPENAL_SetPaused(channels[music1], false);
                    }
                }
                if(mainmenu == 3) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;
                    if(oldgamespeed == 0)
                        oldgamespeed = 1;

                    config.video_resw = newscreenwidth;
                    config.video_resh = newscreenheight;
                    config.video_detail = newdetail;

                    config.save();
                }
                if(mainmenu == 4 || mainmenu == 5 || mainmenu == 6 || mainmenu == 7 || mainmenu == 9 || mainmenu == 13 ||
                   mainmenu == 10 || mainmenu == 11 || mainmenu == 100) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;
                }
                if(mainmenu == 3 && gameon)
                    mainmenu = 2;
                if(mainmenu == 3 && !gameon)
                    mainmenu = 1;
                if(mainmenu == 5 && gameon)
                    mainmenu = 2;
                if(mainmenu == 5 && !gameon)
                    mainmenu = 1;
                if(mainmenu == 4)
                    mainmenu = 3;
                if(mainmenu == 6)
                    mainmenu = 5;
                if(mainmenu == 7)
                    mainmenu = 1;
                if(mainmenu == 9)
                    mainmenu = 5;
                if(mainmenu == 11)
                    mainmenu = 5;
                if(mainmenu == 13)
                    mainmenu = 12;
                if(mainmenu == 10)
                    mainmenu = 5;
                if(mainmenu == 100) {
                    mainmenu = 5;
                    gameon = 0;
                    winfreeze = 0;
                }
                mainmenutogglekeydown = 1;
            }
            if(!IsKeyDown(SDL_SCANCODE_ESCAPE)) {
                mainmenutogglekeydown = 0;
            }
        }

        /*static bool minimaptogglekeydown;
        if(IsKeyDown(theKeyMap, MAC_TAB_KEY)&&!minimaptogglekeydown){
        minimap=1-minimap;
        minimaptogglekeydown=1;
        }
        if(!IsKeyDown(theKeyMap, MAC_TAB_KEY)){
        minimaptogglekeydown=0;
        }
        */

        static bool minimaptogglekeydown;
        if(IsKeyDown(SDL_SCANCODE_TAB) && !minimaptogglekeydown && tutoriallevel) {
            if(tutorialstage != 51)
                tutorialstagetime = tutorialmaxtime;
            PlaySoundEx(consolefailsound, samp[consolefailsound], NULL, true);
            OPENAL_SetVolume(channels[consolefailsound], 128);
            OPENAL_SetPaused(channels[consolefailsound], false);
            minimaptogglekeydown = 1;
        }
        if(!IsKeyDown(SDL_SCANCODE_TAB)) {
            minimaptogglekeydown = 0;
        }

        if(mainmenu) {
            // menu buttons
            if(mainmenu == 1 || mainmenu == 2) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 1) {
                    printf("selected 1");
                    if(!gameon) {
                        float gLoc[3] = { 0, 0, 0 };
                        float vel[3] = { 0, 0, 0 };
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                        PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                        OPENAL_SetVolume(channels[firestartsound], 256);
                        OPENAL_SetPaused(channels[firestartsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                        flashr = 1;
                        flashg = 0;
                        flashb = 0;
                        flashamount = 1;
                        flashdelay = 1;

                        // new game
                        if(accountactive != -1)
                            mainmenu = 5;
                        else
                            mainmenu = 7;
                        /*
                        startbonustotal=0;

                        loading=2;
                        loadtime=0;
                        if(firstload)TickOnceAfter();
                        if(!firstload)LoadStuff();
                        else {
                        Loadlevel(0);
                        }
                        mainmenu=0;
                        gameon=1;
                        OPENAL_SetPaused(channels[music3], true);	*/
                    } else {
                        // resume
                        mainmenu = 0;
                        OPENAL_SetPaused(channels[stream_music3], true);
                        OPENAL_SetPaused(channels[music1], false);
                    }
                }

                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 2) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    // options

                    mainmenu = 3;

                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth > 3000)
                        newscreenwidth = config.video_resw;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight > 3000)
                        newscreenheight = config.video_resh;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;
                }

                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 3) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    if(!gameon) {
                        // quit
                        tryquit = 1;
                        OPENAL_SetPaused(channels[stream_music3], true);
                    } else {
                        // end game
                        gameon = 0;
                        mainmenu = 1;
                    }
                }
            }

            if(mainmenu == 3) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected != -1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);
                }
                /* if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 0) {
                    bool isCustomResolution = true;
                    bool found = false;
                    for(int i = 0; (!found) && (resolutions[i]); i++) {
                        if((resolutions[i]->w == screenwidth) && (resolutions[i]->h == screenwidth))
                            isCustomResolution = false;

                        if((resolutions[i]->w == newscreenwidth) && (resolutions[i]->h == newscreenheight)) {
                            i++;
                            if(resolutions[i] != NULL) {
                                newscreenwidth = (int)resolutions[i]->w;
                                newscreenheight = (int)resolutions[i]->h;
                            } else if(isCustomResolution) {
                                if((screenwidth == newscreenwidth) && (screenheight == newscreenheight)) {
                                    newscreenwidth = (int)resolutions[0]->w;
                                    newscreenheight = (int)resolutions[0]->h;
                                } else {
                                    newscreenwidth = screenwidth;
                                    newscreenheight = screenheight;
                                }
                            } else {
                                newscreenwidth = (int)resolutions[0]->w;
                                newscreenheight = (int)resolutions[0]->h;
                            }
                            found = true;
                        }
                    }

                    if(!found) {
                        newscreenwidth = (int)resolutions[0]->w;
                        newscreenheight = (int)resolutions[0]->h;
                    }
                }*/
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 1) {
                    newdetail++;
                    if(newdetail > 2)
                        newdetail = 0;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 2) {
                    config.video_blood++;
                    if(config.video_blood > 2)
                        config.video_blood = 0;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 3) {
                    config.game_difficulty++;
                    if(config.game_difficulty > 2)
                        config.game_difficulty = 0;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 4) {
                    config.video_blur = !config.video_blur;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 5) {
                    config.video_decals = !config.video_decals;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 6) {
                    config.sound_music = !config.sound_music;

                    if(!config.sound_music) {
                        OPENAL_SetPaused(channels[music1], true);
                        OPENAL_SetPaused(channels[stream_music2], true);
                        OPENAL_SetPaused(channels[stream_music3], true);

                        for(i = 0; i < 4; i++) {
                            oldmusicvolume[i] = 0;
                            musicvolume[i] = 0;
                        }
                    } else {
                        PlayStreamEx(stream_music3, strm[stream_music3], NULL, true);
                        OPENAL_SetPaused(channels[stream_music3], false);
                        OPENAL_SetVolume(channels[stream_music3], 256);
                    }
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 9) {
                    config.input_mouse_invert = !config.input_mouse_invert;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 10) {
                    config.input_mouse_speed++;
                    if(config.input_mouse_speed > 5)
                        config.input_mouse_speed = 1;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 11) {
                    config.sound_volume += 5;
                    if(config.sound_volume > 100)
                        config.sound_volume = 0;
                    OPENAL_SetSFXMasterVolume(config.sound_volume);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 7) {
                    /*float gLoc[3]={0,0,0};
                    float vel[3]={0,0,0};
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx( firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);
                    */
                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    // options

                    mainmenu = 4;
                    keyselect = -1;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 8) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;
                    if(oldgamespeed == 0)
                        oldgamespeed = 1;

                    config.video_resw = newscreenwidth;
                    config.video_resh = newscreenheight;
                    config.video_detail = newdetail;

                    config.save();

                    if(mainmenu == 3 && gameon)
                        mainmenu = 2;
                    if(mainmenu == 3 && !gameon)
                        mainmenu = 1;
                }
            }
            if(mainmenu == 4) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected != -1 && keyselect == -1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected < 9 && keyselect == -1) {
                    keyselect = selected;
                }
                /*if(keyselect != -1) {
                    for(i = 0; i < 3; i++)
                        if(!buttons[i] && !IsButtonPressed(SDL_BUTTON_LEFT))
                            oldbuttons[i] = 0;
                    for(i = 0; i < 140; i++) {
                        if((IsKeyDown(theKeyMap, i) || (buttons[0] && !oldbuttons[0] && !oldbutton) ||
                            (buttons[1] && !oldbuttons[1] && !oldbutton)) &&
                           keyselect != -1) {
                            if(i != MAC_ESCAPE_KEY &&
                               (strcmp(KeyToChar(i), "unknown") || (buttons[0] && !oldbuttons[0] && !oldbutton) ||
                                (buttons[1] && !oldbuttons[1] && !oldbutton))) {
                                float gLoc[3] = { 0, 0, 0 };
                                float vel[3] = { 0, 0, 0 };
                                OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                                PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                                OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                                OPENAL_SetVolume(channels[fireendsound], 256);
                                OPENAL_SetPaused(channels[fireendsound], false);
                                OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                                int keynum;
                                keynum = i;
                                if(buttons[0] && !oldbuttons[0])
                                    keynum = MAC_MOUSEBUTTON1;
                                if(buttons[1] && !oldbuttons[1])
                                    keynum = MAC_MOUSEBUTTON2;

                                if(keyselect == 0)
                                    forwardkey = keynum;
                                if(keyselect == 1)
                                    backkey = keynum;
                                if(keyselect == 2)
                                    leftkey = keynum;
                                if(keyselect == 3)
                                    rightkey = keynum;
                                if(keyselect == 4)
                                    crouchkey = keynum;
                                if(keyselect == 5)
                                    jumpkey = keynum;
                                if(keyselect == 6)
                                    drawkey = keynum;
                                if(keyselect == 7)
                                    throwkey = keynum;
                                if(keyselect == 8)
                                    attackkey = keynum;
                                keyselect = -1;
                            }
                        }
                    }
                }*/
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 9) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 3;

                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth > 3000)
                        newscreenwidth = config.video_resw;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight > 3000)
                        newscreenheight = config.video_resh;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;
                }
            }

            if(mainmenu == 5) {

                if(endgame == 2) {
                    accountcampaignchoicesmade[accountactive] = 0;
                    accountcampaignscore[accountactive] = 0;
                    accountcampaigntime[accountactive] = 0;
                    endgame = 0;
                }

                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    startbonustotal = 0;

                    loading = 2;
                    loadtime = 0;
                    targetlevel = -1;
                    if(firstload)
                        TickOnceAfter();
                    if(!firstload)
                        LoadStuff();
                    else {
                        Loadlevel(-1);
                    }

                    mainmenu = 0;
                    gameon = 1;
                    OPENAL_SetPaused(channels[stream_music3], true);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) &&
                   selected - 7 >=
                       accountcampaignchoicesmade[accountactive]) { // selected>=7&&(selected-7<=campaignnumchoices)){
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    startbonustotal = 0;

                    loading = 2;
                    loadtime = 0;
                    targetlevel = 7;
                    if(firstload)
                        TickOnceAfter();
                    if(!firstload)
                        LoadStuff();
                    // else {
                    for(i = 0; i < 255; i++) {
                        mapname[i] = '\0';
                    }
                    mapname[0] = 'm';
                    mapname[1] = 'a';
                    mapname[2] = 'p';
                    mapname[3] = 's';
                    mapname[4] = '/';
                    strcat(mapname,
                           campaignmapname[campaignchoicewhich[selected - 7 - accountcampaignchoicesmade[accountactive]]]);
                    whichchoice = selected - 7 - accountcampaignchoicesmade[accountactive];
                    visibleloading = 1;
                    stillloading = 1;
                    Loadlevel(mapname);
                    // Loadlevel(campaignmapname[levelorder[selected-7]]);
                    //}
                    campaign = 1;
                    mainmenu = 0;
                    gameon = 1;
                    OPENAL_SetPaused(channels[stream_music3], true);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 4) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    if(mainmenu == 5 && gameon)
                        mainmenu = 2;
                    if(mainmenu == 5 && !gameon)
                        mainmenu = 1;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 5) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 7;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 3) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 6;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 2) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 9;
                }
            }
            if(mainmenu == 9) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected < numchallengelevels && selected >= 0 &&
                   selected <= accountprogress[accountactive]) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    startbonustotal = 0;

                    loading = 2;
                    loadtime = 0;
                    targetlevel = selected;
                    if(firstload)
                        TickOnceAfter();
                    if(!firstload)
                        LoadStuff();
                    else {
                        Loadlevel(selected);
                    }
                    campaign = 0;

                    mainmenu = 0;
                    gameon = 1;
                    OPENAL_SetPaused(channels[stream_music3], true);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == numchallengelevels) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 5;
                }
            }
            if(mainmenu == 11) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected < numchallengelevels && selected >= 0 &&
                   selected <= accountprogress[accountactive]) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    startbonustotal = 0;

                    loading = 2;
                    loadtime = 0;
                    targetlevel = selected;
                    if(firstload)
                        TickOnceAfter();
                    if(!firstload)
                        LoadStuff();
                    else {
                        Loadlevel(selected);
                    }
                    campaign = 0;

                    mainmenu = 0;
                    gameon = 1;
                    OPENAL_SetPaused(channels[stream_music3], true);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == numchallengelevels) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 5;
                }
            }
            if(mainmenu == 10) {
                endgame = 2;
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 3) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 5;
                }
            }

            if(mainmenu == 6) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected != -1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    for(i = accountactive; i < numaccounts - 1; i++) {
                        accountdifficulty[i] = accountdifficulty[i + 1];
                        accountcampaignchoicesmade[i] = accountcampaignchoicesmade[i + 1];
                        for(j = 0; j < accountcampaignchoicesmade[i + 1]; j++) {
                            accountcampaignchoices[i][j] = accountcampaignchoices[i + 1][j];
                        }
                        accountpoints[i] = accountpoints[i + 1];
                        for(j = 0; j < 50; j++) {
                            accounthighscore[i][j] = accounthighscore[i + 1][j];
                            accountfasttime[i][j] = accountfasttime[i + 1][j];
                        }
                        for(j = 0; j < 60; j++) {
                            accountunlocked[i][j] = accountunlocked[i + 1][j];
                        }
                        for(j = 0; j < 256; j++) {
                            accountname[i][j] = accountname[i + 1][j];
                        }
                        accountcampaignhighscore[i] = accountcampaignhighscore[i + 1];
                        accountprogress[i] = accountprogress[i + 1];
                        accountcampaignfasttime[i] = accountcampaignfasttime[i + 1];
                        accountcampaignscore[i] = accountcampaignscore[i + 1];
                        accountcampaigntime[i] = accountcampaigntime[i + 1];
                    }

                    numaccounts--;
                    accountactive = -1;

                    mainmenu = 7;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 2) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 5;
                }
            }
            if(mainmenu == 7) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected != -1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == 0 && numaccounts < 8) {
                    entername = 1;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected > 0 && selected < numaccounts + 1) {
                    accountactive = selected - 1;
                    mainmenu = 5;
                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;
                }
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected == numaccounts + 1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;

                    mainmenu = 1;

                    for(j = 0; j < 255; j++) {
                        displaytext[0][j] = ' ';
                    }
                    displaychars[0] = 0;
                    displayselected = 0;
                    entername = 0;
                }
            }
            if(mainmenu == 8) {
                if(IsButtonPressed(SDL_BUTTON_LEFT) && selected != -1) {
                    float gLoc[3] = { 0, 0, 0 };
                    float vel[3] = { 0, 0, 0 };
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                    OPENAL_SetVolume(channels[firestartsound], 256);
                    OPENAL_SetPaused(channels[firestartsound], false);
                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                    if(selected == 0)
                        accountdifficulty[accountactive] = 0;
                    if(selected == 1)
                        accountdifficulty[accountactive] = 1;
                    if(selected == 2)
                        accountdifficulty[accountactive] = 2;

                    mainmenu = 5;

                    flashr = 1;
                    flashg = 0;
                    flashb = 0;
                    flashamount = 1;
                    flashdelay = 1;
                }
            }

            if(IsKeyDown(SDL_SCANCODE_Q, KMOD_CTRL)) {
                tryquit = 1;
                if(mainmenu == 3) {
                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;

                    config.video_resw = newscreenwidth;
                    config.video_resh = newscreenheight;
                    config.video_detail = newdetail;

                    config.save();
                }
            }

            if(mainmenu == 1 || mainmenu == 2) {
                if(loaddistrib > 4)
                    transition += multiplier / 8;
                if(transition > 1) {
                    transition = 0;
                    anim++;
                    if(anim > 4)
                        anim = 0;
                    loaddistrib = 0;
                }
            }
            OPENAL_SetFrequency(channels[stream_music3], 22050);

            if(entername) {
                // TODO: text input
                /*for(i = 0; i < 140; i++) {
                    if(IsKeyDown(theKeyMap, i)) {
                        togglekeydelay[i] += multiplier;
                        if(togglekeydelay[i] > .4) {
                            togglekey[i] = 0;
                            togglekeydelay[i] = .36;
                        }
                        if(!togglekey[i]) {
                            if(KeyToSingleChar(i) != '\0' && displaychars[0] < 60) {
                                for(j = 255; j >= displayselected + 1; j--) {
                                    displaytext[0][j] = displaytext[0][j - 1];
                                }
                                displaytext[0][displayselected] = KeyToSingleChar(i);
                                if(IsKeyDown(theKeyMap, MAC_SHIFT_KEY))
                                    displaytext[0][displayselected] = Shift(displaytext[0][displayselected]);
                                displayselected++;
                                displaychars[0]++;
                            }
                            if(i == MAC_DELETE_KEY && displayselected != 0) {
                                for(j = displayselected - 1; j < 255; j++) {
                                    displaytext[0][j] = displaytext[0][j + 1];
                                }
                                displaytext[0][255] = ' ';
                                displayselected--;
                                displaychars[0]--;
                            }
                            if(i == MAC_ARROW_LEFT_KEY && displayselected != 0) {
                                displayselected--;
                            }
                            if(i == MAC_ARROW_RIGHT_KEY && displayselected < displaychars[0]) {
                                displayselected++;
                            }
                            if(i == MAC_RETURN_KEY && entername) {
                                if(displaychars[0]) {
                                    numaccounts++;
                                    strcpy(accountname[numaccounts - 1], displaytext[0]);
                                    accountactive = numaccounts - 1;
                                    accountdifficulty[accountactive] = 1;
                                    accountprogress[accountactive] = 0;
                                    accountpoints[accountactive] = 0;
                                    accountcampaigntime[accountactive] = 0;
                                    accountcampaignscore[accountactive] = 0;
                                    accountcampaignfasttime[accountactive] = 0;
                                    accountcampaignhighscore[accountactive] = 0;
                                    for(j = 0; j < 50; j++) {
                                        accounthighscore[accountactive][j] = 0;
                                        accountfasttime[accountactive][j] = 0;
                                    }
                                    for(j = 0; j < 60; j++) {
                                        accountunlocked[accountactive][j] = 0;
                                    }
                                    accountcampaignchoicesmade[accountactive] = 0;

                                    for(j = 0; j < 255; j++) {
                                        displaytext[0][j] = ' ';
                                    }
                                    displaychars[0] = 0;
                                    displayselected = 0;
                                    entername = 0;

                                    mainmenu = 8;

                                    flashr = 1;
                                    flashg = 0;
                                    flashb = 0;
                                    flashamount = 1;
                                    flashdelay = 1;

                                    float gLoc[3] = { 0, 0, 0 };
                                    float vel[3] = { 0, 0, 0 };
                                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                                    OPENAL_SetVolume(channels[firestartsound], 256);
                                    OPENAL_SetPaused(channels[firestartsound], false);
                                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                                    for(j = 0; j < 255; j++) {
                                        displaytext[0][j] = ' ';
                                    }
                                    displaychars[0] = 0;

                                    displayselected = 0;
                                }
                            }

                            if(i == MAC_RETURN_KEY && mainmenu == 13) {
                                if(displaychars[0]) {
                                    sprintf(registrationname, "%s", displaytext[0]);
                                    if(displaychars[0] < 254)
                                        registrationname[displaychars[0]] = '\0';

                                    mainmenu = 5;

                                    flashr = 1;
                                    flashg = 0;
                                    flashb = 0;
                                    flashamount = 1;
                                    flashdelay = 1;

                                    float gLoc[3] = { 0, 0, 0 };
                                    float vel[3] = { 0, 0, 0 };
                                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                                    PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                                    OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                                    OPENAL_SetVolume(channels[firestartsound], 256);
                                    OPENAL_SetPaused(channels[firestartsound], false);
                                    OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                                    for(j = 0; j < 255; j++) {
                                        displaytext[0][j] = ' ';
                                    }
                                    displaychars[0] = 0;

                                    displayselected = 0;
                                }
                            }
                        }
                        togglekey[i] = 1;
                    } else {
                        togglekey[i] = 0;
                        togglekeydelay[i] = 0;
                    }
                }

                displayblinkdelay -= multiplier;
                if(displayblinkdelay <= 0) {
                    displayblinkdelay = .3;
                    displayblink = 1 - displayblink;
                }*/
            }
        }

        if(!mainmenu) {
            if(hostile == 1)
                hostiletime += multiplier;
            else
                hostiletime = 0;
            if(!winfreeze)
                leveltime += multiplier;
            if(IsKeyDown(SDL_SCANCODE_ESCAPE)) {
                chatting = 0;
                console = 0;
                freeze = 0;
                displaychars[0] = 0;
            }

            if(IsKeyDown(keys.chat) && !chattogglekeydown && !console && !chatting && config.game_debug) {
                chatting = 1;
                chattogglekeydown = 1;
                togglekey[keys.chat] = 1;
                togglekeydelay[keys.chat] = -20;
            }

            if(!IsKeyDown(keys.chat)) {
                chattogglekeydown = 0;
            }

            if(chatting) {
                // TODO: write text input using new SDL text input feature
                /*for(i = 0; i < 140; i++) {
                    if(IsKeyDown(i)) {
                        togglekeydelay[i] += multiplier;
                        if(togglekeydelay[i] > .4) {
                            togglekey[i] = 0;
                            togglekeydelay[i] = .36;
                        }
                        if(!togglekey[i]) {
                            if(KeyToSingleChar(i) != '\0' && displaychars[0] < 60) {
                                for(j = 255; j >= displayselected + 1; j--) {
                                    displaytext[0][j] = displaytext[0][j - 1];
                                }
                                displaytext[0][displayselected] = KeyToSingleChar(i);
                                if(IsKeyDown(theKeyMap, MAC_SHIFT_KEY))
                                    displaytext[0][displayselected] = Shift(displaytext[0][displayselected]);
                                displayselected++;
                                displaychars[0]++;
                            }
                            if(i == MAC_DELETE_KEY && displayselected != 0) {
                                for(j = displayselected - 1; j < 255; j++) {
                                    displaytext[0][j] = displaytext[0][j + 1];
                                }
                                displaytext[0][255] = ' ';
                                displayselected--;
                                displaychars[0]--;
                            }
                            if(i == MAC_ARROW_LEFT_KEY && displayselected != 0) {
                                displayselected--;
                            }
                            if(i == MAC_ARROW_RIGHT_KEY && displayselected < displaychars[0]) {
                                displayselected++;
                            }
                            if(i == MAC_RETURN_KEY) {
                                if(displaychars[0]) {
                                    for(j = 0; j < 255; j++) {
                                        displaytext[0][j] = ' ';
                                    }
                                    displaychars[0] = 0;
                                    displayselected = 0;
                                    chatting = 0;
                                }
                            }
                        }
                        togglekey[i] = 1;
                    } else {
                        togglekey[i] = 0;
                        togglekeydelay[i] = 0;
                    }
                }*/

                displayblinkdelay -= multiplier;
                if(displayblinkdelay <= 0) {
                    displayblinkdelay = .3;
                    displayblink = 1 - displayblink;
                }
            }

            if(chatting)
                keyboardfrozen = 1;

            if(IsKeyDown(SDL_SCANCODE_V) && !freezetogglekeydown && config.game_debug) {
                freeze = 1 - freeze;
                if(freeze) {
                    OPENAL_SetFrequency(OPENAL_ALL, 1);
                }
                freezetogglekeydown = 1;
            }

            if(!IsKeyDown(SDL_SCANCODE_V) && !IsKeyDown(SDL_SCANCODE_F1)) {
                freezetogglekeydown = 0;
            }

            if(IsKeyDown(SDL_SCANCODE_NONUSBACKSLASH) && !consoletogglekeydown && config.game_debug) {
                console = 1 - console;
                if(!console)
                    freeze = 0;
                if(console) {
                    OPENAL_SetFrequency(OPENAL_ALL, 1);
                }
                consoletogglekeydown = 1;
            }

            if(!IsKeyDown(SDL_SCANCODE_NONUSBACKSLASH)) {
                consoletogglekeydown = 0;
            }

            if(console)
                freeze = 1;

            if(console && !IsKeyDown(SDL_SCANCODE_LCTRL)) {
                // TODO: more text inputing ??
                /*
                for(i = 0; i < 140; i++) {
                    if(IsKeyDown(theKeyMap, i)) {
                        togglekeydelay[i] += multiplier;
                        if(togglekeydelay[i] > .4) {
                            togglekey[i] = 0;
                            togglekeydelay[i] = .36;
                        }
                        if(!togglekey[i]) {
                            if(KeyToSingleChar(i) != '\0' && consolechars[0] < 255) {
                                for(j = 255; j >= consoleselected + 1; j--) {
                                    consoletext[0][j] = consoletext[0][j - 1];
                                }
                                consoletext[0][consoleselected] = KeyToSingleChar(i);
                                if(IsKeyDown(theKeyMap, MAC_SHIFT_KEY))
                                    consoletext[0][consoleselected] = Shift(consoletext[0][consoleselected]);
                                consoleselected++;
                                consolechars[0]++;
                            } else if(i == MAC_ENTER_KEY) {
                                for(j = 255; j >= consoleselected + 1; j--) {
                                    consoletext[0][j] = consoletext[0][j - 1];
                                }
                                consoletext[0][consoleselected] = '\n';
                                consoleselected++;
                                consolechars[0]++;
                            }
                            if(i == MAC_DELETE_KEY && consoleselected != 0) {
                                for(j = consoleselected - 1; j < 255; j++) {
                                    consoletext[0][j] = consoletext[0][j + 1];
                                }
                                consoletext[0][255] = ' ';
                                consoleselected--;
                                consolechars[0]--;
                            }
                            if(i == MAC_ARROW_UP_KEY) {
                                if(archiveselected < 14)
                                    archiveselected++;
                                for(j = 0; j < 255; j++) {
                                    consolechars[0] = consolechars[archiveselected];
                                    consoletext[0][j] = consoletext[archiveselected][j];
                                    consoleselected = consolechars[0];
                                }
                            }
                            if(i == MAC_ARROW_DOWN_KEY) {
                                if(archiveselected > 0)
                                    archiveselected--;
                                for(j = 0; j < 255; j++) {
                                    consolechars[0] = consolechars[archiveselected];
                                    consoletext[0][j] = consoletext[archiveselected][j];
                                    consoleselected = consolechars[0];
                                }
                            }
                            if(i == MAC_ARROW_LEFT_KEY && consoleselected != 0) {
                                consoleselected--;
                            }
                            if(i == MAC_ARROW_RIGHT_KEY && consoleselected < consolechars[0]) {
                                consoleselected++;
                            }
                            if(i == MAC_RETURN_KEY) {
                                archiveselected = 0;
                                cmd_dispatch(this, consoletext[0]);

                                if(consolechars[0] > 0) {
                                    for(k = 14; k >= 1; k--) {
                                        for(j = 0; j < 255; j++) {
                                            consoletext[k][j] = consoletext[k - 1][j];
                                        }
                                        consolechars[k] = consolechars[k - 1];
                                    }
                                    for(j = 0; j < 255; j++) {
                                        consoletext[0][j] = ' ';
                                    }
                                    consolechars[0] = 0;
                                    consoleselected = 0;
                                }
                            }
                        }
                        togglekey[i] = 1;
                    } else {
                        togglekey[i] = 0;
                        togglekeydelay[i] = 0;
                    }
                }

                consoleblinkdelay -= multiplier;
                if(consoleblinkdelay <= 0) {
                    consoleblinkdelay = .3;
                    consoleblink = 1 - consoleblink;
                }*/
            }

            if(IsKeyDown(SDL_SCANCODE_Q, KMOD_CTRL)) {
                tryquit = 1;
                if(mainmenu == 3) {
                    if(newdetail > 2)
                        newdetail = config.video_detail;
                    if(newdetail < 0)
                        newdetail = config.video_detail;
                    if(newscreenwidth < 0)
                        newscreenwidth = config.video_resw;
                    if(newscreenheight < 0)
                        newscreenheight = config.video_resh;
                    if(oldgamespeed == 0)
                        oldgamespeed = 1;

                    config.video_resw = newscreenwidth;
                    config.video_resh = newscreenheight;
                    config.video_detail = newdetail;

                    config.save();
                }
            }

            static int oldwinfreeze;
            if(winfreeze && !oldwinfreeze) {
                OPENAL_SetFrequency(OPENAL_ALL, 1);
                PlaySoundEx(consolesuccesssound, samp[consolesuccesssound], NULL, true);
                OPENAL_SetVolume(channels[consolesuccesssound], 256);
                OPENAL_SetPaused(channels[consolesuccesssound], false);
            }
            if(winfreeze == 0)
                oldwinfreeze = winfreeze;
            else
                oldwinfreeze++;

            if((IsKeyDown(keys.jump) || IsKeyDown(SDL_SCANCODE_SPACE)) && !oldjumpkeydown && !campaign) {
                if(winfreeze)
                    winfreeze = 0;
                oldjumpkeydown = 1;
            }
            if((IsKeyDown(SDL_SCANCODE_ESCAPE)) && !campaign && gameon) {
                if(winfreeze) {
                    mainmenu = 9;
                    gameon = 0;
                }
            }
            if((IsKeyDown(keys.jump) || IsKeyDown(SDL_SCANCODE_SPACE))) {
                oldjumpkeydown = 1;
            }
            if(!IsKeyDown(keys.jump) && !IsKeyDown(SDL_SCANCODE_SPACE))
                oldjumpkeydown = 0;

            if(!freeze && !winfreeze && !(mainmenu && gameon) && (gameon || !gamestarted)) {

                static bool oldbuttondialogue;

                if(indialogue != -1)
                    talkdelay = 1;
                talkdelay -= multiplier;

                if(talkdelay <= 0)
                    if(indialogue == -1 && (animation[player[0].targetanimation].height != highheight) /*&&!hostile*/)
                        for(i = 0; i < numdialogues; i++) {
                            int realdialoguetype;
                            bool special;
                            if(dialoguetype[i] > 49) {
                                realdialoguetype = dialoguetype[i] - 50;
                                special = 1;
                            } else if(dialoguetype[i] > 39) {
                                realdialoguetype = dialoguetype[i] - 40;
                                special = 1;
                            } else if(dialoguetype[i] > 29) {
                                realdialoguetype = dialoguetype[i] - 30;
                                special = 1;
                            } else if(dialoguetype[i] > 19) {
                                realdialoguetype = dialoguetype[i] - 20;
                                special = 1;
                            } else if(dialoguetype[i] > 9) {
                                realdialoguetype = dialoguetype[i] - 10;
                                special = 1;
                            } else {
                                realdialoguetype = dialoguetype[i];
                                special = 0;
                            }
                            if((!hostile || (dialoguetype[i] > 40 && dialoguetype[i] < 50)) &&
                               realdialoguetype < numplayers && realdialoguetype > 0 &&
                               (dialoguegonethrough[i] == 0 || !special) &&
                               (special || (IsButtonPressed(keys.attack) && !oldbuttondialogue))) {
                                if(findDistancefast(&player[0].coords, &player[realdialoguetype].coords) < 6 ||
                                   player[realdialoguetype].howactive >= typedead1 ||
                                   (dialoguetype[i] > 40 && dialoguetype[i] < 50)) {
                                    whichdialogue = i;
                                    for(j = 0; j < numdialogueboxes[whichdialogue]; j++) {
                                        player[participantfocus[whichdialogue][j]].coords =
                                            participantlocation[whichdialogue][participantfocus[whichdialogue][j]];
                                        player[participantfocus[whichdialogue][j]].rotation =
                                            participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
                                        player[participantfocus[whichdialogue][j]].targetrotation =
                                            participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
                                        player[participantfocus[whichdialogue][j]].velocity = 0;
                                        player[participantfocus[whichdialogue][j]].targetanimation =
                                            player[participantfocus[whichdialogue][j]].getIdle();
                                        player[participantfocus[whichdialogue][j]].targetframe = 0;
                                    }
                                    directing = 0;
                                    indialogue = 0;
                                    dialoguetime = 0;
                                    dialoguegonethrough[i]++;
                                    if(dialogueboxsound[whichdialogue][indialogue] != 0) {
                                        static float gLoc[3];
                                        static float vel[3];
                                        XYZ temppos;
                                        temppos = player[participantfocus[whichdialogue][indialogue]].coords;
                                        temppos = temppos - viewer;
                                        Normalise(&temppos);
                                        temppos += viewer;

                                        gLoc[0] = temppos.x;
                                        gLoc[1] = temppos.y;
                                        gLoc[2] = temppos.z;
                                        vel[0] = 0;
                                        vel[1] = 0;
                                        vel[2] = 0;
                                        int whichsoundplay;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 1)
                                            whichsoundplay = rabbitchitter;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 2)
                                            whichsoundplay = rabbitchitter2;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 3)
                                            whichsoundplay = rabbitpainsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 4)
                                            whichsoundplay = rabbitpain1sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 5)
                                            whichsoundplay = rabbitattacksound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 6)
                                            whichsoundplay = rabbitattack2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 7)
                                            whichsoundplay = rabbitattack3sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 8)
                                            whichsoundplay = rabbitattack4sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 9)
                                            whichsoundplay = growlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 10)
                                            whichsoundplay = growl2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 11)
                                            whichsoundplay = snarlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 12)
                                            whichsoundplay = snarl2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 13)
                                            whichsoundplay = barksound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 14)
                                            whichsoundplay = bark2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 15)
                                            whichsoundplay = bark3sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 16)
                                            whichsoundplay = barkgrowlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -1)
                                            whichsoundplay = fireendsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -2)
                                            whichsoundplay = firestartsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -3)
                                            whichsoundplay = consolesuccesssound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -4)
                                            whichsoundplay = consolefailsound;
                                        PlaySoundEx(whichsoundplay, samp[whichsoundplay], NULL, true);
                                        OPENAL_3D_SetAttributes(channels[whichsoundplay], gLoc, vel);
                                        OPENAL_SetVolume(channels[whichsoundplay], 256);
                                        OPENAL_SetPaused(channels[whichsoundplay], false);
                                    }
                                    if(IsButtonPressed(keys.attack))
                                        oldbuttondialogue = 1;
                                }
                            }
                        }

                windvar += multiplier;
                smoketex += multiplier;
                tutorialstagetime += multiplier;

                static float hotspotvisual[40];
                if(numhotspots) {
                    XYZ hotspotsprite;
                    if(editorenabled)
                        for(int i = 0; i < numhotspots; i++)
                            hotspotvisual[i] -= multiplier / 320;

                    for(int i = 0; i < numhotspots; i++) {
                        // if(hotspottype[i]<=10)
                        while(hotspotvisual[i] < 0) {
                            hotspotsprite = 0;
                            hotspotsprite.x = float(abs(rand() % 100000)) / 100000 * hotspotsize[i];
                            hotspotsprite = DoRotation(hotspotsprite, 0, 0, rand() % 360);
                            hotspotsprite = DoRotation(hotspotsprite, 0, rand() % 360, 0);
                            hotspotsprite += hotspot[i];
                            sprites.MakeSprite(breathsprite, hotspotsprite, hotspotsprite * 0, 1, 0.5, 0, 7, 0.4);
                            hotspotvisual[i] += 0.1 / hotspotsize[i] / hotspotsize[i] / hotspotsize[i];
                        }
                    }

                    for(int i = 0; i < numhotspots; i++) {
                        if(hotspottype[i] <= 10 && hotspottype[i] > 0) {
                            hotspot[i] = player[hotspottype[i]].coords;
                        }
                    }
                }

                // Tutorial
                if(tutoriallevel && tutorialstagetime > tutorialmaxtime) {
                    tutorialstage++;
                    tutorialsuccess = 0;
                    if(tutorialstage <= 1) {
                        canattack = 0;
                        cananger = 0;
                        reversaltrain = 0;
                    }
                    if(tutorialstage == 1) {
                        tutorialmaxtime = 5;
                    }
                    if(tutorialstage == 2) {
                        tutorialmaxtime = 2;
                    }
                    if(tutorialstage == 3) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 4) {
                        tutorialmaxtime = 1000;
                    }
                    if(tutorialstage == 5) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 6) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 7) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 8) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 9) {
                        tutorialmaxtime = 600;
                    }
                    if(tutorialstage == 10) {
                        tutorialmaxtime = 2;
                    }
                    if(tutorialstage == 11) {
                        tutorialmaxtime = 1000;
                    }
                    if(tutorialstage == 12) {
                        tutorialmaxtime = 1000;
                    }
                    if(tutorialstage == 13) {
                        tutorialmaxtime = 2;
                    }
                    if(tutorialstage == 14) {
                        tutorialmaxtime = 3;

                        XYZ temp, temp2;

                        temp.x = 1011;
                        temp.y = 84;
                        temp.z = 491;
                        temp2.x = 1025;
                        temp2.y = 75;
                        temp2.z = 447;

                        player[1].coords = (temp + temp2) / 2;

                        float gLoc[3];
                        float vel[3];
                        gLoc[0] = player[1].coords.x;
                        gLoc[1] = player[1].coords.y;
                        gLoc[2] = player[1].coords.z;
                        vel[0] = 0;
                        vel[1] = 0;
                        vel[2] = 0;
                        PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                        OPENAL_SetVolume(channels[fireendsound], 256);
                        OPENAL_SetPaused(channels[fireendsound], false);

                        for(i = 0; i < player[1].skeleton.num_joints; i++) {
                            if(rand() % 2 == 0) {
                                if(!player[1].skeleton.free)
                                    temp2 = (player[1].coords - player[1].oldcoords) / multiplier / 2; // velocity/2;
                                if(player[1].skeleton.free)
                                    temp2 = player[1].skeleton.joints[i].velocity * player[1].scale / 2;
                                if(!player[1].skeleton.free)
                                    temp = DoRotation(
                                               DoRotation(
                                                   DoRotation(player[1].skeleton.joints[i].position, 0, 0, player[1].tilt),
                                                   player[1].tilt2,
                                                   0,
                                                   0),
                                               0,
                                               player[1].rotation,
                                               0) *
                                               player[1].scale +
                                           player[1].coords;
                                if(player[1].skeleton.free)
                                    temp = player[1].skeleton.joints[i].position * player[1].scale + player[1].coords;
                                sprites.MakeSprite(
                                    breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(rand() % 100) / 200 - .25, 1);
                            }
                        }
                    }
                    if(tutorialstage == 15) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 16) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 17) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 18) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 19) {
                        tutorialstage = 20;
                        // tutorialmaxtime=500;
                    }
                    if(tutorialstage == 20) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 21) {
                        tutorialmaxtime = 500;
                        if(bonus == cannon) {
                            bonus = Slicebonus;
                            againbonus = 1;
                        } else
                            againbonus = 0;
                    }
                    if(tutorialstage == 22) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 23) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 24) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 25) {
                        tutorialmaxtime = 500;
                    }
                    if(tutorialstage == 26) {
                        tutorialmaxtime = 2;
                    }
                    if(tutorialstage == 27) {
                        tutorialmaxtime = 4;
                        reversaltrain = 1;
                        cananger = 1;
                        player[1].aitype = attacktypecutoff;
                    }
                    if(tutorialstage == 28) {
                        tutorialmaxtime = 400;
                    }
                    if(tutorialstage == 29) {
                        tutorialmaxtime = 400;
                        player[0].escapednum = 0;
                    }
                    if(tutorialstage == 30) {
                        tutorialmaxtime = 4;
                        reversaltrain = 0;
                        cananger = 0;
                        player[1].aitype = passivetype;
                    }
                    if(tutorialstage == 31) {
                        tutorialmaxtime = 13;
                    }
                    if(tutorialstage == 32) {
                        tutorialmaxtime = 8;
                    }
                    if(tutorialstage == 33) {
                        tutorialmaxtime = 400;
                        cananger = 1;
                        canattack = 1;
                        player[1].aitype = attacktypecutoff;
                    }
                    if(tutorialstage == 34) {
                        tutorialmaxtime = 400;
                    }
                    if(tutorialstage == 35) {
                        tutorialmaxtime = 400;
                    }
                    if(tutorialstage == 36) {
                        tutorialmaxtime = 2;
                        reversaltrain = 0;
                        cananger = 0;
                        player[1].aitype = passivetype;
                    }
                    if(tutorialstage == 37) {
                        damagedealt = 0;
                        damagetaken = 0;
                        tutorialmaxtime = 50;
                        cananger = 1;
                        canattack = 1;
                        player[1].aitype = attacktypecutoff;
                    }
                    if(tutorialstage == 38) {
                        tutorialmaxtime = 4;
                        canattack = 0;
                        cananger = 0;
                        player[1].aitype = passivetype;
                    }
                    if(tutorialstage == 39) {
                        XYZ temp, temp2;

                        temp.x = 1011;
                        temp.y = 84;
                        temp.z = 491;
                        temp2.x = 1025;
                        temp2.y = 75;
                        temp2.z = 447;

                        weapons.owner[weapons.numweapons] = -1;
                        weapons.type[weapons.numweapons] = knife;
                        weapons.damage[weapons.numweapons] = 0;
                        weapons.mass[weapons.numweapons] = 1;
                        weapons.tipmass[weapons.numweapons] = 1.2;
                        weapons.length[weapons.numweapons] = .25;
                        weapons.position[weapons.numweapons] = (temp + temp2) / 2;
                        weapons.tippoint[weapons.numweapons] = (temp + temp2) / 2;

                        weapons.velocity[weapons.numweapons] = 0.1;
                        weapons.tipvelocity[weapons.numweapons] = 0.1;
                        weapons.missed[weapons.numweapons] = 1;
                        weapons.hitsomething[weapons.numweapons] = 0;
                        weapons.freetime[weapons.numweapons] = 0;
                        weapons.firstfree[weapons.numweapons] = 1;
                        weapons.physics[weapons.numweapons] = 1;

                        weapons.numweapons++;
                    }
                    if(tutorialstage == 40) {
                        tutorialmaxtime = 300;
                    }
                    if(tutorialstage == 41) {
                        tutorialmaxtime = 300;
                    }
                    if(tutorialstage == 42) {
                        tutorialmaxtime = 8;
                    }
                    if(tutorialstage == 43) {
                        tutorialmaxtime = 300;
                    }
                    if(tutorialstage == 44) {
                        weapons.owner[0] = 1;
                        player[0].weaponactive = -1;
                        player[0].numweapons = 0;
                        player[1].weaponactive = 0;
                        player[1].numweapons = 1;
                        player[1].weaponids[0] = 0;

                        cananger = 1;
                        canattack = 1;
                        player[1].aitype = attacktypecutoff;

                        tutorialmaxtime = 300;
                    }
                    if(tutorialstage == 45) {
                        weapons.owner[0] = 1;
                        player[0].weaponactive = -1;
                        player[0].numweapons = 0;
                        player[1].weaponactive = 0;
                        player[1].numweapons = 1;
                        player[1].weaponids[0] = 0;

                        tutorialmaxtime = 300;
                    }
                    if(tutorialstage == 46) {
                        weapons.owner[0] = 1;
                        player[0].weaponactive = -1;
                        player[0].numweapons = 0;
                        player[1].weaponactive = 0;
                        player[1].numweapons = 1;
                        player[1].weaponids[0] = 0;

                        weapons.type[0] = sword;

                        tutorialmaxtime = 300;
                    }

                    if(tutorialstage == 47) {
                        tutorialmaxtime = 10;

                        XYZ temp, temp2;

                        temp.x = 1011;
                        temp.y = 84;
                        temp.z = 491;
                        temp2.x = 1025;
                        temp2.y = 75;
                        temp2.z = 447;

                        weapons.owner[weapons.numweapons] = -1;
                        weapons.type[weapons.numweapons] = sword;
                        weapons.damage[weapons.numweapons] = 0;
                        weapons.mass[weapons.numweapons] = 1;
                        weapons.tipmass[weapons.numweapons] = 1.2;
                        weapons.length[weapons.numweapons] = .25;
                        weapons.position[weapons.numweapons] = (temp + temp2) / 2;
                        weapons.tippoint[weapons.numweapons] = (temp + temp2) / 2;

                        weapons.velocity[weapons.numweapons] = 0.1;
                        weapons.tipvelocity[weapons.numweapons] = 0.1;
                        weapons.missed[weapons.numweapons] = 1;
                        weapons.hitsomething[weapons.numweapons] = 0;
                        weapons.freetime[weapons.numweapons] = 0;
                        weapons.firstfree[weapons.numweapons] = 1;
                        weapons.physics[weapons.numweapons] = 1;

                        weapons.owner[0] = 1;
                        weapons.owner[1] = 0;
                        player[0].weaponactive = 0;
                        player[0].numweapons = 1;
                        player[0].weaponids[0] = 1;
                        player[1].weaponactive = 0;
                        player[1].numweapons = 1;
                        player[1].weaponids[0] = 0;

                        weapons.numweapons++;
                    }
                    if(tutorialstage == 48) {
                        canattack = 0;
                        cananger = 0;
                        player[1].aitype = passivetype;

                        tutorialmaxtime = 15;

                        weapons.owner[0] = 1;
                        weapons.owner[1] = 0;
                        player[0].weaponactive = 0;
                        player[0].numweapons = 1;
                        player[0].weaponids[0] = 1;
                        player[1].weaponactive = 0;
                        player[1].numweapons = 1;
                        player[1].weaponids[0] = 0;

                        if(player[0].weaponactive != -1)
                            weapons.type[player[0].weaponids[player[0].weaponactive]] = staff;
                        else
                            weapons.type[0] = staff;

                        weapons.numweapons++;
                    }
                    if(tutorialstage == 49) {
                        canattack = 0;
                        cananger = 0;
                        player[1].aitype = passivetype;

                        tutorialmaxtime = 200;

                        weapons.position[1] = 1000;
                        weapons.tippoint[1] = 1000;

                        weapons.numweapons = 1;
                        weapons.owner[0] = 0;
                        player[1].weaponactive = -1;
                        player[1].numweapons = 0;
                        player[0].weaponactive = 0;
                        player[0].numweapons = 1;
                        player[0].weaponids[0] = 0;

                        weapons.type[0] = knife;

                        weapons.numweapons++;
                    }
                    if(tutorialstage == 50) {
                        tutorialmaxtime = 8;

                        XYZ temp, temp2;
                        float gLoc[3];
                        float vel[3];
                        gLoc[0] = player[1].coords.x;
                        gLoc[1] = player[1].coords.y;
                        gLoc[2] = player[1].coords.z;
                        vel[0] = 0;
                        vel[1] = 0;
                        vel[2] = 0;
                        PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                        OPENAL_SetVolume(channels[fireendsound], 256);
                        OPENAL_SetPaused(channels[fireendsound], false);

                        for(i = 0; i < player[1].skeleton.num_joints; i++) {
                            if(rand() % 2 == 0) {
                                if(!player[1].skeleton.free)
                                    temp2 = (player[1].coords - player[1].oldcoords) / multiplier / 2; // velocity/2;
                                if(player[1].skeleton.free)
                                    temp2 = player[1].skeleton.joints[i].velocity * player[1].scale / 2;
                                if(!player[1].skeleton.free)
                                    temp = DoRotation(
                                               DoRotation(
                                                   DoRotation(player[1].skeleton.joints[i].position, 0, 0, player[1].tilt),
                                                   player[1].tilt2,
                                                   0,
                                                   0),
                                               0,
                                               player[1].rotation,
                                               0) *
                                               player[1].scale +
                                           player[1].coords;
                                if(player[1].skeleton.free)
                                    temp = player[1].skeleton.joints[i].position * player[1].scale + player[1].coords;
                                sprites.MakeSprite(
                                    breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(rand() % 100) / 200 - .25, 1);
                            }
                        }

                        player[1].numweapons = 0;
                        player[1].weaponstuck = -1;
                        player[1].weaponactive = -1;

                        weapons.numweapons = 0;

                        weapons.owner[0] = -1;
                        weapons.velocity[0] = 0.1;
                        weapons.tipvelocity[0] = -0.1;
                        weapons.missed[0] = 1;
                        weapons.hitsomething[0] = 0;
                        weapons.freetime[0] = 0;
                        weapons.firstfree[0] = 1;
                        weapons.physics[0] = 1;
                    }
                    if(tutorialstage == 51) {
                        tutorialmaxtime = 80000;
                    }
                    if(tutorialstage <= 51)
                        tutorialstagetime = 0;
                }

                // Tutorial success
                if(tutoriallevel && tutorialstagetime < tutorialmaxtime - 3) {
                    if(tutorialstage == 3) {
                        if(deltah || deltav)
                            tutorialsuccess += multiplier;
                    }
                    if(tutorialstage == 4) {
                        if(player[0].forwardkeydown || player[0].backkeydown || player[0].leftkeydown ||
                           player[0].rightkeydown)
                            tutorialsuccess += multiplier;
                    }
                    if(tutorialstage == 5) {
                        if(player[0].jumpkeydown)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 6) {
                        if(player[0].isCrouch())
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 7) {
                        if(player[0].targetanimation == rollanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 8) {
                        if(player[0].targetanimation == sneakanim)
                            tutorialsuccess += multiplier;
                    }
                    if(tutorialstage == 9) {
                        if(player[0].targetanimation == rabbitrunninganim || player[0].targetanimation == wolfrunninganim)
                            tutorialsuccess += multiplier;
                    }
                    if(tutorialstage == 11) {
                        if(player[0].isWallJump())
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 12) {
                        if(player[0].targetanimation == flipanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 15) {
                        if(player[0].targetanimation == upunchanim || player[0].targetanimation == winduppunchanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 16) {
                        if(player[0].targetanimation == winduppunchanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 17) {
                        if(player[0].targetanimation == spinkickanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 18) {
                        if(player[0].targetanimation == sweepanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 19) {
                        if(player[0].targetanimation == dropkickanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 20) {
                        if(player[0].targetanimation == rabbitkickanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 21) {
                        if(bonus == cannon)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 22) {
                        if(bonus == spinecrusher)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 23) {
                        if(player[0].targetanimation == walljumprightkickanim ||
                           player[0].targetanimation == walljumpleftkickanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 24) {
                        if(player[0].targetanimation == rabbittacklinganim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 25) {
                        if(player[0].targetanimation == backhandspringanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 28) {
                        if(animation[player[0].targetanimation].attack == reversed && player[0].feint)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 29) {
                        if(player[0].escapednum == 2) {
                            tutorialsuccess = 1;
                            reversaltrain = 0;
                            cananger = 0;
                            player[1].aitype = passivetype;
                        }
                    }
                    if(tutorialstage == 33) {
                        if(animation[player[0].targetanimation].attack == reversal)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 34) {
                        if(animation[player[0].targetanimation].attack == reversal)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 35) {
                        if(animation[player[0].targetanimation].attack == reversal) {
                            tutorialsuccess = 1;
                            reversaltrain = 0;
                            cananger = 0;
                            player[1].aitype = passivetype;
                        }
                    }
                    if(tutorialstage == 40) {
                        if(player[0].numweapons > 0)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 41) {
                        if(player[0].weaponactive == -1 && player[0].numweapons > 0)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 43) {
                        if(player[0].targetanimation == knifeslashstartanim)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 44) {
                        if(animation[player[0].targetanimation].attack == reversal)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 45) {
                        if(animation[player[0].targetanimation].attack == reversal)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 46) {
                        if(animation[player[0].targetanimation].attack == reversal)
                            tutorialsuccess = 1;
                    }
                    if(tutorialstage == 49) {
                        if(player[1].weaponstuck != -1)
                            tutorialsuccess = 1;
                    }
                    if(tutorialsuccess >= 1)
                        tutorialstagetime = tutorialmaxtime - 3;

                    if(tutorialstagetime == tutorialmaxtime - 3) {
                        PlaySoundEx(consolesuccesssound, samp[consolesuccesssound], NULL, true);
                        OPENAL_SetVolume(channels[consolesuccesssound], 256);
                        OPENAL_SetPaused(channels[consolesuccesssound], false);
                    }

                    if(tutorialsuccess >= 1) {
                        if(tutorialstage == 34 || tutorialstage == 35)
                            tutorialstagetime = tutorialmaxtime - 1;
                    }
                }

                if(tutoriallevel) {
                    if(tutorialstage < 14 || tutorialstage >= 50) {
                        player[1].coords.y = 300;
                        player[1].velocity = 0;
                    }
                }

                if(tutoriallevel != 1) {
                    if(bonustime == 0 && bonus != solidhit && bonus != spinecrusher && bonus != tracheotomy &&
                       bonus != backstab && bonusvalue > 10) {
                        PlaySoundEx(consolesuccesssound, samp[consolesuccesssound], NULL, true);
                        OPENAL_SetVolume(channels[consolesuccesssound], 256);
                        OPENAL_SetPaused(channels[consolesuccesssound], false);
                    }
                } else if(bonustime == 0) {
                    PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                    OPENAL_SetVolume(channels[fireendsound], 256);
                    OPENAL_SetPaused(channels[fireendsound], false);
                }
                if(bonustime == 0) {
                    if(bonus != solidhit && bonus != twoxcombo && bonus != threexcombo && bonus != fourxcombo &&
                       bonus != megacombo)
                        bonusnum[bonus]++;
                    else
                        bonusnum[bonus] += 0.15;
                    if(tutoriallevel)
                        bonusvalue = 0;
                    bonusvalue /= bonusnum[bonus];
                    bonustotal += bonusvalue;
                }
                bonustime += multiplier;

                if(environment == snowyenvironment) {
                    precipdelay -= multiplier;
                    while(precipdelay < 0) {
                        precipdelay += .04;
                        if(!config.video_detail)
                            precipdelay += .04;
                        XYZ footvel, footpoint;

                        footvel = 0;
                        footpoint = viewer + viewerfacing * 6;
                        footpoint.y += ((float)abs(rand() % 1200)) / 100 - 6;
                        footpoint.x += ((float)abs(rand() % 1200)) / 100 - 6;
                        footpoint.z += ((float)abs(rand() % 1200)) / 100 - 6;
                        sprites.MakeSprite(snowsprite, footpoint, footvel, 1, 1, 1, .1, 1);
                    }
                }
                for(k = 0; k < numplayers; k++) {
                    if(player[k].aitype == playercontrolled)
                        player[k].turnspeed = 500;
                    if(player[k].aitype != playercontrolled)
                        player[k].turnspeed = 500;

                    if((player[k].isRun() &&
                        ((player[k].targetrotation != rabbitrunninganim && player[k].targetrotation != wolfrunninganim) ||
                         player[k].targetframe == 4)) ||
                       player[k].targetanimation == removeknifeanim || player[k].targetanimation == crouchremoveknifeanim ||
                       player[k].targetanimation == flipanim || player[k].targetanimation == fightsidestep ||
                       player[k].targetanimation == walkanim) {
                        if(abs(player[k].rotation - player[k].targetrotation) < multiplier * player[k].turnspeed)
                            player[k].rotation = player[k].targetrotation;
                        else if(player[k].rotation > player[k].targetrotation) {
                            player[k].rotation -= multiplier * player[k].turnspeed;
                        } else if(player[k].rotation < player[k].targetrotation) {
                            player[k].rotation += multiplier * player[k].turnspeed;
                        }
                    }

                    if(player[k].isStop() || player[k].isLanding() || player[k].targetanimation == staggerbackhighanim ||
                       (player[k].targetanimation == sneakanim && player[k].currentanimation == sneakanim) ||
                       player[k].targetanimation == staggerbackhardanim ||
                       player[k].targetanimation == backhandspringanim || player[k].targetanimation == dodgebackanim ||
                       player[k].targetanimation == rollanim ||
                       (animation[player[k].targetanimation].attack && player[k].targetanimation != rabbitkickanim &&
                        (player[k].targetanimation != crouchstabanim || player[k].hasvictim) &&
                        (player[k].targetanimation != swordgroundstabanim || player[k].hasvictim))) {
                        player[k].turnspeed *= 2;
                        if(abs(player[k].rotation - player[k].targetrotation) < multiplier * player[k].turnspeed)
                            player[k].rotation = player[k].targetrotation;
                        else if(player[k].rotation > player[k].targetrotation) {
                            player[k].rotation -= multiplier * player[k].turnspeed;
                        } else if(player[k].rotation < player[k].targetrotation) {
                            player[k].rotation += multiplier * player[k].turnspeed;
                        }
                        player[k].turnspeed /= 2;
                    }

                    if(player[k].targetanimation == sneakanim && player[k].currentanimation != sneakanim) {
                        player[k].turnspeed *= 4;
                        if(abs(player[k].rotation - player[k].targetrotation) < multiplier * player[k].turnspeed)
                            player[k].rotation = player[k].targetrotation;
                        else if(player[k].rotation > player[k].targetrotation) {
                            player[k].rotation -= multiplier * player[k].turnspeed;
                        } else if(player[k].rotation < player[k].targetrotation) {
                            player[k].rotation += multiplier * player[k].turnspeed;
                        }
                        player[k].turnspeed /= 4;
                    }

                    /*if(player[k].aitype!=passivetype||(findDistancefast(&player[k].coords,&viewer)<viewdistance*viewdistance))*/ player
                        [k].DoStuff();
                    if(player[k].immobile && k != 0)
                        player[k].coords = player[k].realoldcoords;

                    if(findDistancefast(&player[k].coords, &player[k].realoldcoords) > 0) {
                        if(!player[k].skeleton.free && player[k].targetanimation != climbanim &&
                           player[k].targetanimation != hanganim) {
                            bool tempcollide = 0;

                            if(player[k].collide < -.3)
                                player[k].collide = -.3;
                            if(player[k].collide > 1)
                                player[k].collide = 1;
                            player[k].collide -= multiplier * 30;

                            if(player[k].coords.y < terrain.getHeight(player[k].coords.x, player[k].coords.z)) {
                                player[k].coords.y = terrain.getHeight(player[k].coords.x, player[k].coords.z);
                            }
                            if(terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz])
                                for(l = 0; l < terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz]; l++) {
                                    i = terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
                                    if(objects.type[i] != rocktype ||
                                       (((objects.scale[i] > .5 && player[k].aitype == playercontrolled) ||
                                         objects.position[i].y > player[k].coords.y))) {
                                        lowpoint = player[k].coords;
                                        if(player[k].targetanimation != jumpupanim &&
                                           player[k].targetanimation != jumpdownanim && !player[k].isFlip())
                                            lowpoint.y += 1.25;
                                        else
                                            lowpoint.y += 1.3;
                                        if(player[k].coords.y < terrain.getHeight(player[k].coords.x, player[k].coords.z) &&
                                           player[k].coords.y >
                                               terrain.getHeight(player[k].coords.x, player[k].coords.z) - .1) {
                                            player[k].coords.y = terrain.getHeight(player[k].coords.x, player[k].coords.z);
                                        }
                                        /*while(player[k].coords.y<terrain.getHeight(player[k].coords.x,player[k].coords.z))
                                        player[k].coords+=terrain.getNormal(player[k].coords.x,player[k].coords.z)/50;
                                        */
                                        if(player[k].SphereCheck(&lowpoint,
                                                                 1.3,
                                                                 &colpoint,
                                                                 &objects.position[i],
                                                                 &objects.rotation[i],
                                                                 &objects.model[i]) != -1) {
                                            flatfacing = lowpoint - player[k].coords;
                                            player[k].coords = lowpoint;
                                            player[k].coords.y -= 1.3;
                                            player[k].collide = 1;
                                            tempcollide = 1;
                                            if(player[k].aitype == playercontrolled &&
                                               (player[k].targetanimation == jumpupanim ||
                                                player[k].targetanimation == jumpdownanim || player[k].isFlip()) &&
                                               !player[k].jumptogglekeydown && player[k].jumpkeydown) {
                                                lowpointtarget = lowpoint + DoRotation(player[k].facing, 0, -90, 0) * 1.5;
                                                tempcoords1 = lowpoint;
                                                whichhit = objects.model[i].LineCheck(&lowpoint,
                                                                                      &lowpointtarget,
                                                                                      &colpoint,
                                                                                      &objects.position[i],
                                                                                      &objects.rotation[i]);
                                                if(whichhit != -1 && abs(objects.model[i].facenormals[whichhit].y) < .3) {
                                                    player[k].target = 0;
                                                    player[k].targetanimation = walljumpleftanim;
                                                    player[k].targetframe = 0;
                                                    float gLoc[3];
                                                    float vel[3];
                                                    gLoc[0] = player[k].coords.x;
                                                    gLoc[1] = player[k].coords.y;
                                                    gLoc[2] = player[k].coords.z;
                                                    vel[0] = 0;
                                                    vel[1] = 0;
                                                    vel[2] = 0;
                                                    PlaySoundEx(movewhooshsound, samp[movewhooshsound], NULL, true);
                                                    OPENAL_3D_SetAttributes(channels[movewhooshsound], gLoc, vel);
                                                    OPENAL_SetVolume(channels[movewhooshsound], 256);
                                                    OPENAL_SetPaused(channels[movewhooshsound], false);
                                                    if(k == 0)
                                                        OPENAL_SetPaused(channels[whooshsound], true);

                                                    lowpointtarget = DoRotation(
                                                        objects.model[i].facenormals[whichhit], 0, objects.rotation[i], 0);
                                                    player[k].rotation = -asin(0 - lowpointtarget.x);
                                                    player[k].rotation *= 360 / 6.28;
                                                    if(lowpointtarget.z < 0)
                                                        player[k].rotation = 180 - player[k].rotation;
                                                    player[k].targetrotation = player[k].rotation;
                                                    player[k].lowrotation = player[k].rotation;
                                                    if(k == 0)
                                                        numwallflipped++;
                                                } else {
                                                    lowpoint = tempcoords1;
                                                    lowpointtarget =
                                                        lowpoint + DoRotation(player[k].facing, 0, 90, 0) * 1.5;
                                                    whichhit = objects.model[i].LineCheck(&lowpoint,
                                                                                          &lowpointtarget,
                                                                                          &colpoint,
                                                                                          &objects.position[i],
                                                                                          &objects.rotation[i]);
                                                    if(whichhit != -1 &&
                                                       abs(objects.model[i].facenormals[whichhit].y) < .3) {
                                                        player[k].target = 0;
                                                        player[k].targetanimation = walljumprightanim;
                                                        player[k].targetframe = 0;
                                                        float gLoc[3];
                                                        float vel[3];
                                                        gLoc[0] = player[k].coords.x;
                                                        gLoc[1] = player[k].coords.y;
                                                        gLoc[2] = player[k].coords.z;
                                                        vel[0] = 0;
                                                        vel[1] = 0;
                                                        vel[2] = 0;
                                                        PlaySoundEx(movewhooshsound, samp[movewhooshsound], NULL, true);
                                                        OPENAL_3D_SetAttributes(channels[movewhooshsound], gLoc, vel);
                                                        OPENAL_SetVolume(channels[movewhooshsound], 256);
                                                        OPENAL_SetPaused(channels[movewhooshsound], false);
                                                        if(k == 0)
                                                            OPENAL_SetPaused(channels[whooshsound], true);

                                                        lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit],
                                                                                    0,
                                                                                    objects.rotation[i],
                                                                                    0);
                                                        player[k].rotation = -asin(0 - lowpointtarget.x);
                                                        player[k].rotation *= 360 / 6.28;
                                                        if(lowpointtarget.z < 0)
                                                            player[k].rotation = 180 - player[k].rotation;
                                                        player[k].targetrotation = player[k].rotation;
                                                        player[k].lowrotation = player[k].rotation;
                                                        if(k == 0)
                                                            numwallflipped++;
                                                    } else {
                                                        lowpoint = tempcoords1;
                                                        lowpointtarget = lowpoint + player[k].facing * 2;
                                                        whichhit = objects.model[i].LineCheck(&lowpoint,
                                                                                              &lowpointtarget,
                                                                                              &colpoint,
                                                                                              &objects.position[i],
                                                                                              &objects.rotation[i]);
                                                        if(whichhit != -1 &&
                                                           abs(objects.model[i].facenormals[whichhit].y) < .3) {
                                                            player[k].target = 0;
                                                            player[k].targetanimation = walljumpbackanim;
                                                            player[k].targetframe = 0;
                                                            float gLoc[3];
                                                            float vel[3];
                                                            gLoc[0] = player[k].coords.x;
                                                            gLoc[1] = player[k].coords.y;
                                                            gLoc[2] = player[k].coords.z;
                                                            vel[0] = 0;
                                                            vel[1] = 0;
                                                            vel[2] = 0;
                                                            PlaySoundEx(movewhooshsound, samp[movewhooshsound], NULL, true);
                                                            OPENAL_3D_SetAttributes(channels[movewhooshsound], gLoc, vel);
                                                            OPENAL_SetVolume(channels[movewhooshsound], 256);
                                                            OPENAL_SetPaused(channels[movewhooshsound], false);
                                                            if(k == 0)
                                                                OPENAL_SetPaused(channels[whooshsound], true);

                                                            lowpointtarget =
                                                                DoRotation(objects.model[i].facenormals[whichhit],
                                                                           0,
                                                                           objects.rotation[i],
                                                                           0);
                                                            player[k].rotation = -asin(0 - lowpointtarget.x);
                                                            player[k].rotation *= 360 / 6.28;
                                                            if(lowpointtarget.z < 0)
                                                                player[k].rotation = 180 - player[k].rotation;
                                                            player[k].targetrotation = player[k].rotation;
                                                            player[k].lowrotation = player[k].rotation;
                                                            if(k == 0)
                                                                numwallflipped++;
                                                        } else {
                                                            lowpoint = tempcoords1;
                                                            lowpointtarget = lowpoint - player[k].facing * 2;
                                                            whichhit = objects.model[i].LineCheck(&lowpoint,
                                                                                                  &lowpointtarget,
                                                                                                  &colpoint,
                                                                                                  &objects.position[i],
                                                                                                  &objects.rotation[i]);
                                                            if(whichhit != -1 &&
                                                               abs(objects.model[i].facenormals[whichhit].y) < .3) {
                                                                player[k].target = 0;
                                                                player[k].targetanimation = walljumpfrontanim;
                                                                player[k].targetframe = 0;
                                                                float gLoc[3];
                                                                float vel[3];
                                                                gLoc[0] = player[k].coords.x;
                                                                gLoc[1] = player[k].coords.y;
                                                                gLoc[2] = player[k].coords.z;
                                                                vel[0] = 0;
                                                                vel[1] = 0;
                                                                vel[2] = 0;
                                                                PlaySoundEx(
                                                                    movewhooshsound, samp[movewhooshsound], NULL, true);
                                                                OPENAL_3D_SetAttributes(
                                                                    channels[movewhooshsound], gLoc, vel);
                                                                OPENAL_SetVolume(channels[movewhooshsound], 256);
                                                                OPENAL_SetPaused(channels[movewhooshsound], false);
                                                                if(k == 0)
                                                                    OPENAL_SetPaused(channels[whooshsound], true);

                                                                lowpointtarget =
                                                                    DoRotation(objects.model[i].facenormals[whichhit],
                                                                               0,
                                                                               objects.rotation[i],
                                                                               0);
                                                                player[k].rotation = -asin(0 - lowpointtarget.x);
                                                                player[k].rotation *= 360 / 6.28;
                                                                if(lowpointtarget.z < 0)
                                                                    player[k].rotation = 180 - player[k].rotation;
                                                                player[k].rotation += 180;
                                                                player[k].targetrotation = player[k].rotation;
                                                                player[k].lowrotation = player[k].rotation;
                                                                if(k == 0)
                                                                    numwallflipped++;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    } else if(objects.type[i] == rocktype) {
                                        lowpoint2 = player[k].coords;
                                        lowpoint = player[k].coords;
                                        lowpoint.y += 2;
                                        if(objects.model[i].LineCheck(&lowpoint,
                                                                      &lowpoint2,
                                                                      &colpoint,
                                                                      &objects.position[i],
                                                                      &objects.rotation[i]) != -1) {
                                            player[k].coords = colpoint;
                                            player[k].collide = 1;
                                            tempcollide = 1;

                                            if((player[k].targetanimation == jumpdownanim || player[k].isFlip())) {
                                                if(player[k].isFlip() &&
                                                   animation[player[k].targetanimation].label[player[k].targetframe] == 7)
                                                    player[k].RagDoll(0);

                                                if(player[k].targetanimation == jumpupanim) {
                                                    player[k].jumppower = -4;
                                                    player[k].targetanimation = player[k].getIdle();
                                                }
                                                player[k].target = 0;
                                                player[k].targetframe = 0;
                                                player[k].onterrain = 1;

                                                if(player[k].id == 0) {
                                                    OPENAL_SetPaused(channels[whooshsound], true);
                                                    OPENAL_SetVolume(channels[whooshsound], 0);
                                                }

                                                if((player[k].targetanimation == jumpdownanim || player[k].isFlip()) &&
                                                   !player[k].wasLanding()) {
                                                    if(player[k].isFlip())
                                                        player[k].jumppower = -4;
                                                    player[k].targetanimation = player[k].getLanding();
                                                    float gLoc[3];
                                                    float vel[3];
                                                    gLoc[0] = player[k].coords.x;
                                                    gLoc[1] = player[k].coords.y;
                                                    gLoc[2] = player[k].coords.z;
                                                    vel[0] = player[k].velocity.x;
                                                    vel[1] = player[k].velocity.y;
                                                    vel[2] = player[k].velocity.z;
                                                    PlaySoundEx(landsound, samp[landsound], NULL, true);
                                                    OPENAL_3D_SetAttributes(channels[landsound], gLoc, vel);
                                                    OPENAL_SetVolume(channels[landsound], 128);
                                                    OPENAL_SetPaused(channels[landsound], false);
                                                    if(k == 0) {
                                                        envsound[numenvsounds] = player[k].coords;
                                                        envsoundvol[numenvsounds] = 16;
                                                        envsoundlife[numenvsounds] = .4;
                                                        numenvsounds++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            if(tempcollide && terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz] &&
                               (/*player[k].jumptogglekeydown*/ 1 == 1 || player[k].aitype != playercontrolled))
                                for(l = 0; l < terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz]; l++) {
                                    i = terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
                                    lowpoint = player[k].coords;
                                    lowpoint.y += 1.35;
                                    if(objects.type[i] != rocktype)
                                        if(player[k].SphereCheck(&lowpoint,
                                                                 1.33,
                                                                 &colpoint,
                                                                 &objects.position[i],
                                                                 &objects.rotation[i],
                                                                 &objects.model[i]) != -1) {
                                            if(player[k].targetanimation != jumpupanim &&
                                               player[k].targetanimation != jumpdownanim && player[k].onterrain)
                                                player[k].avoidcollided = 1;
                                            player[k].coords = lowpoint;
                                            player[k].coords.y -= 1.35;
                                            player[k].collide = 1;

                                            if((player[k].grabdelay <= 0 || player[k].aitype != playercontrolled) &&
                                               ((/*(player[k].isRun()||player[k].targetanimation==sneakanim||player[k].targetanimation==walkanim)&&*/ player
                                                         [k].currentanimation != climbanim &&
                                                 player[k].currentanimation != hanganim && !player[k].isWallJump()) ||
                                                player[k].targetanimation == jumpupanim ||
                                                player[k].targetanimation == jumpdownanim)) {
                                                lowpoint = player[k].coords;
                                                objects.model[i].SphereCheckPossible(
                                                    &lowpoint, 1.5, &objects.position[i], &objects.rotation[i]);
                                                lowpoint = player[k].coords;
                                                lowpoint.y += .05;
                                                facing = 0;
                                                facing.z = -1;
                                                facing = DoRotation(facing, 0, player[k].targetrotation + 180, 0);
                                                lowpointtarget = lowpoint + facing * 1.4;
                                                whichhit = objects.model[i].LineCheckPossible(&lowpoint,
                                                                                              &lowpointtarget,
                                                                                              &colpoint,
                                                                                              &objects.position[i],
                                                                                              &objects.rotation[i]);
                                                if(whichhit != -1) {
                                                    lowpoint = player[k].coords;
                                                    lowpoint.y += .1;
                                                    lowpointtarget = lowpoint + facing * 1.4;
                                                    lowpoint2 = lowpoint;
                                                    lowpointtarget2 = lowpointtarget;
                                                    lowpoint3 = lowpoint;
                                                    lowpointtarget3 = lowpointtarget;
                                                    lowpoint4 = lowpoint;
                                                    lowpointtarget4 = lowpointtarget;
                                                    lowpoint5 = lowpoint;
                                                    lowpointtarget5 = lowpointtarget;
                                                    lowpoint6 = lowpoint;
                                                    lowpointtarget6 = lowpointtarget;
                                                    lowpoint7 = lowpoint;
                                                    lowpointtarget7 = lowpoint;
                                                    lowpoint2.x += .1;
                                                    lowpointtarget2.x += .1;
                                                    lowpoint3.z += .1;
                                                    lowpointtarget3.z += .1;
                                                    lowpoint4.x -= .1;
                                                    lowpointtarget4.x -= .1;
                                                    lowpoint5.z -= .1;
                                                    lowpointtarget5.z -= .1;
                                                    lowpoint6.y += 45 / 13;
                                                    lowpointtarget6.y += 45 / 13;
                                                    lowpointtarget6 += facing * .6;
                                                    lowpointtarget7.y += 90 / 13;
                                                    whichhit = objects.model[i].LineCheckPossible(&lowpoint,
                                                                                                  &lowpointtarget,
                                                                                                  &colpoint,
                                                                                                  &objects.position[i],
                                                                                                  &objects.rotation[i]);
                                                    if(objects.friction[i] > .5)
                                                        if(whichhit != -1) {
                                                            // if(k==0){
                                                            if(whichhit != -1)
                                                                if(player[k].targetanimation != jumpupanim &&
                                                                   player[k].targetanimation != jumpdownanim)
                                                                    player[k].collided = 1;
                                                            if(checkcollide(lowpoint7, lowpointtarget7) == -1)
                                                                if(checkcollide(lowpoint6, lowpointtarget6) == -1)
                                                                    if(objects.model[i].LineCheckPossible(
                                                                           &lowpoint2,
                                                                           &lowpointtarget2,
                                                                           &colpoint,
                                                                           &objects.position[i],
                                                                           &objects.rotation[i]) != -1 &&
                                                                       objects.model[i].LineCheckPossible(
                                                                           &lowpoint3,
                                                                           &lowpointtarget3,
                                                                           &colpoint,
                                                                           &objects.position[i],
                                                                           &objects.rotation[i]) != -1 &&
                                                                       objects.model[i].LineCheckPossible(
                                                                           &lowpoint4,
                                                                           &lowpointtarget4,
                                                                           &colpoint,
                                                                           &objects.position[i],
                                                                           &objects.rotation[i]) != -1 &&
                                                                       objects.model[i].LineCheckPossible(
                                                                           &lowpoint5,
                                                                           &lowpointtarget5,
                                                                           &colpoint,
                                                                           &objects.position[i],
                                                                           &objects.rotation[i]) != -1)
                                                                        for(j = 0; j < 45; j++) {
                                                                            lowpoint = player[k].coords;
                                                                            lowpoint.y += (float)j / 13;
                                                                            lowpointtarget = lowpoint + facing * 1.4;
                                                                            if(objects.model[i].LineCheckPossible(
                                                                                   &lowpoint,
                                                                                   &lowpointtarget,
                                                                                   &colpoint2,
                                                                                   &objects.position[i],
                                                                                   &objects.rotation[i]) == -1) {
                                                                                if(j <= 6) {
                                                                                    j = 100;
                                                                                }
                                                                                /*if(j>=25&&(player[k].isRun()||player[k].targetanimation==sneakanim||player[k].targetanimation==walkanim)){
                                                                                j=100;
                                                                                }*/
                                                                                if(j <= 25 &&
                                                                                   player[k].targetanimation ==
                                                                                       jumpdownanim) {
                                                                                    j = 100;
                                                                                }
                                                                                if(j != 100 &&
                                                                                   (/*j>25||(player[k].isRun()||player[k].targetanimation==sneakanim||player[k].targetanimation==walkanim)||*/ player
                                                                                            [k].targetanimation ==
                                                                                        jumpupanim ||
                                                                                    player[k].targetanimation ==
                                                                                        jumpdownanim)) {
                                                                                    lowpoint = DoRotation(
                                                                                        objects.model[i]
                                                                                            .facenormals[whichhit],
                                                                                        0,
                                                                                        objects.rotation[k],
                                                                                        0);
                                                                                    if(1 ==
                                                                                       1 /*dotproduct(&player[k].velocity,&lowpoint)>0||player[k].aitype!=playercontrolled||(player[k].isRun()||player[k].targetanimation==sneakanim||player[k].targetanimation==walkanim||player[k].targetanimation==jumpupanim)*/) {
                                                                                        lowpoint = player[k].coords;
                                                                                        lowpoint.y += (float)j / 13;
                                                                                        lowpointtarget =
                                                                                            lowpoint + facing * 1.3;
                                                                                        flatfacing = player[k].coords;
                                                                                        player[k].coords =
                                                                                            colpoint -
                                                                                            DoRotation(
                                                                                                objects.model[i]
                                                                                                    .facenormals[whichhit],
                                                                                                0,
                                                                                                objects.rotation[k],
                                                                                                0) *
                                                                                                .01;
                                                                                        player[k].coords.y =
                                                                                            lowpointtarget.y - .07;
                                                                                        player[k].currentoffset =
                                                                                            (flatfacing -
                                                                                             player[k].coords) /
                                                                                            player[k].scale;

                                                                                        if(j > 10 || !player[k].isRun()) {
                                                                                            if(player[k].targetanimation ==
                                                                                                   jumpdownanim ||
                                                                                               player[k].targetanimation ==
                                                                                                   jumpupanim) {
                                                                                                if(k == 0)
                                                                                                    OPENAL_SetPaused(
                                                                                                        channels
                                                                                                            [whooshsound],
                                                                                                        true);
                                                                                            }
                                                                                            float gLoc[3];
                                                                                            float vel[3];
                                                                                            gLoc[0] = player[k].coords.x;
                                                                                            gLoc[1] = player[k].coords.y;
                                                                                            gLoc[2] = player[k].coords.z;
                                                                                            vel[0] = player[k].velocity.x;
                                                                                            vel[1] = player[k].velocity.y;
                                                                                            vel[2] = player[k].velocity.z;
                                                                                            PlaySoundEx(jumpsound,
                                                                                                        samp[jumpsound],
                                                                                                        NULL,
                                                                                                        true);
                                                                                            OPENAL_3D_SetAttributes(
                                                                                                channels[jumpsound],
                                                                                                gLoc,
                                                                                                vel);
                                                                                            OPENAL_SetVolume(
                                                                                                channels[jumpsound], 128);
                                                                                            OPENAL_SetPaused(
                                                                                                channels[jumpsound], false);

                                                                                            lowpointtarget = DoRotation(
                                                                                                objects.model[i]
                                                                                                    .facenormals[whichhit],
                                                                                                0,
                                                                                                objects.rotation[i],
                                                                                                0);
                                                                                            player[k].rotation =
                                                                                                -asin(0 - lowpointtarget.x);
                                                                                            player[k].rotation *=
                                                                                                360 / 6.28;
                                                                                            if(lowpointtarget.z < 0)
                                                                                                player[k].rotation =
                                                                                                    180 -
                                                                                                    player[k].rotation;
                                                                                            player[k].targetrotation =
                                                                                                player[k].rotation;
                                                                                            player[k].lowrotation =
                                                                                                player[k].rotation;

                                                                                            // player[k].velocity=lowpointtarget*.03;
                                                                                            player[k].velocity = 0;

                                                                                            if(/*(player[k].isRun()||player[k].targetanimation==sneakanim||player[k].targetanimation==walkanim)||*/ player
                                                                                                   [k].targetanimation ==
                                                                                               jumpupanim) {
                                                                                                // player[k].currentanimation=climbanim;
                                                                                                player[k].targetanimation =
                                                                                                    climbanim;
                                                                                                player[k].jumppower = 0;
                                                                                                player[k].jumpclimb = 1;
                                                                                            }
                                                                                            player[k].transspeed = 6;
                                                                                            player[k].target = 0;

                                                                                            // player[k].currentframe=1;
                                                                                            player[k].targetframe = 1;
                                                                                            if(j > 25) {
                                                                                                // player[k].currentframe=0;
                                                                                                player[k].targetframe = 0;
                                                                                                // player[k].currentanimation=hanganim;
                                                                                                player[k].targetanimation =
                                                                                                    hanganim;
                                                                                                player[k].jumppower = 0;
                                                                                            }
                                                                                        }
                                                                                        j = 100;
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                            //}
                                                        }
                                                }
                                            }
                                        }
                                }
                            if(player[k].collide <= 0) {
                                if(!player[k].onterrain && player[k].targetanimation != jumpupanim &&
                                   player[k].targetanimation != jumpdownanim && player[k].targetanimation != climbanim &&
                                   player[k].targetanimation != hanganim && !player[k].isWallJump() &&
                                   !player[k].isFlip()) {
                                    if(player[k].currentanimation != climbanim && player[k].currentanimation != tempanim &&
                                       player[k].targetanimation != backhandspringanim &&
                                       (player[k].targetanimation != rollanim || player[k].targetframe < 2 ||
                                        player[k].targetframe > 6)) {
                                        if(player[k].targetanimation == staggerbackhighanim ||
                                           player[k].targetanimation == staggerbackhardanim)
                                            player[k].RagDoll(0);
                                        player[k].targetanimation = jumpdownanim;
                                        player[k].targetframe = 0;
                                        player[k].target = 0;

                                        float gLoc[3];
                                        float vel[3];
                                        gLoc[0] = player[k].coords.x;
                                        gLoc[1] = player[k].coords.y;
                                        gLoc[2] = player[k].coords.z;
                                        vel[0] = player[k].velocity.x;
                                        vel[1] = player[k].velocity.y;
                                        vel[2] = player[k].velocity.z;
                                        if(k == 0) {
                                            PlaySoundEx(whooshsound, samp[whooshsound], NULL, true);
                                            OPENAL_3D_SetAttributes(channels[whooshsound], gLoc, vel);
                                            OPENAL_SetVolume(channels[whooshsound], 128);
                                            OPENAL_SetPaused(channels[whooshsound], false);
                                        }
                                    }
                                    player[k].velocity.y += gravity;
                                }
                            }
                        }
                    }
                    player[k].realoldcoords = player[k].coords;
                }

                static XYZ oldviewer;

                if(indialogue == -1) {
                    player[0].forwardkeydown = IsKeyDown(keys.forward);
                    player[0].leftkeydown = IsKeyDown(keys.left);
                    player[0].backkeydown = IsKeyDown(keys.back);
                    player[0].rightkeydown = IsKeyDown(keys.left);
                    player[0].jumpkeydown = IsKeyDown(keys.jump);
                    player[0].crouchkeydown = IsKeyDown(keys.crouch);
                    player[0].drawkeydown = IsKeyDown(keys.draw);
                    player[0].throwkeydown = IsKeyDown(keys.throww);
                } else {
                    player[0].forwardkeydown = 0;
                    player[0].leftkeydown = 0;
                    player[0].backkeydown = 0;
                    player[0].rightkeydown = 0;
                    player[0].jumpkeydown = 0;
                    player[0].crouchkeydown = 0;
                    player[0].drawkeydown = 0;
                    player[0].throwkeydown = 0;
                }

                if(!player[0].jumpkeydown)
                    player[0].jumpclimb = 0;

                static bool endkeydown;
                if(indialogue != -1) {
                    cameramode = 1;
                    if(directing) {
                        facing = 0;
                        facing.z = -1;

                        facing = DoRotation(facing, -rotation2, 0, 0);
                        facing = DoRotation(facing, 0, 0 - rotation, 0);

                        flatfacing = 0;
                        flatfacing.z = -1;

                        flatfacing = DoRotation(flatfacing, 0, -rotation, 0);

                        if(IsKeyDown(keys.forward))
                            viewer += facing * multiplier * 4;
                        if(IsKeyDown(keys.back))
                            viewer -= facing * multiplier * 4;
                        if(IsKeyDown(keys.left))
                            viewer += DoRotation(flatfacing * multiplier, 0, 90, 0) * 4;
                        if(IsKeyDown(keys.right))
                            viewer += DoRotation(flatfacing * multiplier, 0, -90, 0) * 4;
                        if(IsKeyDown(keys.jump))
                            viewer.y += multiplier * 4;
                        if(IsKeyDown(keys.crouch))
                            viewer.y -= multiplier * 4;
                        if(!endkeydown && (IsKeyDown(SDL_SCANCODE_1) || IsKeyDown(SDL_SCANCODE_2) ||
                                           IsKeyDown(SDL_SCANCODE_3) || IsKeyDown(SDL_SCANCODE_4) ||
                                           IsKeyDown(SDL_SCANCODE_5) || IsKeyDown(SDL_SCANCODE_6) ||
                                           IsKeyDown(SDL_SCANCODE_7) || IsKeyDown(SDL_SCANCODE_8) ||
                                           IsKeyDown(SDL_SCANCODE_9) || IsKeyDown(SDL_SCANCODE_0) ||
                                           IsKeyDown(SDL_SCANCODE_MINUS))) {
                            int whichend;
                            if(IsKeyDown(SDL_SCANCODE_1))
                                whichend = 1;
                            if(IsKeyDown(SDL_SCANCODE_2))
                                whichend = 2;
                            if(IsKeyDown(SDL_SCANCODE_3))
                                whichend = 3;
                            if(IsKeyDown(SDL_SCANCODE_4))
                                whichend = 4;
                            if(IsKeyDown(SDL_SCANCODE_5))
                                whichend = 5;
                            if(IsKeyDown(SDL_SCANCODE_6))
                                whichend = 6;
                            if(IsKeyDown(SDL_SCANCODE_7))
                                whichend = 7;
                            if(IsKeyDown(SDL_SCANCODE_8))
                                whichend = 8;
                            if(IsKeyDown(SDL_SCANCODE_9))
                                whichend = 9;
                            if(IsKeyDown(SDL_SCANCODE_0))
                                whichend = 0;
                            if(IsKeyDown(SDL_SCANCODE_MINUS))
                                whichend = -1;
                            if(whichend != -1) {
                                participantfocus[whichdialogue][indialogue] = whichend;
                                participantlocation[whichdialogue][whichend] = player[whichend].coords;
                                participantrotation[whichdialogue][whichend] = player[whichend].rotation;
                            }
                            if(whichend == -1) {
                                participantfocus[whichdialogue][indialogue] = -1;
                            }
                            if(player[participantfocus[whichdialogue][indialogue]].dead) {
                                indialogue = -1;
                                directing = 0;
                                cameramode = 0;
                            }
                            dialoguecamera[whichdialogue][indialogue] = viewer;
                            dialoguecamerarotation[whichdialogue][indialogue] = rotation;
                            dialoguecamerarotation2[whichdialogue][indialogue] = rotation2;
                            indialogue++;
                            if(indialogue < numdialogueboxes[whichdialogue]) {
                                if(dialogueboxsound[whichdialogue][indialogue] != 0) {
                                    static float gLoc[3];
                                    static float vel[3];
                                    XYZ temppos;
                                    temppos = player[participantfocus[whichdialogue][indialogue]].coords;
                                    temppos = temppos - viewer;
                                    Normalise(&temppos);
                                    temppos += viewer;

                                    gLoc[0] = temppos.x;
                                    gLoc[1] = temppos.y;
                                    gLoc[2] = temppos.z;
                                    vel[0] = 0;
                                    vel[1] = 0;
                                    vel[2] = 0;
                                    int whichsoundplay;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 1)
                                        whichsoundplay = rabbitchitter;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 2)
                                        whichsoundplay = rabbitchitter2;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 3)
                                        whichsoundplay = rabbitpainsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 4)
                                        whichsoundplay = rabbitpain1sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 5)
                                        whichsoundplay = rabbitattacksound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 6)
                                        whichsoundplay = rabbitattack2sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 7)
                                        whichsoundplay = rabbitattack3sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 8)
                                        whichsoundplay = rabbitattack4sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 9)
                                        whichsoundplay = growlsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 10)
                                        whichsoundplay = growl2sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 11)
                                        whichsoundplay = snarlsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 12)
                                        whichsoundplay = snarl2sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 13)
                                        whichsoundplay = barksound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 14)
                                        whichsoundplay = bark2sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 15)
                                        whichsoundplay = bark3sound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == 16)
                                        whichsoundplay = barkgrowlsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == -1)
                                        whichsoundplay = fireendsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == -2)
                                        whichsoundplay = firestartsound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == -3)
                                        whichsoundplay = consolesuccesssound;
                                    if(dialogueboxsound[whichdialogue][indialogue] == -4)
                                        whichsoundplay = consolefailsound;
                                    PlaySoundEx(whichsoundplay, samp[whichsoundplay], NULL, true);
                                    OPENAL_3D_SetAttributes(channels[whichsoundplay], gLoc, vel);
                                    OPENAL_SetVolume(channels[whichsoundplay], 256);
                                    OPENAL_SetPaused(channels[whichsoundplay], false);
                                }
                            }

                            for(j = 0; j < numplayers; j++) {
                                participantfacing[whichdialogue][indialogue][j] =
                                    participantfacing[whichdialogue][indialogue - 1][j];
                            }

                            endkeydown = 1;
                        }
                        if((IsKeyDown(SDL_SCANCODE_KP_1) || IsKeyDown(SDL_SCANCODE_KP_2) ||
                            IsKeyDown(SDL_SCANCODE_KP_3) || IsKeyDown(SDL_SCANCODE_KP_4) ||
                            IsKeyDown(SDL_SCANCODE_KP_5) || IsKeyDown(SDL_SCANCODE_KP_6) ||
                            IsKeyDown(SDL_SCANCODE_KP_7) || IsKeyDown(SDL_SCANCODE_KP_8) ||
                            IsKeyDown(SDL_SCANCODE_KP_9) || IsKeyDown(SDL_SCANCODE_KP_0))) {
                            int whichend;
                            if(IsKeyDown(SDL_SCANCODE_KP_1))
                                whichend = 1;
                            if(IsKeyDown(SDL_SCANCODE_KP_2))
                                whichend = 2;
                            if(IsKeyDown(SDL_SCANCODE_KP_3))
                                whichend = 3;
                            if(IsKeyDown(SDL_SCANCODE_KP_4))
                                whichend = 4;
                            if(IsKeyDown(SDL_SCANCODE_KP_5))
                                whichend = 5;
                            if(IsKeyDown(SDL_SCANCODE_KP_6))
                                whichend = 6;
                            if(IsKeyDown(SDL_SCANCODE_KP_7))
                                whichend = 7;
                            if(IsKeyDown(SDL_SCANCODE_KP_8))
                                whichend = 8;
                            if(IsKeyDown(SDL_SCANCODE_KP_9))
                                whichend = 9;
                            if(IsKeyDown(SDL_SCANCODE_KP_0))
                                whichend = 0;
                            participantfacing[whichdialogue][indialogue][whichend] = facing;
                        }
                        if(!IsKeyDown(SDL_SCANCODE_1) && !IsKeyDown(SDL_SCANCODE_2) &&
                           !IsKeyDown(SDL_SCANCODE_3) && !IsKeyDown(SDL_SCANCODE_4) &&
                           !IsKeyDown(SDL_SCANCODE_5) && !IsKeyDown(SDL_SCANCODE_6) &&
                           !IsKeyDown(SDL_SCANCODE_7) && !IsKeyDown(SDL_SCANCODE_8) &&
                           !IsKeyDown(SDL_SCANCODE_9) && !IsKeyDown(SDL_SCANCODE_0) &&
                           !IsKeyDown(SDL_SCANCODE_MINUS)) {
                            endkeydown = 0;
                        }
                        if(indialogue >= numdialogueboxes[whichdialogue]) {
                            indialogue = -1;
                            directing = 0;
                            cameramode = 0;
                        }
                    }
                    if(!directing) {
                        OPENAL_SetPaused(channels[whooshsound], true);
                        viewer = dialoguecamera[whichdialogue][indialogue];
                        if(viewer.y < terrain.getHeight(viewer.x, viewer.z) + .1) {
                            viewer.y = terrain.getHeight(viewer.x, viewer.z) + .1;
                        }
                        rotation = dialoguecamerarotation[whichdialogue][indialogue];
                        rotation2 = dialoguecamerarotation2[whichdialogue][indialogue];
                        if(dialoguetime > 0.5)
                            if((!endkeydown && (IsKeyDown(SDL_SCANCODE_1) || IsKeyDown(SDL_SCANCODE_2) ||
                                                IsKeyDown(SDL_SCANCODE_3) || IsKeyDown(SDL_SCANCODE_4) ||
                                                IsKeyDown(SDL_SCANCODE_5) || IsKeyDown(SDL_SCANCODE_6) ||
                                                IsKeyDown(SDL_SCANCODE_7) || IsKeyDown(SDL_SCANCODE_8) ||
                                                IsKeyDown(SDL_SCANCODE_9) || IsKeyDown(SDL_SCANCODE_0) ||
                                                IsKeyDown(SDL_SCANCODE_MINUS))) ||
                               (IsButtonPressed(keys.attack) && !oldbuttondialogue)) {
                                indialogue++;
                                endkeydown = 1;
                                if(indialogue < numdialogueboxes[whichdialogue]) {
                                    if(dialogueboxsound[whichdialogue][indialogue] != 0) {
                                        static float gLoc[3];
                                        static float vel[3];
                                        XYZ temppos;
                                        temppos = player[participantfocus[whichdialogue][indialogue]].coords;
                                        temppos = temppos - viewer;
                                        Normalise(&temppos);
                                        temppos += viewer;

                                        gLoc[0] = temppos.x;
                                        gLoc[1] = temppos.y;
                                        gLoc[2] = temppos.z;
                                        vel[0] = 0;
                                        vel[1] = 0;
                                        vel[2] = 0;
                                        int whichsoundplay;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 1)
                                            whichsoundplay = rabbitchitter;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 2)
                                            whichsoundplay = rabbitchitter2;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 3)
                                            whichsoundplay = rabbitpainsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 4)
                                            whichsoundplay = rabbitpain1sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 5)
                                            whichsoundplay = rabbitattacksound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 6)
                                            whichsoundplay = rabbitattack2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 7)
                                            whichsoundplay = rabbitattack3sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 8)
                                            whichsoundplay = rabbitattack4sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 9)
                                            whichsoundplay = growlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 10)
                                            whichsoundplay = growl2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 11)
                                            whichsoundplay = snarlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 12)
                                            whichsoundplay = snarl2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 13)
                                            whichsoundplay = barksound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 14)
                                            whichsoundplay = bark2sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 15)
                                            whichsoundplay = bark3sound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == 16)
                                            whichsoundplay = barkgrowlsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -1)
                                            whichsoundplay = fireendsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -2)
                                            whichsoundplay = firestartsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -3)
                                            whichsoundplay = consolesuccesssound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -4)
                                            whichsoundplay = consolefailsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] == -6)
                                            whichsoundplay = alarmsound;
                                        if(dialogueboxsound[whichdialogue][indialogue] != -5) {
                                            PlaySoundEx(whichsoundplay, samp[whichsoundplay], NULL, true);
                                            OPENAL_3D_SetAttributes(channels[whichsoundplay], gLoc, vel);
                                            OPENAL_SetVolume(channels[whichsoundplay], 256);
                                            OPENAL_SetPaused(channels[whichsoundplay], false);
                                        }
                                        if(dialogueboxsound[whichdialogue][indialogue] == -5) {
                                            hotspot[numhotspots] = player[0].coords;
                                            hotspotsize[numhotspots] = 10;
                                            hotspottype[numhotspots] = -1;

                                            numhotspots++;
                                        }
                                        if(dialogueboxsound[whichdialogue][indialogue] == -6) {
                                            hostile = 1;
                                        }

                                        if(player[participantfocus[whichdialogue][indialogue]].dead) {
                                            indialogue = -1;
                                            directing = 0;
                                            cameramode = 0;
                                        }
                                    }
                                }
                            }
                        if(!IsKeyDown(SDL_SCANCODE_1) && !IsKeyDown(SDL_SCANCODE_2) &&
                           !IsKeyDown(SDL_SCANCODE_3) && !IsKeyDown(SDL_SCANCODE_4) &&
                           !IsKeyDown(SDL_SCANCODE_5) && !IsKeyDown(SDL_SCANCODE_6) &&
                           !IsKeyDown(SDL_SCANCODE_7) && !IsKeyDown(SDL_SCANCODE_8) &&
                           !IsKeyDown(SDL_SCANCODE_9) && !IsKeyDown(SDL_SCANCODE_0) &&
                           !IsKeyDown(SDL_SCANCODE_MINUS)) {
                            endkeydown = 0;
                        }
                        if(indialogue >= numdialogueboxes[whichdialogue]) {
                            indialogue = -1;
                            directing = 0;
                            cameramode = 0;
                            if(dialoguetype[whichdialogue] > 19 && dialoguetype[whichdialogue] < 30) {
                                hostile = 1;
                            }
                            if(dialoguetype[whichdialogue] > 29 && dialoguetype[whichdialogue] < 40) {
                                windialogue = 1;
                            }
                            if(dialoguetype[whichdialogue] > 49 && dialoguetype[whichdialogue] < 60) {
                                hostile = 1;
                                for(i = 1; i < numplayers; i++) {
                                    player[i].aitype = attacktypecutoff;
                                }
                            }
                        }
                    }
                }

                if(!IsButtonPressed(keys.attack))
                    oldbuttondialogue = 0;
                else
                    oldbuttondialogue = 1;

                if(!player[0].jumpkeydown) {
                    player[0].jumptogglekeydown = 0;
                }
                if(player[0].jumpkeydown && player[0].targetanimation != jumpupanim &&
                   player[0].targetanimation != jumpdownanim && !player[0].isFlip())
                    player[0].jumptogglekeydown = 1;

                dialoguetime += multiplier;
                skybox.cloudmove += multiplier;
                hawkrotation += multiplier * 25;
                realhawkcoords = 0;
                realhawkcoords.x = 25;
                realhawkcoords = DoRotation(realhawkcoords, 0, hawkrotation, 0) + hawkcoords;
                hawkcalldelay -= multiplier / 2;

                if(hawkcalldelay <= 0) {
                    static float gLoc[3];
                    static float vel[3];
                    gLoc[0] = realhawkcoords.x;
                    gLoc[1] = realhawkcoords.y;
                    gLoc[2] = realhawkcoords.z;
                    vel[0] = 0;
                    vel[1] = 0;
                    vel[2] = 0;
                    PlaySoundEx(hawksound, samp[hawksound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[hawksound], gLoc, vel);
                    OPENAL_SetVolume(channels[hawksound], 128);
                    OPENAL_SetPaused(channels[hawksound], false);

                    hawkcalldelay = 16 + abs(rand() % 8);
                }

                if(IsKeyDown(SDL_SCANCODE_H) && config.game_debug) {
                    player[0].damagetolerance = 200000;
                    player[0].damage = 0;
                    player[0].burnt = 0;
                    player[0].permanentdamage = 0;
                    player[0].superpermanentdamage = 0;
                    /*
                    int whichchar;
                    whichchar = abs(rand()%7);
                    registrationname[whichchar]+=1;
                    */
                }

                if(IsKeyDown(SDL_SCANCODE_J) && !envtogglekeydown && config.game_debug) {
                    environment++;
                    if(environment > 2)
                        environment = 0;
                    Setenvironment(environment);

                    envtogglekeydown = 1;
                }

                if(!IsKeyDown(SDL_SCANCODE_J)) {
                    envtogglekeydown = 0;
                }

                if(IsKeyDown(SDL_SCANCODE_C) && !cameratogglekeydown && config.game_debug) {
                    cameramode = 1 - cameramode;
                    cameratogglekeydown = 1;
                }

                if(!IsKeyDown(SDL_SCANCODE_C)) {
                    cameratogglekeydown = 0;
                }

                if(IsKeyDown(SDL_SCANCODE_X) && !IsKeyDown(SDL_SCANCODE_LSHIFT) && !detailtogglekeydown &&
                   config.game_debug) {
                    if(player[0].numweapons > 0) {
                        if(weapons.type[player[0].weaponids[0]] == sword)
                            weapons.type[player[0].weaponids[0]] = staff;
                        else if(weapons.type[player[0].weaponids[0]] == staff)
                            weapons.type[player[0].weaponids[0]] = knife;
                        else
                            weapons.type[player[0].weaponids[0]] = sword;
                        if(weapons.type[player[0].weaponids[0]] == sword) {
                            weapons.mass[player[0].weaponids[0]] = 1.5;
                            weapons.tipmass[player[0].weaponids[0]] = 1;
                            weapons.length[player[0].weaponids[0]] = .8;
                        }
                        if(weapons.type[player[0].weaponids[0]] == staff) {
                            weapons.mass[player[0].weaponids[0]] = 2;
                            weapons.tipmass[player[0].weaponids[0]] = 1;
                            weapons.length[player[0].weaponids[0]] = 1.5;
                        }

                        if(weapons.type[player[0].weaponids[0]] == knife) {
                            weapons.mass[player[0].weaponids[0]] = 1;
                            weapons.tipmass[player[0].weaponids[0]] = 1.2;
                            weapons.length[player[0].weaponids[0]] = .25;
                        }
                    }

                    /*for(i=0;i<objects.numobjects;i++){
                    if(objects.type[i]==treeleavestype){
                    for(j=0;j<objects.numobjects;j++){
                    if(objects.type[j]==treetrunktype)
                    if(findDistancefast(&objects.position[i],&objects.position[j])<.5)
                    objects.scale[i]=objects.scale[j];
                    }
                    }
                    }*/
                    detailtogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_X, KMOD_SHIFT) && !detailtogglekeydown && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }
                    if(closest != -1) {
                        if(player[closest].numweapons) {
                            if(weapons.type[player[closest].weaponids[0]] == sword)
                                weapons.type[player[closest].weaponids[0]] = staff;
                            else if(weapons.type[player[closest].weaponids[0]] == staff)
                                weapons.type[player[closest].weaponids[0]] = knife;
                            else
                                weapons.type[player[closest].weaponids[0]] = sword;
                            if(weapons.type[player[closest].weaponids[0]] == sword) {
                                weapons.mass[player[closest].weaponids[0]] = 1.5;
                                weapons.tipmass[player[closest].weaponids[0]] = 1;
                                weapons.length[player[closest].weaponids[0]] = .8;
                            }
                            if(weapons.type[player[0].weaponids[0]] == staff) {
                                weapons.mass[player[0].weaponids[0]] = 2;
                                weapons.tipmass[player[0].weaponids[0]] = 1;
                                weapons.length[player[0].weaponids[0]] = 1.5;
                            }
                            if(weapons.type[player[closest].weaponids[0]] == knife) {
                                weapons.mass[player[closest].weaponids[0]] = 1;
                                weapons.tipmass[player[closest].weaponids[0]] = 1.2;
                                weapons.length[player[closest].weaponids[0]] = .25;
                            }
                        }
                        if(!player[closest].numweapons) {
                            player[closest].weaponids[0] = weapons.numweapons;
                            weapons.owner[weapons.numweapons] = closest;
                            weapons.type[weapons.numweapons] = knife;
                            weapons.damage[weapons.numweapons] = 0;
                            weapons.numweapons++;
                            player[closest].numweapons = 1;
                            if(weapons.type[player[closest].weaponids[0]] == sword) {
                                weapons.mass[player[closest].weaponids[0]] = 1.5;
                                weapons.tipmass[player[closest].weaponids[0]] = 1;
                                weapons.length[player[closest].weaponids[0]] = .8;
                            }
                            if(weapons.type[player[closest].weaponids[0]] == knife) {
                                weapons.mass[player[closest].weaponids[0]] = 1;
                                weapons.tipmass[player[closest].weaponids[0]] = 1.2;
                                weapons.length[player[closest].weaponids[0]] = .25;
                            }
                        }
                    }
                    detailtogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_U) && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }

                    player[closest].rotation += multiplier * 50;
                    player[closest].targetrotation = player[closest].rotation;
                }

                if(IsKeyDown(SDL_SCANCODE_O) && !IsKeyDown(SDL_SCANCODE_LSHIFT) && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }
                    if(IsKeyDown(SDL_SCANCODE_LCTRL))
                        closest = 0;

                    if(closest != -1) {
                        player[closest].whichskin++;
                        if(player[closest].whichskin > 9)
                            player[closest].whichskin = 0;
                        if(player[closest].whichskin > 2 && player[closest].creature == wolftype)
                            player[closest].whichskin = 0;

                        LoadTextureSave(creatureskin[player[closest].creature][player[closest].whichskin],
                                        &player[closest].skeleton.drawmodel.textureptr,
                                        1,
                                        &player[closest].skeleton.skinText[0],
                                        &player[closest].skeleton.skinsize);
                    }

                    if(player[closest].numclothes) {
                        for(i = 0; i < player[closest].numclothes; i++) {
                            tintr = player[closest].clothestintr[i];
                            tintg = player[closest].clothestintg[i];
                            tintb = player[closest].clothestintb[i];
                            AddClothes((char*)player[closest].clothes[i],
                                       0,
                                       1,
                                       &player[closest].skeleton.skinText[0],
                                       &player[closest].skeleton.skinsize);
                        }
                        player[closest].DoMipmaps(
                            5, 0, 0, player[closest].skeleton.skinsize, player[closest].skeleton.skinsize);
                    }

                    detailtogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_O, KMOD_SHIFT) && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }
                    if(closest != -1) {
                        if(player[closest].creature == wolftype) {
                            headprop = player[closest].proportionhead.x / 1.1;
                            bodyprop = player[closest].proportionbody.x / 1.1;
                            armprop = player[closest].proportionarms.x / 1.1;
                            legprop = player[closest].proportionlegs.x / 1.1;
                        }

                        if(player[closest].creature == rabbittype) {
                            headprop = player[closest].proportionhead.x / 1.2;
                            bodyprop = player[closest].proportionbody.x / 1.05;
                            armprop = player[closest].proportionarms.x / 1.00;
                            legprop = player[closest].proportionlegs.x / 1.1;
                        }

                        if(player[closest].creature == rabbittype) {
                            player[closest].skeleton.id = closest;
                            player[closest].skeleton.Load((char*)"Skeleton/Basic Figure Wolf",
                                                          (char*)"Skeleton/Basic Figure Wolf Low",
                                                          (char*)"Skeleton/Rabbitbelt",
                                                          (char*)"Models/wolf.solid",
                                                          (char*)"Models/wolf2.solid",
                                                          (char*)"Models/wolf3.solid",
                                                          (char*)"Models/wolf4.solid",
                                                          (char*)"Models/wolf5.solid",
                                                          (char*)"Models/wolf6.solid",
                                                          (char*)"Models/wolf7.solid",
                                                          (char*)"Models/wolflow.solid",
                                                          (char*)"Models/belt.solid",
                                                          0);
                            LoadTextureSave("wolf.jpg",
                                            &player[closest].skeleton.drawmodel.textureptr,
                                            1,
                                            &player[closest].skeleton.skinText[closest],
                                            &player[closest].skeleton.skinsize);
                            player[closest].whichskin = 0;
                            player[closest].creature = wolftype;

                            player[closest].proportionhead = 1.1;
                            player[closest].proportionbody = 1.1;
                            player[closest].proportionarms = 1.1;
                            player[closest].proportionlegs = 1.1;
                            player[closest].proportionlegs.y = 1.1;
                            player[closest].scale = .23 * 5 * player[0].scale;

                            player[closest].damagetolerance = 300;
                        } else {
                            player[closest].skeleton.id = closest;
                            player[closest].skeleton.Load((char*)"Skeleton/Basic Figure",
                                                          (char*)"Skeleton/Basic Figurelow",
                                                          (char*)"Skeleton/Rabbitbelt",
                                                          (char*)"Models/Body.solid",
                                                          (char*)"Models/Body2.solid",
                                                          (char*)"Models/Body3.solid",
                                                          (char*)"Models/Body4.solid",
                                                          (char*)"Models/Body5.solid",
                                                          (char*)"Models/Body6.solid",
                                                          (char*)"Models/Body7.solid",
                                                          (char*)"Models/Bodylow.solid",
                                                          (char*)"Models/Belt.solid",
                                                          1);
                            LoadTextureSave("fur3.jpg",
                                            &player[closest].skeleton.drawmodel.textureptr,
                                            1,
                                            &player[closest].skeleton.skinText[0],
                                            &player[closest].skeleton.skinsize);
                            player[closest].whichskin = 0;
                            player[closest].creature = rabbittype;

                            player[closest].proportionhead = 1.2;
                            player[closest].proportionbody = 1.05;
                            player[closest].proportionarms = 1.00;
                            player[closest].proportionlegs = 1.1;
                            player[closest].proportionlegs.y = 1.05;
                            player[closest].scale = .2 * 5 * player[0].scale;

                            player[closest].damagetolerance = 200;
                        }

                        if(player[closest].creature == wolftype) {
                            player[closest].proportionhead = 1.1 * headprop;
                            player[closest].proportionbody = 1.1 * bodyprop;
                            player[closest].proportionarms = 1.1 * armprop;
                            player[closest].proportionlegs = 1.1 * legprop;
                        }

                        if(player[closest].creature == rabbittype) {
                            player[closest].proportionhead = 1.2 * headprop;
                            player[closest].proportionbody = 1.05 * bodyprop;
                            player[closest].proportionarms = 1.00 * armprop;
                            player[closest].proportionlegs = 1.1 * legprop;
                            player[closest].proportionlegs.y = 1.05 * legprop;
                        }
                    }
                    detailtogglekeydown = 1;
                }

                if(!IsKeyDown(SDL_SCANCODE_X)) {
                    detailtogglekeydown = 0;
                }

                if(IsKeyDown(SDL_SCANCODE_B) && !slomotogglekeydown && !IsKeyDown(SDL_SCANCODE_LSHIFT) &&
                   config.game_debug) {
                    slomo = 1 - slomo;
                    slomodelay = 1000;
                    slomotogglekeydown = 1;
                }

                if(((IsKeyDown(SDL_SCANCODE_I) && !IsKeyDown(SDL_SCANCODE_LSHIFT))) &&
                   !explodetogglekeydown && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    XYZ flatfacing2, flatvelocity2;
                    XYZ blah;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(distance < 144 && !player[i].headless)
                                if(closestdist == -1 || distance < closestdist) {
                                    closestdist = distance;
                                    closest = i;
                                    blah = player[i].coords;
                                }
                        }

                    if(closest != -1) {
                        XYZ headspurtdirection;
                        int i = player[closest].skeleton.jointlabels[head];
                        for(k = 0; k < player[closest].skeleton.num_joints; k++) {
                            if(!player[closest].skeleton.free)
                                flatvelocity2 = player[closest].velocity;
                            if(player[closest].skeleton.free)
                                flatvelocity2 = player[closest].skeleton.joints[i].velocity;
                            if(!player[closest].skeleton.free)
                                flatfacing2 = DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,
                                                                               0,
                                                                               0,
                                                                               player[closest].tilt),
                                                                    player[closest].tilt2,
                                                                    0,
                                                                    0),
                                                         0,
                                                         player[closest].rotation,
                                                         0) *
                                                  player[closest].scale +
                                              player[closest].coords;
                            if(player[closest].skeleton.free)
                                flatfacing2 = player[closest].skeleton.joints[i].position * player[closest].scale +
                                              player[closest].coords;
                            flatvelocity2.x += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.y += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.z += (float)(abs(rand() % 100) - 50) / 10;
                            headspurtdirection =
                                player[closest].skeleton.joints[player[closest].skeleton.jointlabels[head]].position -
                                player[closest].skeleton.joints[player[closest].skeleton.jointlabels[neck]].position;
                            Normalise(&headspurtdirection);
                            sprites.MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, .6, 1);
                            flatvelocity2 += headspurtdirection * 8;
                            sprites.MakeSprite(bloodsprite, flatfacing2, flatvelocity2 / 2, 1, 1, 1, .16, 1);
                        }
                        sprites.MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);

                        float gLoc[3];
                        float vel[3];
                        gLoc[0] = blah.x;
                        gLoc[1] = blah.y;
                        gLoc[2] = blah.z;
                        vel[0] = 0;
                        vel[1] = 0;
                        vel[2] = 0;
                        PlaySoundEx(splattersound, samp[splattersound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[splattersound], gLoc, vel);
                        OPENAL_SetVolume(channels[splattersound], 256);
                        OPENAL_SetPaused(channels[splattersound], false);

                        PlaySoundEx(breaksound2, samp[breaksound2], NULL, true);
                        OPENAL_3D_SetAttributes(channels[breaksound2], gLoc, vel);
                        OPENAL_SetVolume(channels[breaksound2], 100);
                        OPENAL_SetPaused(channels[breaksound2], false);

                        if(player[closest].skeleton.free == 2)
                            player[closest].skeleton.free = 0;
                        player[closest].RagDoll(0);
                        player[closest].dead = 2;
                        player[closest].headless = 1;
                        player[closest].DoBloodBig(3, 165);

                        camerashake += .3;
                    }

                    explodetogglekeydown = 1;
                }

                if(((IsKeyDown(SDL_SCANCODE_I) && IsKeyDown(SDL_SCANCODE_LSHIFT)) ) && !explodetogglekeydown && config.game_debug) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    XYZ flatfacing2, flatvelocity2;
                    XYZ blah;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(distance < 144)
                                if(closestdist == -1 || distance < closestdist) {
                                    closestdist = distance;
                                    closest = i;
                                    blah = player[i].coords;
                                }
                        }

                    if(closest != -1) {
                        float gLoc[3];
                        float vel[3];
                        gLoc[0] = blah.x;
                        gLoc[1] = blah.y;
                        gLoc[2] = blah.z;
                        vel[0] = 0;
                        vel[1] = 0;
                        vel[2] = 0;

                        PlaySoundEx(splattersound, samp[splattersound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[splattersound], gLoc, vel);
                        OPENAL_SetVolume(channels[splattersound], 256);
                        OPENAL_SetPaused(channels[splattersound], false);

                        PlaySoundEx(breaksound2, samp[breaksound2], NULL, true);
                        OPENAL_3D_SetAttributes(channels[breaksound2], gLoc, vel);
                        OPENAL_SetVolume(channels[breaksound2], 600);
                        OPENAL_SetPaused(channels[breaksound2], false);

                        for(i = 0; i < player[closest].skeleton.num_joints; i++) {
                            if(!player[closest].skeleton.free)
                                flatvelocity2 = player[closest].velocity;
                            if(player[closest].skeleton.free)
                                flatvelocity2 = player[closest].skeleton.joints[i].velocity;
                            if(!player[closest].skeleton.free)
                                flatfacing2 = DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,
                                                                               0,
                                                                               0,
                                                                               player[closest].tilt),
                                                                    player[closest].tilt2,
                                                                    0,
                                                                    0),
                                                         0,
                                                         player[closest].rotation,
                                                         0) *
                                                  player[closest].scale +
                                              player[closest].coords;
                            if(player[closest].skeleton.free)
                                flatfacing2 = player[closest].skeleton.joints[i].position * player[closest].scale +
                                              player[closest].coords;
                            flatvelocity2.x += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.y += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.z += (float)(abs(rand() % 100) - 50) / 10;
                            sprites.MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                            sprites.MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .3, 1);
                            sprites.MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);
                        }

                        for(i = 0; i < player[closest].skeleton.num_joints; i++) {
                            if(!player[closest].skeleton.free)
                                flatvelocity2 = player[closest].velocity;
                            if(player[closest].skeleton.free)
                                flatvelocity2 = player[closest].skeleton.joints[i].velocity;
                            if(!player[closest].skeleton.free)
                                flatfacing2 = DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,
                                                                               0,
                                                                               0,
                                                                               player[closest].tilt),
                                                                    player[closest].tilt2,
                                                                    0,
                                                                    0),
                                                         0,
                                                         player[closest].rotation,
                                                         0) *
                                                  player[closest].scale +
                                              player[closest].coords;
                            if(player[closest].skeleton.free)
                                flatfacing2 = player[closest].skeleton.joints[i].position * player[closest].scale +
                                              player[closest].coords;
                            flatvelocity2.x += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.y += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.z += (float)(abs(rand() % 100) - 50) / 10;
                            sprites.MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                            sprites.MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .4, 1);
                        }

                        for(i = 0; i < player[closest].skeleton.num_joints; i++) {
                            if(!player[closest].skeleton.free)
                                flatvelocity2 = player[closest].velocity;
                            if(player[closest].skeleton.free)
                                flatvelocity2 = player[closest].skeleton.joints[i].velocity;
                            if(!player[closest].skeleton.free)
                                flatfacing2 = DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,
                                                                               0,
                                                                               0,
                                                                               player[closest].tilt),
                                                                    player[closest].tilt2,
                                                                    0,
                                                                    0),
                                                         0,
                                                         player[closest].rotation,
                                                         0) *
                                                  player[closest].scale +
                                              player[closest].coords;
                            if(player[closest].skeleton.free)
                                flatfacing2 = player[closest].skeleton.joints[i].position * player[closest].scale +
                                              player[closest].coords;
                            flatvelocity2.x += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.y += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.z += (float)(abs(rand() % 100) - 50) / 10;
                            sprites.MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                            sprites.MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);
                        }

                        for(i = 0; i < player[closest].skeleton.num_joints; i++) {
                            if(!player[closest].skeleton.free)
                                flatvelocity2 = player[closest].velocity;
                            if(player[closest].skeleton.free)
                                flatvelocity2 = player[closest].skeleton.joints[i].velocity;
                            if(!player[closest].skeleton.free)
                                flatfacing2 = DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,
                                                                               0,
                                                                               0,
                                                                               player[closest].tilt),
                                                                    player[closest].tilt2,
                                                                    0,
                                                                    0),
                                                         0,
                                                         player[closest].rotation,
                                                         0) *
                                                  player[closest].scale +
                                              player[closest].coords;
                            if(player[closest].skeleton.free)
                                flatfacing2 = player[closest].skeleton.joints[i].position * player[closest].scale +
                                              player[closest].coords;
                            flatvelocity2.x += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.y += (float)(abs(rand() % 100) - 50) / 10;
                            flatvelocity2.z += (float)(abs(rand() % 100) - 50) / 10;
                            sprites.MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                            sprites.MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);
                        }

                        XYZ temppos;
                        for(j = 0; j < numplayers; j++) {
                            if(j != closest) {
                                if(findDistancefast(&player[j].coords, &player[closest].coords) < 25) {
                                    player[j].DoDamage((25 - findDistancefast(&player[j].coords, &player[closest].coords)) *
                                                       60);
                                    if(player[j].skeleton.free == 2)
                                        player[j].skeleton.free = 1;
                                    player[j].skeleton.longdead = 0;
                                    player[j].RagDoll(0);
                                    for(i = 0; i < player[j].skeleton.num_joints; i++) {
                                        temppos = player[j].skeleton.joints[i].position + player[j].coords;
                                        if(findDistancefast(&temppos, &player[closest].coords) < 25) {
                                            flatvelocity2 = temppos - player[closest].coords;
                                            Normalise(&flatvelocity2);
                                            player[j].skeleton.joints[i].velocity +=
                                                flatvelocity2 *
                                                ((20 - findDistancefast(&temppos, &player[closest].coords)) * 20);
                                        }
                                    }
                                }
                            }
                        }

                        player[closest].DoDamage(10000);
                        player[closest].RagDoll(0);
                        player[closest].dead = 2;
                        player[closest].coords = 20;
                        player[closest].skeleton.free = 2;

                        camerashake += .6;
                    }

                    explodetogglekeydown = 1;
                }

                if(!IsKeyDown(SDL_SCANCODE_I)) {
                    explodetogglekeydown = 0;
                }

                /*
                if(IsKeyDown(theKeyMap, MAC_S_KEY)&&IsKeyDown(theKeyMap, MAC_COMMAND_KEY)&&!slomotogglekeydown){
                FILE			*tfile;
                //tfile=fopen( "maps/mapsave", "wb" );
                if(whichlevel==0)tfile=fopen( "maps/map1", "wb" );
                else if(whichlevel==1)tfile=fopen( "maps/map2", "wb" );
                else if(whichlevel==2)tfile=fopen( "maps/map3", "wb" );
                else if(whichlevel==3)tfile=fopen( "maps/map4", "wb" );
                else if(whichlevel==4)tfile=fopen( "maps/map5", "wb" );
                else tfile=fopen( "maps/mapsave", "wb" );

                fwrite( &player[0].coords, 1, sizeof(XYZ), tfile );
                fwrite( &player[0].rotation, 1, sizeof(float), tfile );
                fwrite( &player[0].targetrotation, 1, sizeof(float), tfile );
                fwrite( &player[0].numweapons, 1, sizeof(int), tfile );
                for(j=0;j<player[0].numweapons;j++){
                fwrite( &weapons.type[player[0].weaponids[j]], 1, sizeof(int), tfile );
                }
                fwrite( &environment, 1, sizeof(int), tfile );

                fwrite( &objects.numobjects, 1, sizeof(int), tfile );
                fwrite( &objects.type, 1, sizeof(int)*objects.numobjects, tfile );
                fwrite( &objects.rotation, 1, sizeof(float)*objects.numobjects, tfile );
                fwrite( &objects.position, 1, sizeof(XYZ)*objects.numobjects, tfile );
                fwrite( &objects.scale, 1, sizeof(float)*objects.numobjects, tfile );

                fwrite( &numplayers, 1, sizeof(int), tfile );
                if(numplayers>1&&numplayers<maxplayers)
                for(i=1;i<numplayers;i++){
                fwrite( &player[i].coords, 1, sizeof(XYZ), tfile );
                fwrite( &player[i].numweapons, 1, sizeof(int), tfile );
                for(j=0;j<player[i].numweapons;j++){
                fwrite( &weapons.type[player[i].weaponids[j]], 1, sizeof(int), tfile );
                }
                if(player[i].numwaypoints<30){
                fwrite( &player[i].numwaypoints, 1, sizeof(int), tfile );
                fwrite( &player[i].waypoints, 1, sizeof(XYZ)*player[i].numwaypoints, tfile );
                fwrite( &player[i].waypoint, 1, sizeof(int), tfile );
                //fwrite( &player[i].jumppath, 1, sizeof(bool), tfile );
                }
                else{
                player[i].numwaypoints=0;
                player[i].waypoint=0;
                fwrite( &player[i].numwaypoints, 1, sizeof(int), tfile );
                fwrite( &player[i].waypoint, 1, sizeof(int), tfile );
                fwrite( &player[i].waypoint, 1, sizeof(int), tfile );
                }
                }

                fclose(tfile);

                slomotogglekeydown=1;
                }*/

                if(!IsKeyDown(SDL_SCANCODE_B) && !IsKeyDown(SDL_SCANCODE_F) &&
                   !IsKeyDown(SDL_SCANCODE_K) && !IsKeyDown(SDL_SCANCODE_S)) {
                    slomotogglekeydown = 0;
                }

                if(IsKeyDown(SDL_SCANCODE_F) && !slomotogglekeydown && config.game_debug) {
                    player[0].onfire = 1 - player[0].onfire;
                    if(player[0].onfire) {
                        player[0].CatchFire();
                    }
                    if(!player[0].onfire) {
                        float gLoc[3];
                        float vel[3];
                        gLoc[0] = player[0].coords.x;
                        gLoc[1] = player[0].coords.y;
                        gLoc[2] = player[0].coords.z;
                        vel[0] = 0;
                        vel[1] = 0;
                        vel[2] = 0;
                        PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                        OPENAL_SetVolume(channels[fireendsound], 256);
                        OPENAL_SetPaused(channels[fireendsound], false);
                        OPENAL_SetPaused(channels[stream_firesound], true);
                    }
                    slomotogglekeydown = 1;
                }
                /*
                if(IsKeyDown(theKeyMap, MAC_L_KEY)){
                if(player[0].bleeding<=0)
                player[0].DoBlood(1,255);
                }*/

                if(IsKeyDown(SDL_SCANCODE_DELETE, KMOD_SHIFT) && editorenabled && !drawmodetogglekeydown) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(numplayers > 1)
                        for(i = 1; i < numplayers; i++) {
                            distance = findDistancefast(&player[i].coords, &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }
                    if(closestdist > 0 && closest >= 0) {
                        // player[closest]=player[numplayers-1];
                        // player[closest].skeleton=player[numplayers-1].skeleton;
                        numplayers--;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_DELETE, KMOD_CTRL) && editorenabled && !drawmodetogglekeydown) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    if(max_objects > 1)
                        for(i = 1; i < max_objects; i++) {
                            distance = findDistancefast(&objects.position[i], &player[0].coords);
                            if(closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = i;
                            }
                        }
                    if(closestdist > 0 && closest >= 0) {
                        objects.position[closest].y -= 500;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_M) && !drawmodetogglekeydown && !IsKeyDown(SDL_SCANCODE_LSHIFT) &&
                   editorenabled && config.game_debug) {
                    // drawmode++;
                    // if(drawmode>2)drawmode=0;
                    if(objects.numobjects < max_objects - 1) {
                        XYZ boxcoords;
                        boxcoords.x = player[0].coords.x;
                        boxcoords.z = player[0].coords.z;
                        boxcoords.y = player[0].coords.y - 3;
                        if(editortype == bushtype)
                            boxcoords.y = player[0].coords.y - .5;
                        if(editortype == firetype)
                            boxcoords.y = player[0].coords.y - .5;
                        // objects.MakeObject(abs(rand()%3),boxcoords,rand()%360);
                        float temprotat, temprotat2;
                        temprotat = editorrotation;
                        temprotat2 = editorrotation2;
                        if(temprotat < 0 || editortype == bushtype)
                            temprotat = rand() % 360;
                        if(temprotat2 < 0)
                            temprotat2 = rand() % 360;

                        objects.MakeObject(
                            editortype, boxcoords, (int)temprotat - ((int)temprotat) % 30, (int)temprotat2, editorsize);
                        if(editortype == treetrunktype)
                            objects.MakeObject(treeleavestype,
                                               boxcoords,
                                               rand() % 360 * (temprotat2 < 2) + (int)editorrotation -
                                                   ((int)editorrotation) % 30,
                                               editorrotation2,
                                               editorsize);
                    }

                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_P, ~KMOD_SHIFT | ~KMOD_CTRL) && !drawmodetogglekeydown && editorenabled) {
                    if(numplayers < maxplayers - 1) {
                        player[numplayers].scale = .2 * 5 * player[0].scale;
                        player[numplayers].creature = rabbittype;
                        player[numplayers].howactive = editoractive;
                        player[numplayers].skeleton.id = numplayers;
                        player[numplayers].skeleton.Load((char*)"Skeleton/Basic Figure",
                                                         (char*)"Skeleton/Basic Figurelow",
                                                         (char*)"Skeleton/Rabbitbelt",
                                                         (char*)"Models/body.solid",
                                                         (char*)"Models/body2.solid",
                                                         (char*)"Models/body3.solid",
                                                         (char*)"Models/body4.solid",
                                                         (char*)"Models/body5.solid",
                                                         (char*)"Models/body6.solid",
                                                         (char*)"Models/body7.solid",
                                                         (char*)"Models/bodylow.solid",
                                                         (char*)"Models/belt.solid",
                                                         1);

                        // texsize=512*512*3/texdetail/texdetail;
                        // if(!player[numplayers].loaded)player[numplayers].skeleton.skinText = new GLubyte[texsize];
                        // player[numplayers].skeleton.skinText.resize(texsize);

                        k = abs(rand() % 2) + 1;
                        if(k == 0) {
                            LoadTextureSave("fur3.jpg",
                                            &player[numplayers].skeleton.drawmodel.textureptr,
                                            1,
                                            &player[numplayers].skeleton.skinText[0],
                                            &player[numplayers].skeleton.skinsize);
                            player[numplayers].whichskin = 0;
                        } else if(k == 1) {
                            LoadTextureSave("fur.jpg",
                                            &player[numplayers].skeleton.drawmodel.textureptr,
                                            1,
                                            &player[numplayers].skeleton.skinText[0],
                                            &player[numplayers].skeleton.skinsize);
                            player[numplayers].whichskin = 1;
                        } else {
                            LoadTextureSave("fur2.jpg",
                                            &player[numplayers].skeleton.drawmodel.textureptr,
                                            1,
                                            &player[numplayers].skeleton.skinText[0],
                                            &player[numplayers].skeleton.skinsize);
                            player[numplayers].whichskin = 2;
                        }

                        LoadTexture("belt.png", &player[numplayers].skeleton.drawmodelclothes.textureptr, 1, 1);
                        player[numplayers].power = 1;
                        player[numplayers].speedmult = 1;
                        player[numplayers].currentanimation = bounceidleanim;
                        player[numplayers].targetanimation = bounceidleanim;
                        player[numplayers].currentframe = 0;
                        player[numplayers].targetframe = 1;
                        player[numplayers].target = 0;
                        player[numplayers].bled = 0;
                        player[numplayers].speed = 1 + (float)(rand() % 100) / 1000;

                        player[numplayers].targetrotation = player[0].targetrotation;
                        player[numplayers].rotation = player[0].rotation;

                        player[numplayers].velocity = 0;
                        player[numplayers].coords = player[0].coords;
                        player[numplayers].oldcoords = player[numplayers].coords;
                        player[numplayers].realoldcoords = player[numplayers].coords;

                        player[numplayers].id = numplayers;
                        player[numplayers].skeleton.id = numplayers;
                        player[numplayers].updatedelay = 0;
                        player[numplayers].normalsupdatedelay = 0;

                        player[numplayers].aitype = passivetype;
                        player[numplayers].aitarget = 0;

                        if(player[0].creature == wolftype) {
                            headprop = player[0].proportionhead.x / 1.1;
                            bodyprop = player[0].proportionbody.x / 1.1;
                            armprop = player[0].proportionarms.x / 1.1;
                            legprop = player[0].proportionlegs.x / 1.1;
                        }

                        if(player[0].creature == rabbittype) {
                            headprop = player[0].proportionhead.x / 1.2;
                            bodyprop = player[0].proportionbody.x / 1.05;
                            armprop = player[0].proportionarms.x / 1.00;
                            legprop = player[0].proportionlegs.x / 1.1;
                        }

                        if(player[numplayers].creature == wolftype) {
                            player[numplayers].proportionhead = 1.1 * headprop;
                            player[numplayers].proportionbody = 1.1 * bodyprop;
                            player[numplayers].proportionarms = 1.1 * armprop;
                            player[numplayers].proportionlegs = 1.1 * legprop;
                        }

                        if(player[numplayers].creature == rabbittype) {
                            player[numplayers].proportionhead = 1.2 * headprop;
                            player[numplayers].proportionbody = 1.05 * bodyprop;
                            player[numplayers].proportionarms = 1.00 * armprop;
                            player[numplayers].proportionlegs = 1.1 * legprop;
                            player[numplayers].proportionlegs.y = 1.05 * legprop;
                        }

                        player[numplayers].headless = 0;
                        player[numplayers].onfire = 0;

                        if(cellophane) {
                            player[numplayers].proportionhead.z = 0;
                            player[numplayers].proportionbody.z = 0;
                            player[numplayers].proportionarms.z = 0;
                            player[numplayers].proportionlegs.z = 0;
                        }

                        player[numplayers].tempanimation.Load((char*)"animations/tempanim.anim", 0, 0);

                        player[numplayers].damagetolerance = 200;

                        player[numplayers].protectionhead = player[0].protectionhead;
                        player[numplayers].protectionhigh = player[0].protectionhigh;
                        player[numplayers].protectionlow = player[0].protectionlow;
                        player[numplayers].armorhead = player[0].armorhead;
                        player[numplayers].armorhigh = player[0].armorhigh;
                        player[numplayers].armorlow = player[0].armorlow;
                        player[numplayers].metalhead = player[0].metalhead;
                        player[numplayers].metalhigh = player[0].metalhigh;
                        player[numplayers].metallow = player[0].metallow;

                        player[numplayers].immobile = player[0].immobile;

                        player[numplayers].numclothes = player[0].numclothes;
                        if(player[numplayers].numclothes)
                            for(i = 0; i < player[numplayers].numclothes; i++) {
                                strcpy(player[numplayers].clothes[i], player[0].clothes[i]);
                                player[numplayers].clothestintr[i] = player[0].clothestintr[i];
                                player[numplayers].clothestintg[i] = player[0].clothestintg[i];
                                player[numplayers].clothestintb[i] = player[0].clothestintb[i];
                                tintr = player[numplayers].clothestintr[i];
                                tintg = player[numplayers].clothestintg[i];
                                tintb = player[numplayers].clothestintb[i];
                                AddClothes((char*)player[numplayers].clothes[i],
                                           0,
                                           1,
                                           &player[numplayers].skeleton.skinText[0],
                                           &player[numplayers].skeleton.skinsize);
                            }
                        if(player[numplayers].numclothes) {
                            player[numplayers].DoMipmaps(
                                5, 0, 0, player[numplayers].skeleton.skinsize, player[numplayers].skeleton.skinsize);
                        }

                        player[numplayers].power = player[0].power;
                        player[numplayers].speedmult = player[0].speedmult;

                        player[numplayers].damage = 0;
                        player[numplayers].permanentdamage = 0;
                        player[numplayers].superpermanentdamage = 0;
                        player[numplayers].deathbleeding = 0;
                        player[numplayers].bleeding = 0;
                        player[numplayers].numwaypoints = 0;
                        player[numplayers].waypoint = 0;
                        player[numplayers].jumppath = 0;
                        player[numplayers].weaponstuck = -1;
                        player[numplayers].weaponactive = -1;
                        player[numplayers].numweapons = 0;
                        player[numplayers].bloodloss = 0;
                        player[numplayers].dead = 0;

                        player[numplayers].loaded = 1;

                        numplayers++;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_P, KMOD_SHIFT) && !drawmodetogglekeydown && editorenabled) {
                    if(player[numplayers - 1].numwaypoints < 90) {
                        player[numplayers - 1].waypoints[player[numplayers - 1].numwaypoints] = player[0].coords;
                        player[numplayers - 1].waypointtype[player[numplayers - 1].numwaypoints] = editorpathtype;
                        player[numplayers - 1].numwaypoints++;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_P, KMOD_CTRL) && !drawmodetogglekeydown && editorenabled) {
                    if(numpathpoints < 30) {
                        bool connected, alreadyconnected;
                        connected = 0;
                        if(numpathpoints > 1)
                            for(i = 0; i < numpathpoints; i++) {
                                if(findDistancefast(&pathpoint[i], &player[0].coords) < .5 && i != pathpointselected &&
                                   !connected) {
                                    alreadyconnected = 0;
                                    for(j = 0; j < numpathpointconnect[pathpointselected]; j++) {
                                        if(pathpointconnect[pathpointselected][j] == i)
                                            alreadyconnected = 1;
                                    }
                                    if(!alreadyconnected) {
                                        numpathpointconnect[pathpointselected]++;
                                        connected = 1;
                                        pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected] - 1] = i;
                                    }
                                }
                            }
                        if(!connected) {
                            numpathpoints++;
                            pathpoint[numpathpoints - 1] = player[0].coords;
                            numpathpointconnect[numpathpoints - 1] = 0;
                            if(numpathpoints > 1 && pathpointselected != -1) {
                                numpathpointconnect[pathpointselected]++;
                                pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected] - 1] =
                                    numpathpoints - 1;
                            }
                            pathpointselected = numpathpoints - 1;
                        }
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_PERIOD) && !drawmodetogglekeydown && editorenabled) {
                    pathpointselected++;
                    if(pathpointselected >= numpathpoints)
                        pathpointselected = -1;
                    drawmodetogglekeydown = 1;
                }
                if(IsKeyDown(SDL_SCANCODE_COMMA, ~KMOD_SHIFT) && !drawmodetogglekeydown && editorenabled) {
                    pathpointselected--;
                    if(pathpointselected <= -2)
                        pathpointselected = numpathpoints - 1;
                    drawmodetogglekeydown = 1;
                }
                if(IsKeyDown(SDL_SCANCODE_COMMA, KMOD_SHIFT) && !drawmodetogglekeydown &&
                   editorenabled) {
                    if(pathpointselected != -1) {
                        numpathpoints--;
                        pathpoint[pathpointselected] = pathpoint[numpathpoints];
                        numpathpointconnect[pathpointselected] = numpathpointconnect[numpathpoints];
                        for(i = 0; i < numpathpointconnect[pathpointselected]; i++) {
                            pathpointconnect[pathpointselected][i] = pathpointconnect[numpathpoints][i];
                        }
                        for(i = 0; i < numpathpoints; i++) {
                            for(j = 0; j < numpathpointconnect[i]; j++) {
                                if(pathpointconnect[i][j] == pathpointselected) {
                                    pathpointconnect[i][j] = pathpointconnect[i][numpathpointconnect[i] - 1];
                                    numpathpointconnect[i]--;
                                }
                                if(pathpointconnect[i][j] == numpathpoints) {
                                    pathpointconnect[i][j] = pathpointselected;
                                }
                            }
                        }
                        pathpointselected = numpathpoints - 1;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_M, KMOD_SHIFT) && !drawmodetogglekeydown && config.game_debug) {
                    editorenabled = 1 - editorenabled;
                    if(editorenabled) {
                        player[0].damagetolerance = 100000;
                        player[0].damage = 0;
                        player[0].superpermanentdamage = 0;
                        player[0].bloodloss = 0;
                        player[0].deathbleeding = 0;
                    }
                    if(!editorenabled) {
                        player[0].damagetolerance = 200;
                        player[0].damage = 0;
                        player[0].permanentdamage = 0;
                        player[0].superpermanentdamage = 0;
                        player[0].bloodloss = 0;
                        player[0].deathbleeding = 0;
                    }
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_LEFT, KMOD_SHIFT | ~KMOD_CTRL) && !drawmodetogglekeydown && editorenabled) {
                    editortype--;
                    if(editortype == treeleavestype || editortype == 10)
                        editortype--;
                    if(editortype < 0)
                        editortype = firetype;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_RIGHT, KMOD_SHIFT | ~KMOD_CTRL) && !drawmodetogglekeydown && editorenabled) {
                    editortype++;
                    if(editortype == treeleavestype || editortype == 10)
                        editortype++;
                    if(editortype > firetype)
                        editortype = 0;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_LEFT, ~KMOD_SHIFT | ~KMOD_CTRL) && editorenabled) {
                    editorrotation -= multiplier * 100;
                    if(editorrotation < -.01)
                        editorrotation = -.01;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_RIGHT, ~KMOD_SHIFT | ~KMOD_CTRL) && editorenabled) {
                    editorrotation += multiplier * 100;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_UP, ~KMOD_CTRL) && editorenabled) {
                    editorsize += multiplier;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_DOWN, ~KMOD_CTRL) && editorenabled) {
                    editorsize -= multiplier;
                    if(editorsize < .1)
                        editorsize = .1;
                    drawmodetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_LEFT, KMOD_CTRL | KMOD_SHIFT) && !drawmodetogglekeydown && editorenabled) {
                    mapradius -= multiplier * 10;
                }

                if(IsKeyDown(SDL_SCANCODE_RIGHT, KMOD_SHIFT | KMOD_CTRL) && !drawmodetogglekeydown && editorenabled) {
                    mapradius += multiplier * 10;
                }
                /*
                if(IsKeyDown(theKeyMap, MAC_ARROW_LEFT_KEY)&&editorenabled&&!IsKeyDown(theKeyMap,
                MAC_SHIFT_KEY)&&IsKeyDown(theKeyMap, MAC_CONTROL_KEY)){
                mapcenter.x+=multiplier*20;
                }

                if(IsKeyDown(theKeyMap, MAC_ARROW_RIGHT_KEY)&&editorenabled&&!IsKeyDown(theKeyMap,
                MAC_SHIFT_KEY)&&IsKeyDown(theKeyMap, MAC_CONTROL_KEY)){
                mapcenter.x-=multiplier*20;
                }

                if(IsKeyDown(theKeyMap, MAC_ARROW_UP_KEY)&&editorenabled&&IsKeyDown(theKeyMap, MAC_CONTROL_KEY)){
                mapcenter.z+=multiplier*20;
                }

                if(IsKeyDown(theKeyMap, MAC_ARROW_DOWN_KEY)&&editorenabled&&IsKeyDown(theKeyMap, MAC_CONTROL_KEY)){
                mapcenter.z-=multiplier*20;
                }
                */
                if(IsKeyDown(SDL_SCANCODE_UP, KMOD_CTRL) && editorenabled) {
                    editorrotation2 += multiplier * 100;
                }

                if(IsKeyDown(SDL_SCANCODE_DOWN, KMOD_CTRL) && editorenabled) {
                    editorrotation2 -= multiplier * 100;
                    if(editorrotation2 < -.01)
                        editorrotation2 = -.01;
                }
                if(IsKeyDown(SDL_SCANCODE_DELETE, ~KMOD_SHIFT) && editorenabled && objects.numobjects && !drawmodetogglekeydown) {
                    int closest = -1;
                    float closestdist = -1;
                    float distance;
                    for(i = 0; i < objects.numobjects; i++) {
                        distance = findDistancefast(&objects.position[i], &player[0].coords);
                        if(closestdist == -1 || distance < closestdist) {
                            closestdist = distance;
                            closest = i;
                        }
                    }
                    if(closestdist > 0 && closest >= 0)
                        objects.DeleteObject(closest);
                    drawmodetogglekeydown = 1;
                }

                if(!IsKeyDown(SDL_SCANCODE_M) && !IsKeyDown(SDL_SCANCODE_LEFT) &&
                   !IsKeyDown(SDL_SCANCODE_COMMA) && !IsKeyDown(SDL_SCANCODE_PERIOD) &&
                   !IsKeyDown(SDL_SCANCODE_RIGHT) && !IsKeyDown(SDL_SCANCODE_DELETE) &&
                   !IsKeyDown(SDL_SCANCODE_P)) {
                    drawmodetogglekeydown = 0;
                }

                if(IsKeyDown(SDL_SCANCODE_N, ~KMOD_CTRL) && !texturesizetogglekeydown && config.game_debug) {
                    // if(!player[0].skeleton.free)player[0].damage+=500;
                    player[0].RagDoll(0);
                    // player[0].spurt=1;
                    // player[0].DoDamage(1000);

                    float gLoc[3];
                    float vel[3];
                    gLoc[0] = player[0].coords.x;
                    gLoc[1] = player[0].coords.y;
                    gLoc[2] = player[0].coords.z;
                    vel[0] = player[0].velocity.x;
                    vel[1] = player[0].velocity.y;
                    vel[2] = player[0].velocity.z;
                    PlaySoundEx(whooshsound, samp[whooshsound], NULL, true);
                    OPENAL_3D_SetAttributes(channels[whooshsound], gLoc, vel);
                    OPENAL_SetVolume(channels[whooshsound], 128);
                    OPENAL_SetPaused(channels[whooshsound], false);
                    // OPENAL_SetPaused(channels[whooshsound], true);

                    texturesizetogglekeydown = 1;
                }

                if(IsKeyDown(SDL_SCANCODE_N, KMOD_CTRL) && !texturesizetogglekeydown && config.game_debug)
                {
                    for(i = 0; i < objects.numobjects; i++) {
                        if(objects.type[i] == treeleavestype) {
                            objects.scale[i] *= .9;
                        }
                    }
                    texturesizetogglekeydown = 1;
                }

                static XYZ relative;
                static int randattack;
                // Attack
                static bool playerrealattackkeydown = 0;

                if (!IsButtonPressed(keys.attack))
                    keys.oldattack = 0;
                if (keys.oldattack)
                    player[0].attackkeydown = 0;
                if (keys.oldattack)
                    playerrealattackkeydown = 0;
                if (!keys.oldattack)
                    playerrealattackkeydown = IsButtonPressed(keys.attack);
                if ((player[0].parriedrecently <= 0 || player[0].weaponactive == -1) &&
                   (!keys.oldattack ||
                    (realthreat && player[0].lastattack != swordslashanim && player[0].lastattack != knifeslashstartanim &&
                     player[0].lastattack != staffhitanim && player[0].lastattack != staffspinhitanim)))
                    player[0].attackkeydown = IsButtonPressed(keys.attack);
                if (IsButtonPressed(keys.attack) && !keys.oldattack && !player[0].backkeydown) {
                    for(k = 0; k < numplayers; k++) {
                        if((player[k].targetanimation == swordslashanim || player[k].targetanimation == staffhitanim ||
                            player[k].targetanimation == staffspinhitanim) &&
                           player[0].currentanimation != dodgebackanim && !player[k].skeleton.free)
                            player[k].Reverse();
                    }
                }

                if(!hostile || indialogue != -1)
                    player[0].attackkeydown = 0;

                for(k = 0; k < numplayers; k++) {
                    if(indialogue != -1)
                        player[k].attackkeydown = 0;
                    if(player[k].targetanimation != rabbitrunninganim && player[k].targetanimation != wolfrunninganim) {
                        if(player[k].aitype != playercontrolled)
                            player[k].victim = &player[0];
                        if(player[k].attackkeydown && player[k].jumppower <= 1 && player[k].backkeydown &&
                           player[k].targetanimation != backhandspringanim &&
                           (player[k].isIdle() || player[k].isStop() || player[k].isRun() ||
                            player[k].targetanimation == walkanim)) {
                            player[k].jumppower -= 2;
                        }
                        if(player[k].attackkeydown && player[k].jumppower > 1 && player[k].backkeydown &&
                           player[k].targetanimation != backhandspringanim &&
                           (player[k].isIdle() || player[k].isStop() || player[k].isRun() ||
                            player[k].targetanimation == walkanim)) {
                            for(i = 0; i < numplayers; i++) {
                                if(i == k)
                                    i++;
                                if(player[i].targetanimation == swordslashanim ||
                                   player[i].targetanimation == knifeslashstartanim ||
                                   player[i].targetanimation == staffhitanim ||
                                   player[i].targetanimation == staffspinhitanim)
                                    if(findDistancefast(&player[k].coords, &player[i].coords) < 6.5 &&
                                       !player[i].skeleton.free) {
                                        player[k].targetanimation = dodgebackanim;
                                        player[k].target = 0;
                                        player[k].targetframe = 0;
                                        rotatetarget = player[i].coords - player[k].coords;
                                        Normalise(&rotatetarget);
                                        player[k].targetrotation = -asin(0 - rotatetarget.x);
                                        player[k].targetrotation *= 360 / 6.28;
                                        if(rotatetarget.z < 0)
                                            player[k].targetrotation = 180 - player[k].targetrotation;

                                        player[k].targettilt2 = -asin(rotatetarget.y) * 360 / 6.28;
                                    }
                            }
                            if(player[k].targetanimation != dodgebackanim) {
                                if(k == 0)
                                    numflipped++;
                                player[k].targetanimation = backhandspringanim;
                                player[k].target = 0;
                                player[k].targetframe = 0;
                                player[k].targetrotation = -rotation + 180;
                                if(player[k].leftkeydown)
                                    player[k].targetrotation -= 45;
                                if(player[k].rightkeydown)
                                    player[k].targetrotation += 45;
                                player[k].rotation = player[k].targetrotation;
                                player[k].jumppower -= 2;
                            }
                        }
                        if(player[k].attackkeydown && !animation[player[k].targetanimation].attack &&
                           !player[k].backkeydown &&
                           (player[k].isIdle() || player[k].isRun() || player[k].targetanimation == walkanim ||
                            player[k].targetanimation == sneakanim || player[k].isCrouch())) {
                            player[k].hasvictim = 0;
                            if(numplayers > 1)
                                for(i = 0; i < numplayers; i++) {
                                    if(i == k)
                                        i++;
                                    if(!player[k].hasvictim)
                                        if((k == 0 || i == 0) && i != k && i < numplayers && k < numplayers &&
                                           animation[player[k].targetanimation].attack != reversal) {
                                            if(findDistancefast(&player[k].coords, &player[i].coords) < 4.5 &&
                                               !player[i].skeleton.free && player[i].howactive < typedead1 &&
                                               player[i].targetanimation != jumpreversedanim &&
                                               player[i].targetanimation != rabbitkickreversedanim &&
                                               player[i].targetanimation != rabbitkickanim &&
                                               player[k].targetanimation != rabbitkickanim &&
                                               player[i].targetanimation != getupfrombackanim &&
                                               (player[i].targetanimation != staggerbackhighanim &&
                                                (player[i].targetanimation != staggerbackhardanim ||
                                                 animation[staggerbackhardanim].label[player[i].targetframe] == 6)) &&
                                               player[i].targetanimation != jumpdownanim &&
                                               player[i].targetanimation != jumpupanim &&
                                               player[i].targetanimation != getupfromfrontanim) {
                                                player[k].victim = &player[i];
                                                player[k].hasvictim = 1;
                                                if(player[k].aitype == playercontrolled) {
                                                    if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                           2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                       player[k].crouchkeydown &&
                                                       animation[player[i].targetanimation].height != lowheight)
                                                        player[k].targetanimation = sweepanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                1.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].forwardkeydown && !player[k].leftkeydown &&
                                                            !player[k].rightkeydown && !player[k].crouchkeydown &&
                                                            player[k].weaponactive == -1 && !reversaltrain)
                                                        player[k].targetanimation = winduppunchanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].forwardkeydown && !player[k].leftkeydown &&
                                                            !player[k].rightkeydown && !player[k].crouchkeydown &&
                                                            player[k].weaponactive == -1)
                                                        player[k].targetanimation = upunchanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            player[k].weaponactive != -1 && player[i].staggerdelay > 0 &&
                                                            weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                                knife &&
                                                            player[i].bloodloss > player[i].damagetolerance / 2)
                                                        player[k].targetanimation = knifefollowanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].forwardkeydown && !player[k].leftkeydown &&
                                                            !player[k].rightkeydown && !player[k].crouchkeydown &&
                                                            player[k].weaponactive != -1 &&
                                                            weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                                knife &&
                                                            player[k].weaponmissdelay <= 0)
                                                        player[k].targetanimation = knifeslashstartanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                4.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].crouchkeydown && player[k].weaponactive != -1 &&
                                                            weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                                sword &&
                                                            player[k].weaponmissdelay <= 0)
                                                        player[k].targetanimation = swordslashanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                4.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].crouchkeydown && player[k].weaponactive != -1 &&
                                                            weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                                staff &&
                                                            player[k].weaponmissdelay <= 0 && !player[k].leftkeydown &&
                                                            !player[k].rightkeydown && !player[k].forwardkeydown)
                                                        player[k].targetanimation = staffhitanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                4.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight &&
                                                            !player[k].crouchkeydown && player[k].weaponactive != -1 &&
                                                            weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                                staff &&
                                                            player[k].weaponmissdelay <= 0)
                                                        player[k].targetanimation = staffspinhitanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height != lowheight)
                                                        player[k].targetanimation = spinkickanim;
                                                    else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                            animation[player[i].targetanimation].height == lowheight &&
                                                            animation[player[k].targetanimation].attack != normalattack)
                                                        player[k].targetanimation = lowkickanim;
                                                } else {
                                                    if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                       4.5 * (player[k].scale * 5) * (player[k].scale * 5)) {
                                                        if(player[k].weaponactive == -1)
                                                            randattack = abs(rand() % 5);
                                                        else
                                                            randattack = abs(rand() % 5);
                                                        if(player[k].weaponactive == -1 &&
                                                           findDistancefast(&player[k].coords, &player[i].coords) <
                                                               2.5 * (player[k].scale * 5) * (player[k].scale * 5)) {
                                                            if(randattack == 0 &&
                                                               animation[player[i].targetanimation].height != lowheight)
                                                                player[k].targetanimation = sweepanim;
                                                            else if(randattack == 1 &&
                                                                    animation[player[i].targetanimation].height !=
                                                                        lowheight &&
                                                                    player[k].weaponactive == -1)
                                                                player[k].targetanimation = upunchanim;
                                                            else if(randattack == 2 &&
                                                                    animation[player[i].targetanimation].height !=
                                                                        lowheight)
                                                                player[k].targetanimation = spinkickanim;
                                                            else if(animation[player[i].targetanimation].height ==
                                                                    lowheight)
                                                                player[k].targetanimation = lowkickanim;
                                                        }
                                                        if(player[k].weaponactive != -1) {
                                                            if((tutoriallevel != 1 || player[k].weaponactive == -1) &&
                                                               findDistancefast(&player[k].coords, &player[i].coords) <
                                                                   2.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                               randattack == 0 &&
                                                               animation[player[i].targetanimation].height != lowheight)
                                                                player[k].targetanimation = sweepanim;
                                                            else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                        2.5 * (player[k].scale * 5) *
                                                                            (player[k].scale *
                                                                             5) /*&&animation[player[i].targetanimation].height!=lowheight*/ &&
                                                                    weapons.type[player[k]
                                                                                     .weaponids[player[k].weaponactive]] ==
                                                                        knife &&
                                                                    player[k].weaponmissdelay <= 0)
                                                                player[k].targetanimation = knifeslashstartanim;
                                                            // else
                                                            // if(findDistancefast(&player[k].coords,&player[i].coords)<2.5&&player[k].weaponactive!=-1&&player[i].staggerdelay>0&&weapons.type[player[k].weaponids[player[k].weaponactive]]==knife)player[k].targetanimation=knifefollowanim;
                                                            else if(!(player[0].victim == &player[i] &&
                                                                      player[0].hasvictim &&
                                                                      player[0].targetanimation == swordslashanim) &&
                                                                    weapons.type[player[k]
                                                                                     .weaponids[player[k].weaponactive]] ==
                                                                        sword &&
                                                                    player[k].weaponmissdelay <= 0)
                                                                player[k].targetanimation = swordslashanim;
                                                            else if(!(player[0].victim == &player[i] &&
                                                                      player[0].hasvictim &&
                                                                      player[0].targetanimation == swordslashanim) &&
                                                                    weapons.type[player[k]
                                                                                     .weaponids[player[k].weaponactive]] ==
                                                                        staff &&
                                                                    player[k].weaponmissdelay <= 0 && randattack < 3)
                                                                player[k].targetanimation = staffhitanim;
                                                            else if(!(player[0].victim == &player[i] &&
                                                                      player[0].hasvictim &&
                                                                      player[0].targetanimation == swordslashanim) &&
                                                                    weapons.type[player[k]
                                                                                     .weaponids[player[k].weaponactive]] ==
                                                                        staff &&
                                                                    player[k].weaponmissdelay <= 0 && randattack >= 3)
                                                                player[k].targetanimation = staffspinhitanim;
                                                            else if((tutoriallevel != 1 || player[k].weaponactive == -1) &&
                                                                    findDistancefast(&player[k].coords, &player[i].coords) <
                                                                        2.5 * (player[k].scale * 5) *
                                                                            (player[k].scale * 5) &&
                                                                    randattack == 1 &&
                                                                    animation[player[i].targetanimation].height !=
                                                                        lowheight)
                                                                player[k].targetanimation = spinkickanim;
                                                            else if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                                        2.5 * (player[k].scale * 5) *
                                                                            (player[k].scale * 5) &&
                                                                    animation[player[i].targetanimation].height ==
                                                                        lowheight &&
                                                                    animation[player[k].targetanimation].attack !=
                                                                        normalattack)
                                                                player[k].targetanimation = lowkickanim;
                                                        }
                                                    }
                                                }
                                                if(player[k].targetanimation == upunchanim &&
                                                   player[k].creature == wolftype)
                                                    player[k].targetanimation = wolfslapanim;
                                            }
                                            if((tutoriallevel != 1 || tutorialstage == 22) &&
                                               player[i].howactive < typedead1 &&
                                               findDistancefast(&player[k].coords, &player[i].coords) <
                                                   1.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                               !player[i].skeleton.free && player[i].targetanimation != getupfrombackanim &&
                                               player[i].targetanimation != getupfromfrontanim &&
                                               (((player[i].stunned > 0 && player[k].madskills) ||
                                                 (player[i].surprised > 0) || player[i].aitype == passivetype) ||
                                                (player[k].weaponactive != -1 && player[i].stunned > 0)) &&
                                               normaldotproduct(player[i].facing, player[i].coords - player[k].coords) >
                                                   0 &&
                                               (k == 0)) {
                                                if(player[k].weaponactive == -1) {
                                                    player[i].targetanimation = sneakattackedanim;
                                                    player[i].currentanimation = sneakattackedanim;
                                                    player[k].currentanimation = sneakattackanim;
                                                    player[k].targetanimation = sneakattackanim;
                                                    player[k].oldcoords = player[k].coords;
                                                    player[k].coords = player[i].coords;
                                                }
                                                if(player[k].weaponactive != -1 &&
                                                   weapons.type[player[k].weaponids[player[k].weaponactive]] == knife) {
                                                    player[i].targetanimation = knifesneakattackedanim;
                                                    player[i].currentanimation = knifesneakattackedanim;
                                                    player[k].currentanimation = knifesneakattackanim;
                                                    player[k].targetanimation = knifesneakattackanim;
                                                    player[i].oldcoords = player[i].coords;
                                                    player[i].coords = player[k].coords;
                                                }
                                                if(player[k].weaponactive != -1 &&
                                                   weapons.type[player[k].weaponids[player[k].weaponactive]] == sword) {
                                                    player[i].targetanimation = swordsneakattackedanim;
                                                    player[i].currentanimation = swordsneakattackedanim;
                                                    player[k].currentanimation = swordsneakattackanim;
                                                    player[k].targetanimation = swordsneakattackanim;
                                                    player[i].oldcoords = player[i].coords;
                                                    player[i].coords = player[k].coords;
                                                }
                                                if(player[k].weaponactive == -1 ||
                                                   weapons.type[player[k].weaponids[player[k].weaponactive]] != staff) {
                                                    player[k].victim = &player[i];
                                                    player[k].hasvictim = 1;
                                                    player[i].targettilt2 = 0;
                                                    player[i].targetframe = 1;
                                                    player[i].currentframe = 0;
                                                    player[i].target = 0;
                                                    player[i].velocity = 0;
                                                    player[k].targettilt2 = player[i].targettilt2;
                                                    player[k].currentframe = player[i].currentframe;
                                                    player[k].targetframe = player[i].targetframe;
                                                    player[k].target = player[i].target;
                                                    player[k].velocity = 0;
                                                    player[k].targetrotation = player[i].rotation;
                                                    player[k].rotation = player[i].rotation;
                                                    player[i].targetrotation = player[i].rotation;
                                                }
                                            }
                                            if(animation[player[k].targetanimation].attack == normalattack &&
                                               player[k].victim == &player[i] && (!player[i].skeleton.free)) {
                                                keys.oldattack = 1;
                                                player[k].targetframe = 0;
                                                player[k].target = 0;
                                                // player[k].velocity=0;

                                                rotatetarget = player[i].coords - player[k].coords;
                                                Normalise(&rotatetarget);
                                                player[k].targetrotation = -asin(0 - rotatetarget.x);
                                                player[k].targetrotation *= 360 / 6.28;
                                                if(rotatetarget.z < 0)
                                                    player[k].targetrotation = 180 - player[k].targetrotation;

                                                player[k].targettilt2 = -asin(rotatetarget.y) * 360 / 6.28; //*-70;

                                                player[k].lastattack3 = player[k].lastattack2;
                                                player[k].lastattack2 = player[k].lastattack;
                                                player[k].lastattack = player[k].targetanimation;
                                                // player[k].targettilt2=0;
                                                // slomo=1;
                                                // slomodelay=.2;
                                            }
                                            if(player[k].targetanimation == knifefollowanim &&
                                               player[k].victim == &player[i]) {
                                                rotatetarget = player[i].coords - player[k].coords;
                                                Normalise(&rotatetarget);
                                                player[k].targetrotation = -asin(0 - rotatetarget.x);
                                                player[k].targetrotation *= 360 / 6.28;
                                                if(rotatetarget.z < 0)
                                                    player[k].targetrotation = 180 - player[k].targetrotation;
                                                player[k].targettilt2 = -asin(rotatetarget.y) * 360 / 6.28; //*-70;
                                                keys.oldattack = 1;
                                                player[k].victim = &player[i];
                                                player[k].hasvictim = 1;
                                                player[i].targetanimation = knifefollowedanim;
                                                player[i].currentanimation = knifefollowedanim;
                                                player[i].targettilt2 = 0;
                                                player[i].targettilt2 = player[k].targettilt2;
                                                player[i].targetframe = 1;
                                                player[i].currentframe = 0;
                                                player[i].target = 0;
                                                player[i].velocity = 0;
                                                player[k].currentanimation = knifefollowanim;
                                                player[k].targetanimation = knifefollowanim;
                                                player[k].targettilt2 = player[i].targettilt2;
                                                player[k].currentframe = player[i].currentframe;
                                                player[k].targetframe = player[i].targetframe;
                                                player[k].target = player[i].target;
                                                player[k].velocity = 0;
                                                player[k].oldcoords = player[k].coords;
                                                player[i].coords = player[k].coords;
                                                player[i].targetrotation = player[k].targetrotation;
                                                player[i].rotation = player[k].targetrotation;
                                                player[k].rotation = player[k].targetrotation;
                                                player[i].rotation = player[k].targetrotation;
                                            }
                                        }
                                }
                            bool hasstaff = 0;
                            if(player[k].weaponactive != -1) {
                                if(weapons.type[player[k].weaponids[player[k].weaponactive]] == staff)
                                    hasstaff = 1;
                            }
                            if(numplayers > 1)
                                for(i = 0; i < numplayers; i++) {
                                    if(i == k)
                                        i++;
                                    if((playerrealattackkeydown || player[i].dead || !hasstaff) && (k == 0 || i == 0) &&
                                       i != k && i < numplayers && k < numplayers &&
                                       animation[player[k].targetanimation].attack == neutral && k == 0) {
                                        if(!player[i].dead || !realthreat ||
                                           (player[k].weaponactive == -1 && player[k].crouchkeydown))
                                            if(player[i].skeleton.free)
                                                if(findDistancefast(&player[k].coords, &player[i].coords) <
                                                       3.5 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                                   (player[i].dead || player[i].skeleton.longdead > 1000 ||
                                                    player[k].isRun() || (hasstaff) ||
                                                    (player[k].weaponactive != -1 && player[i].skeleton.free &&
                                                     (player[i].skeleton.longdead > 2000 ||
                                                      player[i].damage > player[i].damagetolerance / 8 ||
                                                      player[i].bloodloss > player[i].damagetolerance / 2) &&
                                                     findDistancefast(&player[k].coords, &player[i].coords) <
                                                         1.5 * (player[k].scale * 5) * (player[k].scale * 5)))) {
                                                    player[k].victim = &player[i];
                                                    player[k].hasvictim = 1;
                                                    if(player[k].weaponactive != -1 && tutoriallevel != 1) {
                                                        if(player[k].crouchkeydown && player[k].weaponactive != -1 &&
                                                           weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                               knife &&
                                                           findDistancefast(&player[k].coords, &player[i].coords) <
                                                               1.5 * (player[k].scale * 5) * (player[k].scale * 5))
                                                            player[k].targetanimation = crouchstabanim;
                                                        if(player[k].crouchkeydown &&
                                                           findDistancefast(&player[k].coords, &player[i].coords) <
                                                               1.5 * (player[k].scale * 5) *
                                                                   (player[k].scale * 5) /*&&player[i].dead!=2*/ &&
                                                           weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                               sword)
                                                            player[k].targetanimation = swordgroundstabanim;
                                                        if(/*(player[k].crouchkeydown||!player[i].dead)&&*/ findDistancefast(
                                                               &player[k].coords, &player[i].coords) <
                                                               3.5 * (player[k].scale * 5) *
                                                                   (player[k].scale * 5) /*&&player[i].dead!=2*/ &&
                                                           weapons.type[player[k].weaponids[player[k].weaponactive]] ==
                                                               staff)
                                                            player[k].targetanimation = staffgroundsmashanim;
                                                    }
                                                    if(findDistancefast(&player[k].coords, &player[i].coords) < 2.5 &&
                                                       player[k].crouchkeydown &&
                                                       player[k].targetanimation != crouchstabanim &&
                                                       (player[k].weaponactive == -1) && player[i].dead &&
                                                       player[i].skeleton.free && player[i].skeleton.longdead > 1000) {
                                                        player[k].targetanimation = killanim;
                                                        for(j = 0; j < terrain.numdecals; j++) {
                                                            if((terrain.decaltype[j] == blooddecal ||
                                                                terrain.decaltype[j] == blooddecalslow) &&
                                                               terrain.decalalivetime[j] < 2) {
                                                                terrain.DeleteDecal(j);
                                                            }
                                                        }
                                                        for(l = 0; l < objects.numobjects; l++) {
                                                            if(objects.model[l].type == decalstype)
                                                                for(j = 0; j < objects.model[l].numdecals; j++) {
                                                                    if((objects.model[l].decaltype[j] == blooddecal ||
                                                                        objects.model[l].decaltype[j] == blooddecalslow) &&
                                                                       objects.model[l].decalalivetime[j] < 2) {
                                                                        objects.model[l].DeleteDecal(j);
                                                                    }
                                                                }
                                                        }
                                                    }
                                                    if(!player[i].dead || musictype != 2)
                                                        if(findDistancefast(&player[k].coords, &player[i].coords) < 3.5 &&
                                                           (player[k].isRun() ||
                                                            (player[k].isIdle() && player[k].attackkeydown)) &&
                                                           (player[k].staggerdelay <= 0 &&
                                                            (player[i].dead || (player[i].skeleton.longdead < 300 &&
                                                                                player[k].lastattack != spinkickanim &&
                                                                                player[i].skeleton.free))) &&
                                                           (!player[i].dead || musictype != stream_music2)) {
                                                            player[k].targetanimation = dropkickanim;
                                                            for(j = 0; j < terrain.numdecals; j++) {
                                                                if((terrain.decaltype[j] == blooddecal ||
                                                                    terrain.decaltype[j] == blooddecalslow) &&
                                                                   terrain.decalalivetime[j] < 2) {
                                                                    terrain.DeleteDecal(j);
                                                                }
                                                            }
                                                            for(l = 0; l < objects.numobjects; l++) {
                                                                if(objects.model[l].type == decalstype)
                                                                    for(j = 0; j < objects.model[l].numdecals; j++) {
                                                                        if((objects.model[l].decaltype[j] == blooddecal ||
                                                                            objects.model[l].decaltype[j] ==
                                                                                blooddecalslow) &&
                                                                           objects.model[l].decalalivetime[j] < 2) {
                                                                            objects.model[l].DeleteDecal(j);
                                                                        }
                                                                    }
                                                            }
                                                        }
                                                }
                                        if(animation[player[k].targetanimation].attack == normalattack &&
                                           player[k].victim == &player[i] &&
                                           (!player[i].skeleton.free || player[k].targetanimation == killanim ||
                                            player[k].targetanimation == crouchstabanim ||
                                            player[k].targetanimation == swordgroundstabanim ||
                                            player[k].targetanimation == staffgroundsmashanim ||
                                            player[k].targetanimation == dropkickanim)) {
                                            keys.oldattack = 1;
                                            player[k].targetframe = 0;
                                            player[k].target = 0;
                                            // player[k].velocity=0;

                                            rotatetarget = player[i].coords - player[k].coords;
                                            if(player[k].targetanimation == crouchstabanim ||
                                               player[k].targetanimation == swordgroundstabanim ||
                                               player[k].targetanimation == staffgroundsmashanim) {
                                                rotatetarget =
                                                    (player[i].coords +
                                                     (player[i]
                                                          .skeleton.joints[player[i].skeleton.jointlabels[abdomen]]
                                                          .position +
                                                      player[i]
                                                          .skeleton.joints[player[i].skeleton.jointlabels[neck]]
                                                          .position) /
                                                         2 * player[i].scale) -
                                                    player[k].coords;
                                            }
                                            Normalise(&rotatetarget);
                                            player[k].targetrotation = -asin(0 - rotatetarget.x);
                                            player[k].targetrotation *= 360 / 6.28;
                                            if(rotatetarget.z < 0)
                                                player[k].targetrotation = 180 - player[k].targetrotation;

                                            if(player[k].targetanimation == crouchstabanim ||
                                               player[k].targetanimation == swordgroundstabanim) {
                                                player[k].targetrotation += (float)(abs(rand() % 100) - 50) / 4;
                                            }

                                            player[k].targettilt2 = -asin(rotatetarget.y) * 360 / 6.28; //*-70;
                                            if(player[k].targetanimation == staffgroundsmashanim)
                                                player[k].targettilt2 += 10;

                                            player[k].lastattack3 = player[k].lastattack2;
                                            player[k].lastattack2 = player[k].lastattack;
                                            player[k].lastattack = player[k].targetanimation;

                                            if(player[k].targetanimation == swordgroundstabanim) {
                                                player[k].targetrotation += 30;
                                            }
                                            // player[k].targettilt2=0;
                                            // slomo=1;
                                            // slomodelay=.2;
                                        }
                                    }
                                }
                            if(!player[k].hasvictim) {
                                for(i = 0; i < numplayers; i++) {
                                    if((player[k].hasvictim == 0) && i != k && (i == 0 || k == 0) &&
                                       !player[i].skeleton.free) {
                                        player[k].victim = &player[i];
                                        player[k].hasvictim = 1;
                                    }
                                    if(player[k].hasvictim && !player[i].skeleton.free)
                                        if(findDistancefast(&player[k].coords, &player[i].coords) <
                                               findDistancefast(&player[k].coords, &player[k].victim->coords) &&
                                           i != k && (i == 0 || k == 0)) {
                                            player[k].victim = &player[i];
                                        }
                                }
                            }
                            if(player[k].aitype == playercontrolled)
                                if(player[k].attackkeydown && (player[k].isRun()) && player[k].wasRun() &&
                                   ((player[k].hasvictim &&
                                     findDistancefast(&player[k].coords, &player[k].victim->coords) <
                                         12 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                     findDistancefast(&player[k].coords, &player[k].victim->coords) >
                                         7 * (player[k].scale * 5) * (player[k].scale * 5) &&
                                     !player[k].victim->skeleton.free &&
                                     player[k].victim->targetanimation != getupfrombackanim &&
                                     player[k].victim->targetanimation != getupfromfrontanim &&
                                     animation[player[k].victim->targetanimation].height != lowheight &&
                                     player[k].aitype != playercontrolled &&
                                     normaldotproduct(player[k].facing, player[k].victim->coords - player[k].coords) > 0 &&
                                     player[k].rabbitkickenabled) ||
                                    player[k].jumpkeydown)) {
                                    keys.oldattack = 1;
                                    player[k].targetanimation = rabbitkickanim;
                                    player[k].targetframe = 0;
                                    player[k].target = 0;
                                }
                            if(animation[player[k].targetanimation].attack && k == 0) {
                                numattacks++;
                                bool armedstaff = 0;
                                if(player[k].weaponactive != -1) {
                                    if(weapons.type[player[k].weaponids[player[k].weaponactive]] == staff)
                                        armedstaff = 1;
                                }
                                bool armedsword = 0;
                                if(player[k].weaponactive != -1) {
                                    if(weapons.type[player[k].weaponids[player[k].weaponactive]] == sword)
                                        armedsword = 1;
                                }
                                bool armedknife = 0;
                                if(player[k].weaponactive != -1) {
                                    if(weapons.type[player[k].weaponids[player[k].weaponactive]] == knife)
                                        armedknife = 1;
                                }
                                if(armedstaff)
                                    numstaffattack++;
                                else if(armedsword)
                                    numswordattack++;
                                else if(armedknife)
                                    numknifeattack++;
                                else
                                    numunarmedattack++;
                            }
                        }
                    }
                }

                // Collisions
                static float collisionradius;
                if(numplayers > 1)
                    for(k = 0; k < numplayers; k++) {
                        for(i = k; i < numplayers; i++) {
                            if(i == k)
                                i++;
                            if(i < numplayers)
                                if((animation[player[i].targetanimation].attack != reversed &&
                                    animation[player[i].targetanimation].attack != reversal &&
                                    animation[player[k].targetanimation].attack != reversed &&
                                    animation[player[k].targetanimation].attack != reversal) ||
                                   (i != 0 && k != 0))
                                    if((animation[player[i].currentanimation].attack != reversed &&
                                        animation[player[i].currentanimation].attack != reversal &&
                                        animation[player[k].currentanimation].attack != reversed &&
                                        animation[player[k].currentanimation].attack != reversal) ||
                                       (i != 0 && k != 0))
                                        if(player[i].howactive <= typesleeping && player[k].howactive <= typesleeping)
                                            if(player[i].howactive != typesittingwall &&
                                               player[k].howactive != typesittingwall)
                                                if(i != k && player[i].whichpatchx == player[k].whichpatchx &&
                                                   player[i].whichpatchz == player[k].whichpatchz &&
                                                   player[k].skeleton.oldfree == player[k].skeleton.free &&
                                                   player[i].skeleton.oldfree == player[i].skeleton.free &&
                                                   player[i].targetanimation != climbanim &&
                                                   player[i].targetanimation != hanganim &&
                                                   player[k].targetanimation != climbanim &&
                                                   player[k].targetanimation != hanganim)
                                                    if(player[i].coords.y > player[k].coords.y - 3)
                                                        if(player[i].coords.y < player[k].coords.y + 3)
                                                            if(player[i].coords.x > player[k].coords.x - 3)
                                                                if(player[i].coords.x < player[k].coords.x + 3)
                                                                    if(player[i].coords.z > player[k].coords.z - 3)
                                                                        if(player[i].coords.z < player[k].coords.z + 3) {
                                                                            if(findDistancefast(&player[i].coords,
                                                                                                &player[k].coords) <
                                                                               3 * ((player[i].scale + player[k].scale) *
                                                                                    2.5) *
                                                                                   ((player[i].scale + player[k].scale) *
                                                                                    2.5)) {
                                                                                if(player[i].onfire || player[k].onfire) {
                                                                                    if(!player[i].onfire)
                                                                                        player[i].CatchFire();
                                                                                    if(!player[k].onfire)
                                                                                        player[k].CatchFire();
                                                                                }
                                                                            }

                                                                            tempcoords1 = player[i].coords;
                                                                            tempcoords2 = player[k].coords;
                                                                            if(!player[i].skeleton.oldfree)
                                                                                tempcoords1.y +=
                                                                                    player[i]
                                                                                        .skeleton
                                                                                        .joints[player[i]
                                                                                                    .skeleton
                                                                                                    .jointlabels[abdomen]]
                                                                                        .position.y *
                                                                                    player[i].scale;
                                                                            if(!player[k].skeleton.oldfree)
                                                                                tempcoords2.y +=
                                                                                    player[k]
                                                                                        .skeleton
                                                                                        .joints[player[k]
                                                                                                    .skeleton
                                                                                                    .jointlabels[abdomen]]
                                                                                        .position.y *
                                                                                    player[k].scale;
                                                                            collisionradius =
                                                                                1.2 * ((player[i].scale + player[k].scale) *
                                                                                       2.5) *
                                                                                ((player[i].scale + player[k].scale) * 2.5);
                                                                            if(player[0].hasvictim)
                                                                                if(player[0].targetanimation ==
                                                                                       rabbitkickanim &&
                                                                                   (k == 0 || i == 0) &&
                                                                                   !player[0].victim->skeleton.free)
                                                                                    collisionradius = 3;
                                                                            if((!player[i].skeleton.oldfree ||
                                                                                !player[k].skeleton.oldfree) &&
                                                                               (findDistancefast(&tempcoords1,
                                                                                                 &tempcoords2) <
                                                                                    collisionradius ||
                                                                                findDistancefast(&player[i].coords,
                                                                                                 &player[k].coords) <
                                                                                    collisionradius)) {
                                                                                if(k == 0)
                                                                                    if(player[k].targetanimation ==
                                                                                           jumpdownanim &&
                                                                                       !player[k].skeleton.oldfree &&
                                                                                       !player[k].skeleton.free &&
                                                                                       player[i].skeleton.oldfree &&
                                                                                       player[i].skeleton.free &&
                                                                                       player[i].dead &&
                                                                                       player[k].lastcollide <= 0 &&
                                                                                       abs(player[i].coords.y -
                                                                                           player[k].coords.y) < .2 &&
                                                                                       findDistancefast(&player[k].coords,
                                                                                                        &player[i].coords) <
                                                                                           .7 * ((player[i].scale +
                                                                                                  player[k].scale) *
                                                                                                 2.5) *
                                                                                               ((player[i].scale +
                                                                                                 player[k].scale) *
                                                                                                2.5)) {
                                                                                        player[k].coords.y =
                                                                                            player[i].coords.y;
                                                                                        player[i].velocity =
                                                                                            player[k].velocity;
                                                                                        player[i].skeleton.free = 0;
                                                                                        player[i].rotation = 0;
                                                                                        player[i].RagDoll(0);
                                                                                        player[i].DoDamage(20);
                                                                                        if(k == 0)
                                                                                            camerashake += .3;
                                                                                        player[i].skeleton.longdead = 0;
                                                                                        player[k].lastcollide = 1;
                                                                                    }
                                                                                if(i == 0)
                                                                                    if(player[i].targetanimation ==
                                                                                           jumpdownanim &&
                                                                                       !player[i].skeleton.oldfree &&
                                                                                       !player[i].skeleton.free &&
                                                                                       player[k].skeleton.oldfree &&
                                                                                       player[k].skeleton.free &&
                                                                                       player[k].dead &&
                                                                                       player[i].lastcollide <= 0 &&
                                                                                       abs(player[i].coords.y -
                                                                                           player[k].coords.y) < .2 &&
                                                                                       findDistancefast(&player[k].coords,
                                                                                                        &player[i].coords) <
                                                                                           .7 * ((player[i].scale +
                                                                                                  player[k].scale) *
                                                                                                 2.5) *
                                                                                               ((player[i].scale +
                                                                                                 player[k].scale) *
                                                                                                2.5)) {
                                                                                        player[i].coords.y =
                                                                                            player[k].coords.y;
                                                                                        player[k].velocity =
                                                                                            player[i].velocity;
                                                                                        player[k].skeleton.free = 0;
                                                                                        player[k].rotation = 0;
                                                                                        player[k].RagDoll(0);
                                                                                        player[k].DoDamage(20);
                                                                                        if(i == 0)
                                                                                            camerashake += .3;
                                                                                        player[k].skeleton.longdead = 0;
                                                                                        player[i].lastcollide = 1;
                                                                                    }

                                                                                if((player[i].skeleton.oldfree == 1 &&
                                                                                    findLengthfast(&player[i].velocity) >
                                                                                        1) ||
                                                                                   (player[k].skeleton.oldfree == 1 &&
                                                                                    findLengthfast(&player[k].velocity) >
                                                                                        1) ||
                                                                                   (player[i].skeleton.oldfree == 0 &&
                                                                                    player[k].skeleton.oldfree == 0)) {
                                                                                    rotatetarget = player[k].velocity -
                                                                                                   player[i].velocity;
                                                                                    if(((player[i].targetanimation !=
                                                                                             getupfrombackanim &&
                                                                                         player[i].targetanimation !=
                                                                                             getupfromfrontanim) ||
                                                                                        player[i].skeleton.free) &&
                                                                                       ((player[k].targetanimation !=
                                                                                             getupfrombackanim &&
                                                                                         player[k].targetanimation !=
                                                                                             getupfromfrontanim) ||
                                                                                        player[k].skeleton.free))
                                                                                        if(((((findLengthfast(
                                                                                                   &rotatetarget) > 150 &&
                                                                                               (i != 0 && k != 0)) ||
                                                                                              (findLengthfast(
                                                                                                   &rotatetarget) > 50 &&
                                                                                               player[0]
                                                                                                   .rabbitkickragdoll /*currentanimation==rabbitkickanim*/ &&
                                                                                               (i == 0 || k == 0))) &&
                                                                                             normaldotproduct(
                                                                                                 rotatetarget,
                                                                                                 player[k].coords -
                                                                                                     player[i].coords) >
                                                                                                 0) &&
                                                                                            ((i == 0 || k == 0) ||
                                                                                             ((player[i].skeleton.oldfree ==
                                                                                                   1 &&
                                                                                               k != 0 &&
                                                                                               animation
                                                                                                       [player[k]
                                                                                                            .currentanimation]
                                                                                                           .attack ==
                                                                                                   neutral) ||
                                                                                              (player[k].skeleton.oldfree ==
                                                                                                   1 &&
                                                                                               i != 0 &&
                                                                                               animation
                                                                                                       [player[i]
                                                                                                            .currentanimation]
                                                                                                           .attack ==
                                                                                                   neutral) ||
                                                                                              (player[i].isFlip() &&
                                                                                               !player[i]
                                                                                                    .skeleton.oldfree &&
                                                                                               (i == 0 || k == 0)) ||
                                                                                              (player[k].isFlip() &&
                                                                                               !player[k]
                                                                                                    .skeleton.oldfree &&
                                                                                               (i == 0 || k == 0)) ||
                                                                                              (i == 0 || k == 0)))) ||
                                                                                           ((player[i].targetanimation ==
                                                                                                 jumpupanim ||
                                                                                             player[i].targetanimation ==
                                                                                                 jumpdownanim ||
                                                                                             player[i].isFlip()) &&
                                                                                            (player[k].targetanimation ==
                                                                                                 jumpupanim ||
                                                                                             player[k].targetanimation ==
                                                                                                 jumpdownanim ||
                                                                                             player[k].isFlip()) &&
                                                                                            (i == 0 || k == 0) &&
                                                                                            (!player[i].skeleton.oldfree &&
                                                                                             !player[k]
                                                                                                  .skeleton.oldfree))) {
                                                                                            // If hit by body
                                                                                            if(((i != 0 ||
                                                                                                player[i].skeleton.free) &&
                                                                                                   (k != 0 ||
                                                                                                    player[k]
                                                                                                        .skeleton.free)) ||
                                                                                               (animation
                                                                                                        [player[i]
                                                                                                             .targetanimation]
                                                                                                            .height ==
                                                                                                    highheight &&
                                                                                                animation
                                                                                                        [player[k]
                                                                                                             .targetanimation]
                                                                                                            .height ==
                                                                                                    highheight)) {
                                                                                                static float gLoc[3];
                                                                                                static float vel[3];
                                                                                                gLoc[0] =
                                                                                                    player[i].coords.x;
                                                                                                gLoc[1] =
                                                                                                    player[i].coords.y;
                                                                                                gLoc[2] =
                                                                                                    player[i].coords.z;
                                                                                                vel[0] =
                                                                                                    player[i].velocity.x;
                                                                                                vel[1] =
                                                                                                    player[i].velocity.y;
                                                                                                vel[2] =
                                                                                                    player[i].velocity.z;
                                                                                                if(tutoriallevel != 1) {
                                                                                                    PlaySoundEx(
                                                                                                        heavyimpactsound,
                                                                                                        samp[heavyimpactsound],
                                                                                                        NULL,
                                                                                                        true);
                                                                                                    OPENAL_3D_SetAttributes(
                                                                                                        channels
                                                                                                            [heavyimpactsound],
                                                                                                        gLoc,
                                                                                                        vel);
                                                                                                    OPENAL_SetVolume(
                                                                                                        channels
                                                                                                            [heavyimpactsound],
                                                                                                        256);
                                                                                                    OPENAL_SetPaused(
                                                                                                        channels
                                                                                                            [heavyimpactsound],
                                                                                                        false);
                                                                                                }
                                                                                                // player[i].velocity=player[k].velocity;
                                                                                                // player[k].velocity=player[i].velocity;

                                                                                                player[i].RagDoll(0);
                                                                                                if(player[i].damage >
                                                                                                       player[i]
                                                                                                               .damagetolerance -
                                                                                                           findLengthfast(
                                                                                                               &rotatetarget) /
                                                                                                               4 &&
                                                                                                   !player[i].dead) {
                                                                                                    bonus = aimbonus;
                                                                                                    bonustime = 0;
                                                                                                    bonusvalue = 150;
                                                                                                }
                                                                                                player[i].DoDamage(
                                                                                                    findLengthfast(
                                                                                                        &rotatetarget) /
                                                                                                    4);
                                                                                                player[k].RagDoll(0);
                                                                                                if(player[k].damage >
                                                                                                       player[k]
                                                                                                               .damagetolerance -
                                                                                                           findLengthfast(
                                                                                                               &rotatetarget) /
                                                                                                               4 &&
                                                                                                   !player[k].dead) {
                                                                                                    bonus = aimbonus;
                                                                                                    bonustime = 0;
                                                                                                    bonusvalue = 150;
                                                                                                }
                                                                                                player[k].DoDamage(
                                                                                                    findLengthfast(
                                                                                                        &rotatetarget) /
                                                                                                    4);

                                                                                                // if(player[i].skeleton.oldfree){
                                                                                                for(j = 0;
                                                                                                    j < player[i]
                                                                                                            .skeleton
                                                                                                            .num_joints;
                                                                                                    j++) {
                                                                                                    player[i]
                                                                                                        .skeleton.joints[j]
                                                                                                        .velocity =
                                                                                                        player[i]
                                                                                                                .skeleton
                                                                                                                .joints[j]
                                                                                                                .velocity /
                                                                                                            5 +
                                                                                                        player[k].velocity;
                                                                                                }
                                                                                                //}
                                                                                                // if(player[k].skeleton.oldfree){
                                                                                                for(j = 0;
                                                                                                    j < player[k]
                                                                                                            .skeleton
                                                                                                            .num_joints;
                                                                                                    j++) {
                                                                                                    player[k]
                                                                                                        .skeleton.joints[j]
                                                                                                        .velocity =
                                                                                                        player[k]
                                                                                                                .skeleton
                                                                                                                .joints[j]
                                                                                                                .velocity /
                                                                                                            5 +
                                                                                                        player[i].velocity;
                                                                                                }
                                                                                                //}
                                                                                            }
                                                                                        }
                                                                                    if((animation[player[i].targetanimation]
                                                                                                .attack == neutral ||
                                                                                        animation[player[i].targetanimation]
                                                                                                .attack == normalattack) &&
                                                                                       (animation[player[k].targetanimation]
                                                                                                .attack == neutral ||
                                                                                        animation[player[k].targetanimation]
                                                                                                .attack == normalattack)) {
                                                                                        // If bumped
                                                                                        if(player[i].skeleton.oldfree ==
                                                                                               0 &&
                                                                                           player[k].skeleton.oldfree ==
                                                                                               0) {
                                                                                            if(findDistancefast(
                                                                                                   &player[k].coords,
                                                                                                   &player[i].coords) <
                                                                                               .5 * ((player[i].scale +
                                                                                                      player[k].scale) *
                                                                                                     2.5) *
                                                                                                   ((player[i].scale +
                                                                                                     player[k].scale) *
                                                                                                    2.5)) {
                                                                                                rotatetarget =
                                                                                                    player[k].coords -
                                                                                                    player[i].coords;
                                                                                                Normalise(&rotatetarget);
                                                                                                player[k].coords =
                                                                                                    (player[k].coords +
                                                                                                     player[i].coords) /
                                                                                                    2;
                                                                                                player[i].coords =
                                                                                                    player[k].coords -
                                                                                                    rotatetarget *
                                                                                                        fast_sqrt(.6) / 2 *
                                                                                                        ((player[i].scale +
                                                                                                          player[k].scale) *
                                                                                                         2.5) *
                                                                                                        ((player[i].scale +
                                                                                                          player[k].scale) *
                                                                                                         2.5);
                                                                                                player[k].coords +=
                                                                                                    rotatetarget *
                                                                                                    fast_sqrt(.6) / 2 *
                                                                                                    ((player[i].scale +
                                                                                                      player[k].scale) *
                                                                                                     2.5) *
                                                                                                    ((player[i].scale +
                                                                                                      player[k].scale) *
                                                                                                     2.5);
                                                                                                if(player[k].howactive ==
                                                                                                       typeactive ||
                                                                                                   hostile)
                                                                                                    if(player[k].isIdle()) {
                                                                                                        if(player[k]
                                                                                                               .howactive <
                                                                                                           typesleeping) {
                                                                                                            player[k]
                                                                                                                .targetanimation =
                                                                                                                player[k]
                                                                                                                    .getStop();
                                                                                                            player[k]
                                                                                                                .targetframe =
                                                                                                                0;
                                                                                                            player[k]
                                                                                                                .target = 0;
                                                                                                        } else if(player[k]
                                                                                                                      .howactive ==
                                                                                                                  typesleeping) {
                                                                                                            player[k]
                                                                                                                .targetanimation =
                                                                                                                getupfromfrontanim;
                                                                                                            player[k]
                                                                                                                .targetframe =
                                                                                                                0;
                                                                                                            player[k]
                                                                                                                .target = 0;
                                                                                                        }
                                                                                                        if(!editorenabled)
                                                                                                            player[k]
                                                                                                                .howactive =
                                                                                                                typeactive;
                                                                                                    }
                                                                                                if(player[i].howactive ==
                                                                                                       typeactive ||
                                                                                                   hostile)
                                                                                                    if(player[i].isIdle()) {
                                                                                                        if(player[i]
                                                                                                               .howactive <
                                                                                                           typesleeping) {
                                                                                                            player[i]
                                                                                                                .targetanimation =
                                                                                                                player[i]
                                                                                                                    .getStop();
                                                                                                            player[i]
                                                                                                                .targetframe =
                                                                                                                0;
                                                                                                            player[i]
                                                                                                                .target = 0;
                                                                                                        } else {
                                                                                                            player[i]
                                                                                                                .targetanimation =
                                                                                                                getupfromfrontanim;
                                                                                                            player[i]
                                                                                                                .targetframe =
                                                                                                                0;
                                                                                                            player[i]
                                                                                                                .target = 0;
                                                                                                        }
                                                                                                        if(!editorenabled)
                                                                                                            player[i]
                                                                                                                .howactive =
                                                                                                                typeactive;
                                                                                                    }
                                                                                            }
                                                                                            if(hostile) {
                                                                                                if(k == 0 && i != 0 &&
                                                                                                   player[k]
                                                                                                           .targetanimation ==
                                                                                                       jumpdownanim &&
                                                                                                   !player[i].isCrouch() &&
                                                                                                   player[i]
                                                                                                           .targetanimation !=
                                                                                                       rollanim &&
                                                                                                   !player[k]
                                                                                                        .skeleton.oldfree &&
                                                                                                   !player[k]
                                                                                                        .skeleton.free &&
                                                                                                   player[k].lastcollide <=
                                                                                                       0 &&
                                                                                                   player[k].velocity.y <
                                                                                                       -10) {
                                                                                                    player[i].velocity =
                                                                                                        player[k].velocity;
                                                                                                    player[k].velocity =
                                                                                                        player[k].velocity *
                                                                                                        -.5;
                                                                                                    // player[i].velocity.y-=10;
                                                                                                    player[k].velocity.y =
                                                                                                        player[i]
                                                                                                            .velocity.y;
                                                                                                    player[i].DoDamage(20);
                                                                                                    player[i].RagDoll(0);
                                                                                                    player[k].lastcollide =
                                                                                                        1;
                                                                                                    if(k == 0) {
                                                                                                        bonus = AboveBonus;
                                                                                                        bonustime = 0;
                                                                                                        bonusvalue = 50;
                                                                                                    }
                                                                                                }
                                                                                                if(i == 0 && k != 0 &&
                                                                                                   player[i]
                                                                                                           .targetanimation ==
                                                                                                       jumpdownanim &&
                                                                                                   !player[k].isCrouch() &&
                                                                                                   player[k]
                                                                                                           .targetanimation !=
                                                                                                       rollanim &&
                                                                                                   !player[i]
                                                                                                        .skeleton.oldfree &&
                                                                                                   !player[i]
                                                                                                        .skeleton.free &&
                                                                                                   player[i].lastcollide <=
                                                                                                       0 &&
                                                                                                   player[i].velocity.y <
                                                                                                       -10) {
                                                                                                    player[k].velocity =
                                                                                                        player[i].velocity;
                                                                                                    player[i].velocity =
                                                                                                        player[i].velocity *
                                                                                                        -.3;
                                                                                                    // player[k].velocity.y-=10;
                                                                                                    player[i].velocity.y =
                                                                                                        player[k]
                                                                                                            .velocity.y;
                                                                                                    player[k].DoDamage(20);
                                                                                                    player[k].RagDoll(0);
                                                                                                    player[i].lastcollide =
                                                                                                        1;
                                                                                                    if(i == 0) {
                                                                                                        bonus = AboveBonus;
                                                                                                        bonustime = 0;
                                                                                                        bonusvalue = 50;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                                player[i].CheckKick();
                                                                                player[k].CheckKick();
                                                                            }
                                                                        }
                        }
                    }

                for(k = 0; k < numplayers; k++) {
                    for(i = k; i < numplayers; i++) {
                        if(i == k)
                            i++;
                        if(i < numplayers && i != k && player[k].skeleton.free == 0 && player[i].skeleton.oldfree == 0 &&
                           (player[i].targetanimation == jumpupanim || player[k].targetanimation == jumpupanim) &&
                           (player[i].aitype == playercontrolled || player[k].aitype == playercontrolled) &&
                           ((player[i].aitype == attacktypecutoff && player[i].stunned <= 0) ||
                            (player[k].aitype == attacktypecutoff && player[k].stunned <= 0))) {
                            if(findDistancefast(&player[i].coords, &player[k].coords) <
                                   10 * ((player[i].scale + player[k].scale) * 2.5) *
                                       ((player[i].scale + player[k].scale) * 2.5) &&
                               findDistancefastflat(&player[i].coords, &player[k].coords) <
                                   2 * ((player[i].scale + player[k].scale) * 2.5) *
                                       ((player[i].scale + player[k].scale) * 2.5)) {
                                if(player[i].targetanimation == jumpupanim &&
                                   player[k].targetanimation != getupfrombackanim &&
                                   player[k].targetanimation != getupfromfrontanim &&
                                   animation[player[k].targetanimation].height == middleheight &&
                                   normaldotproduct(player[i].velocity, player[k].coords - player[i].coords) < 0 &&
                                   ((player[k].aitype == playercontrolled && player[k].attackkeydown) ||
                                    player[k].aitype != playercontrolled)) {
                                    player[i].victim = &player[k];
                                    player[i].targetanimation = jumpreversedanim;
                                    player[i].currentanimation = jumpreversedanim;
                                    player[k].currentanimation = jumpreversalanim;
                                    player[k].targetanimation = jumpreversalanim;
                                    player[i].targettilt2 = 0;
                                    player[i].currentframe = 0;
                                    player[i].targetframe = 1;
                                    player[k].currentframe = 0;
                                    player[k].targetframe = 1;
                                    if(player[i].coords.y < player[k].coords.y + 1) {
                                        player[i].targetanimation = rabbitkickreversedanim;
                                        player[i].currentanimation = rabbitkickreversedanim;
                                        player[k].currentanimation = rabbitkickreversalanim;
                                        player[k].targetanimation = rabbitkickreversalanim;
                                        player[k].currentframe = 1;
                                        player[k].targetframe = 2;
                                        player[i].currentframe = 1;
                                        player[i].targetframe = 2;
                                    }
                                    player[k].targettilt2 = 0;
                                    player[i].target = 0;
                                    player[i].velocity = 0;
                                    player[k].velocity = 0;
                                    player[k].oldcoords = player[k].coords;
                                    player[i].coords = player[k].coords;
                                    player[k].targetrotation = player[i].targetrotation;
                                    player[k].rotation = player[i].targetrotation;
                                    player[k].victim = &player[i];
                                    if(player[k].aitype == attacktypecutoff)
                                        player[k].stunned = .5;
                                }
                                if(player[k].targetanimation == jumpupanim &&
                                   player[i].targetanimation != getupfrombackanim &&
                                   player[i].targetanimation != getupfromfrontanim &&
                                   animation[player[i].targetanimation].height == middleheight &&
                                   normaldotproduct(player[k].velocity, player[i].coords - player[k].coords) < 0 &&
                                   ((player[i].aitype == playercontrolled && player[i].attackkeydown) ||
                                    player[i].aitype != playercontrolled)) {
                                    player[k].victim = &player[i];
                                    player[k].targetanimation = jumpreversedanim;
                                    player[k].currentanimation = jumpreversedanim;
                                    player[i].currentanimation = jumpreversalanim;
                                    player[i].targetanimation = jumpreversalanim;
                                    player[k].targettilt2 = 0;
                                    player[i].targettilt2 = 0;
                                    player[k].currentframe = 0;
                                    player[k].targetframe = 1;
                                    player[i].currentframe = 0;
                                    player[i].targetframe = 1;
                                    if(player[k].coords.y < player[i].coords.y + 1) {
                                        player[k].targetanimation = rabbitkickreversedanim;
                                        player[k].currentanimation = rabbitkickreversedanim;
                                        player[i].currentanimation = rabbitkickreversalanim;
                                        player[i].targetanimation = rabbitkickreversalanim;
                                        player[k].currentframe = 1;
                                        player[k].targetframe = 2;
                                        player[i].currentframe = 1;
                                        player[i].targetframe = 2;
                                    }
                                    player[k].target = 0;
                                    player[k].velocity = 0;
                                    player[i].velocity = 0;
                                    player[i].oldcoords = player[i].coords;
                                    player[k].coords = player[i].coords;
                                    player[i].targetrotation = player[k].targetrotation;
                                    player[i].rotation = player[k].targetrotation;
                                    player[i].victim = &player[k];
                                    if(player[i].aitype == attacktypecutoff)
                                        player[i].stunned = .5;
                                }
                            }
                        }
                    }
                }

                for(k = 0; k < numplayers; k++)
                    if(player[k].immobile && k != 0)
                        player[k].coords = player[k].realoldcoords;

                // pile
                /*
                XYZ tempdiff;
                XYZ tempoldpos;
                XYZ temp1,temp2;
                bool isgood;
                static float checkdelay;
                checkdelay-=multiplier;
                int m;
                static bool checkedcoll[maxplayers][maxplayers];
                static bool above[maxplayers];

                for(i=0;i<maxplayers;i++){
                for(j=0;j<maxplayers;j++){
                checkedcoll[i][j]=0;
                }
                }

                if(numplayers>1&&checkdelay<=0){
                checkdelay=.015;
                for(k=0;k<numplayers;k++){
                if(player[k].skeleton.free!=2)above[k]=-1;

                for(i=k;i<numplayers;i++){
                if(i==k)i++;
                if(i<numplayers)
                if(!checkedcoll[i][k]){
                checkedcoll[i][k]=1;
                checkedcoll[k][i]=1;
                if(player[i].skeleton.free&&player[k].skeleton.free)
                if(player[i].skeleton.free!=2||player[k].skeleton.free!=2)
                if(i!=k&&player[i].whichpatchx==player[k].whichpatchx&&player[i].whichpatchz==player[k].whichpatchz)
                if(player[i].coords.y>player[k].coords.y-3)
                if(player[i].coords.y<player[k].coords.y+3)
                if(player[i].coords.x>player[k].coords.x-3)
                if(player[i].coords.x<player[k].coords.x+3)
                if(player[i].coords.z>player[k].coords.z-3)
                if(player[i].coords.z<player[k].coords.z+3)
                if(findDistancefast(&player[i].coords,&player[k].coords)<3*((player[i].scale+player[k].scale)*2.5)*((player[i].scale+player[k].scale)*2.5)){
                int stuck,moving;
                if((player[i].skeleton.longdead>player[k].skeleton.longdead&&player[k].skeleton.free!=2)||player[i].skeleton.free==2){
                stuck=i;
                moving=k;
                }
                else
                {
                moving=i;
                stuck=k;
                }
                isgood=1;

                if(isgood){
                above[moving]=stuck;
                for(l=0;l<player[moving].skeleton.num_joints;l++){
                for(m=0;m<player[stuck].skeleton.num_joints;m++){
                while(findDistancefast(player[moving].skeleton.joints[l].position+player[moving].coords,player[stuck].skeleton.joints[m].position+player[stuck].coords)<.25)
                {
                player[moving].skeleton.joints[l].position.y+=.003;
                if(player[moving].skeleton.joints[l].velocity.y<-.05)player[moving].skeleton.joints[l].velocity.y+=.003/.015/2;

                }
                }
                }
                }
                }
                }
                }
                }
                }

                */

                if(!IsKeyDown(SDL_SCANCODE_N)) {
                    texturesizetogglekeydown = 0;
                }

                for(k = 0; k < numplayers; k++) {
                    if(!isnormal(player[k].coords.x) || !isnormal(player[k].coords.y) || !isnormal(player[k].coords.z)) {
                        if(!isnormal(player[k].coords.x) || !isnormal(player[k].coords.y) ||
                           !isnormal(player[k].coords.z)) {
                            player[k].DoDamage(1000);
                        }
                    }
                }

                static bool respawnkeydown;
                if(!editorenabled &&
                   ((whichlevel != -2 && (IsKeyDown(SDL_SCANCODE_Z, KMOD_GUI) && config.game_debug && !editorenabled)) ||
                    (IsKeyDown(keys.jump) && !respawnkeydown && !keys.oldattack && player[0].dead))) {
                    targetlevel = whichlevel;
                    loading = 1;
                    leveltime = 5;
                }
                if(!IsKeyDown(keys.jump))
                    respawnkeydown = 0;
                if(IsKeyDown(keys.jump))
                    respawnkeydown = 1;

                if(whichlevel != -2 && IsKeyDown(SDL_SCANCODE_K, KMOD_SHIFT) && !slomotogglekeydown && config.game_debug && !editorenabled) {
                    targetlevel++;
                    if(targetlevel > numchallengelevels - 1)
                        targetlevel = 0;
                    loading = 1;
                    leveltime = 5;
                    slomotogglekeydown = 1;
                }

                /*
                if(IsKeyDown(theKeyMap, MAC_Z_KEY)){
                //Respawn
                OPENAL_SetPaused(channels[whooshsound], true);
                changedelay=0;
                for(k=0;k<numplayers;k++){
                player[k].dead=0;
                player[k].damage=0;
                player[k].permanentdamage=0;
                if(player[k].skeleton.free==2)player[k].skeleton.free=1;
                player[k].aitype=passivetype;
                }
                player[0].aitype=playercontrolled;
                }
                */

                static bool movekey;
                static bool connected;
                /*player[0].forwardkeydown=IsKeyDown(theKeyMap, MAC_W_KEY);
                player[0].leftkeydown=IsKeyDown(theKeyMap, MAC_A_KEY);
                player[0].backkeydown=IsKeyDown(theKeyMap, MAC_S_KEY);
                player[0].rightkeydown=IsKeyDown(theKeyMap, MAC_D_KEY);
                player[0].jumpkeydown=IsKeyDown(theKeyMap, MAC_SPACE_KEY);
                player[0].crouchkeydown=IsKeyDown(theKeyMap, MAC_SHIFT_KEY);*/

                // if(!player[0].crouchkeydown)player[0].crouchkeydown=IsKeyDown(theKeyMap, MAC_CONTROL_KEY);

                for(int i = 0; i < numplayers; i++) {
                    if(!player[i].skeleton.free) {
                        oldtargetrotation = player[i].targetrotation;
                        if(i == 0 && indialogue == -1) {
                            if(!animation[player[0].targetanimation].attack &&
                               player[0].targetanimation != staggerbackhighanim &&
                               player[0].targetanimation != staggerbackhardanim &&
                               player[0].targetanimation != crouchremoveknifeanim &&
                               player[0].targetanimation != removeknifeanim &&
                               player[0].targetanimation != backhandspringanim &&
                               player[0].targetanimation != dodgebackanim &&
                               player[0].targetanimation != walljumprightkickanim &&
                               player[0].targetanimation != walljumpleftkickanim) {
                                if(!cameramode)
                                    player[0].targetrotation = -rotation + 180;
                                if(cameramode)
                                    player[0].targetrotation = 0;
                            }

                            facing = 0;
                            facing.z = -1;

                            flatfacing = DoRotation(facing, 0, player[i].rotation + 180, 0);
                            if(cameramode) {
                                facing = flatfacing;
                            } else {
                                facing = DoRotation(facing, -rotation2, 0, 0);
                                facing = DoRotation(facing, 0, 0 - rotation, 0);
                            }

                            player[0].lookrotation = -rotation;

                            player[i].targetheadrotation = rotation;
                            player[i].targetheadrotation2 = rotation2;
                        }
                        if(i != 0 && player[i].aitype == playercontrolled && indialogue == -1) {
                            if(!animation[player[i].targetanimation].attack &&
                               player[i].targetanimation != staggerbackhighanim &&
                               player[i].targetanimation != staggerbackhardanim &&
                               player[i].targetanimation != crouchremoveknifeanim &&
                               player[i].targetanimation != removeknifeanim &&
                               player[i].targetanimation != backhandspringanim &&
                               player[i].targetanimation != dodgebackanim &&
                               player[i].targetanimation != walljumprightkickanim &&
                               player[i].targetanimation != walljumpleftkickanim) {
                                player[i].targetrotation = -player[i].lookrotation + 180;
                            }

                            facing = 0;
                            facing.z = -1;

                            flatfacing = DoRotation(facing, 0, player[i].rotation + 180, 0);

                            facing = DoRotation(facing, -player[i].lookrotation2, 0, 0);
                            facing = DoRotation(facing, 0, 0 - player[i].lookrotation, 0);

                            player[i].targetheadrotation = player[i].lookrotation;
                            player[i].targetheadrotation2 = player[i].lookrotation2;
                        }
                        if(indialogue != -1) {
                            rotatetarget = participantfacing[whichdialogue][indialogue][i];
                            Normalise(&rotatetarget);
                            player[i].targetheadrotation = -asin(0 - rotatetarget.x);
                            player[i].targetheadrotation *= 360 / 6.28;
                            if(rotatetarget.z < 0)
                                player[i].targetheadrotation = 180 - player[i].targetheadrotation;

                            player[i].targetheadrotation *= -1;
                            player[i].targetheadrotation += 180;
                            player[i].targetheadrotation2 = -asin(rotatetarget.y) * 360 / 6.28;
                        }

                        if(leveltime < .5)
                            numenvsounds = 0;

                        player[i].avoidsomething = 0;

                        for(j = 0; j < objects.numobjects; j++) {
                            if(objects.onfire[j]) {
                                if(findDistancefast(&objects.position[j], &player[i].coords) <
                                   objects.scale[j] * objects.scale[j] * 200) {
                                    if(findDistancefast(&player[i].coords, &objects.position[j]) <
                                       findDistancefast(&player[i].coords, &player[0].coords)) {
                                        player[i].collided = 0;
                                        player[i].avoidcollided = 1;
                                        if(player[i].avoidsomething == 0 ||
                                           findDistancefast(&objects.position[j], &player[i].coords) <
                                               findDistancefast(&player[i].coords, &player[i].avoidwhere))
                                            player[i].avoidwhere = objects.position[j];
                                    }
                                }
                            }
                        }

                        // Add avoidwhere to players

                        for(j = 0; j < numplayers; j++) {
                            if(player[j].onfire) {
                                if(findDistancefast(&player[j].coords, &player[i].coords) < 0.3 * 0.3 * 200) {
                                    if(findDistancefast(&player[i].coords, &player[j].coords) <
                                       findDistancefast(&player[i].coords, &player[0].coords)) {
                                        player[i].collided = 0;
                                        player[i].avoidcollided = 1;
                                        if(player[i].avoidsomething == 0 ||
                                           findDistancefast(&player[j].coords, &player[i].coords) <
                                               findDistancefast(&player[i].coords, &player[i].avoidwhere))
                                            player[i].avoidwhere = objects.position[j];
                                    }
                                }
                            }
                        }

                        if(player[i].collided > .8)
                            player[i].avoidcollided = 0;
                        if(player[i].aitype != playercontrolled && indialogue == -1) {
                            player[i].jumpclimb = 0;
                            // AI
                            if(editorenabled)
                                player[i].stunned = 1;

                            player[i].pause = 0;
                            // if(findDistancefastflat(&player[i].coords,&player[0].coords)<3/*&&player[0].coords.y>player[i].coords.y+.1*/)player[i].pause=1;
                            if(findDistancefastflat(&player[0].coords, &player[i].coords) < 30 &&
                               player[0].coords.y > player[i].coords.y + 2 && !player[0].onterrain)
                                player[i].pause = 1;

                            /*if(player[i].aitype==passivetype&&player[i].numwaypoints<=1){
                            player[i].forwardkeydown=0;
                            player[i].leftkeydown=0;
                            player[i].backkeydown=0;
                            player[i].rightkeydown=0;
                            player[i].crouchkeydown=0;
                            player[i].attackkeydown=0;
                            player[i].jumpkeydown=0;
                            player[i].throwkeydown=0;
                            }*/

                            if(player[i].aitype == pathfindtype) {
                                if(player[i].finalpathfindpoint == -1) {
                                    float closestdistance;
                                    float tempdist;
                                    int closest;
                                    XYZ colpoint;
                                    closest = -1;
                                    closestdistance = -1;
                                    for(j = 0; j < numpathpoints; j++) {
                                        if(closest == -1 ||
                                           findDistancefast(&player[i].finalfinaltarget, &pathpoint[j]) < closestdistance) {
                                            closestdistance = findDistancefast(&player[i].finalfinaltarget, &pathpoint[j]);
                                            closest = j;
                                            player[i].finaltarget = pathpoint[j];
                                        }
                                    }
                                    player[i].finalpathfindpoint = closest;
                                    for(j = 0; j < numpathpoints; j++) {
                                        if(numpathpointconnect[j])
                                            for(k = 0; k < numpathpointconnect[j]; k++) {
                                                DistancePointLine(&player[i].finalfinaltarget,
                                                                  &pathpoint[j],
                                                                  &pathpoint[pathpointconnect[j][k]],
                                                                  &tempdist,
                                                                  &colpoint);
                                                if(tempdist * tempdist < closestdistance) {
                                                    if(findDistance(&colpoint, &pathpoint[j]) +
                                                           findDistance(&colpoint, &pathpoint[pathpointconnect[j][k]]) <
                                                       findDistance(&pathpoint[j], &pathpoint[pathpointconnect[j][k]]) +
                                                           .1) {
                                                        closestdistance = tempdist * tempdist;
                                                        closest = j;
                                                        player[i].finaltarget = colpoint;
                                                    }
                                                }
                                            }
                                    }
                                    player[i].finalpathfindpoint = closest;
                                }
                                if(player[i].targetpathfindpoint == -1) {
                                    float closestdistance;
                                    float tempdist;
                                    int closest;
                                    XYZ colpoint;
                                    closest = -1;
                                    closestdistance = -1;
                                    if(player[i].lastpathfindpoint == -1) {
                                        for(j = 0; j < numpathpoints; j++) {
                                            if(j != player[i].lastpathfindpoint)
                                                if(closest == -1 ||
                                                   (findDistancefast(&player[i].coords, &pathpoint[j]) <
                                                    closestdistance /*&&findDistancefast(&player[i].finaltarget,&pathpoint[j])<findDistancefast(&player[i].finaltarget,&player[i].coords)*/)) {
                                                    closestdistance = findDistancefast(&player[i].coords, &pathpoint[j]);
                                                    closest = j;
                                                }
                                        }
                                        player[i].targetpathfindpoint = closest;
                                        for(j = 0; j < numpathpoints; j++) {
                                            if(j != player[i].lastpathfindpoint)
                                                if(numpathpointconnect[j])
                                                    for(k = 0; k < numpathpointconnect[j]; k++) {
                                                        DistancePointLine(&player[i].coords,
                                                                          &pathpoint[j],
                                                                          &pathpoint[pathpointconnect[j][k]],
                                                                          &tempdist,
                                                                          &colpoint);
                                                        if(tempdist * tempdist < closestdistance) {
                                                            if(findDistance(&colpoint, &pathpoint[j]) +
                                                                   findDistance(&colpoint,
                                                                                &pathpoint[pathpointconnect[j][k]]) <
                                                               findDistance(&pathpoint[j],
                                                                            &pathpoint[pathpointconnect[j][k]]) +
                                                                   .1) {
                                                                // if(findDistancefast(&player[i].finaltarget,&colpoint)<findDistancefast(&player[i].finaltarget,&player[i].coords)){
                                                                closestdistance = tempdist * tempdist;
                                                                closest = j;
                                                                //}
                                                            }
                                                        }
                                                    }
                                        }
                                        player[i].targetpathfindpoint = closest;
                                    } else {
                                        for(j = 0; j < numpathpoints; j++) {
                                            if(j != player[i].lastpathfindpoint && j != player[i].lastpathfindpoint2 &&
                                               j != player[i].lastpathfindpoint3 && j != player[i].lastpathfindpoint4) {
                                                connected = 0;
                                                if(numpathpointconnect[j])
                                                    for(k = 0; k < numpathpointconnect[j]; k++) {
                                                        if(pathpointconnect[j][k] == player[i].lastpathfindpoint)
                                                            connected = 1;
                                                    }
                                                if(!connected)
                                                    if(numpathpointconnect[player[i].lastpathfindpoint])
                                                        for(k = 0; k < numpathpointconnect[player[i].lastpathfindpoint];
                                                            k++) {
                                                            if(pathpointconnect[player[i].lastpathfindpoint][k] == j)
                                                                connected = 1;
                                                        }
                                                if(connected) {
                                                    tempdist = findPathDist(j, player[i].finalpathfindpoint);
                                                    if(closest == -1 || tempdist < closestdistance) {
                                                        closestdistance = tempdist;
                                                        closest = j;
                                                    }
                                                }
                                            }
                                        }
                                        player[i].targetpathfindpoint = closest;
                                    }
                                }
                                player[i].losupdatedelay -= multiplier;

                                rotatetarget = pathpoint[player[i].targetpathfindpoint] - player[i].coords;
                                Normalise(&rotatetarget);
                                player[i].targetrotation = -asin(0 - rotatetarget.x);
                                player[i].targetrotation *= 360 / 6.28;
                                if(rotatetarget.z < 0)
                                    player[i].targetrotation = 180 - player[i].targetrotation;
                                player[i].lookrotation = player[i].targetrotation;
                                // player[i].aiupdatedelay=.05;

                                if(findDistancefastflat(&player[i].coords, &pathpoint[player[i].targetpathfindpoint]) <
                                   .6) {
                                    player[i].lastpathfindpoint4 = player[i].lastpathfindpoint3;
                                    player[i].lastpathfindpoint3 = player[i].lastpathfindpoint2;
                                    player[i].lastpathfindpoint2 = player[i].lastpathfindpoint;
                                    player[i].lastpathfindpoint = player[i].targetpathfindpoint;
                                    if(player[i].lastpathfindpoint2 == -1)
                                        player[i].lastpathfindpoint2 = player[i].lastpathfindpoint;
                                    if(player[i].lastpathfindpoint3 == -1)
                                        player[i].lastpathfindpoint3 = player[i].lastpathfindpoint2;
                                    if(player[i].lastpathfindpoint4 == -1)
                                        player[i].lastpathfindpoint4 = player[i].lastpathfindpoint3;
                                    player[i].targetpathfindpoint = -1;
                                }
                                if(findDistancefastflat(&player[i].coords, &player[i].finalfinaltarget) <
                                       findDistancefastflat(&player[i].coords, &player[i].finaltarget) ||
                                   findDistancefastflat(&player[i].coords, &player[i].finaltarget) <
                                       .6 * (player[i].scale * 5) * (player[i].scale * 5) ||
                                   player[i].lastpathfindpoint == player[i].finalpathfindpoint) {
                                    player[i].aitype = passivetype;
                                }

                                player[i].forwardkeydown = 1;
                                player[i].leftkeydown = 0;
                                player[i].backkeydown = 0;
                                player[i].rightkeydown = 0;
                                player[i].crouchkeydown = 0;
                                player[i].attackkeydown = 0;
                                player[i].throwkeydown = 0;

                                if(player[i].avoidcollided > .8 && !player[i].jumpkeydown && player[i].collided < .8)
                                    player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);

                                if(player[i].collided < 1 || player[i].targetanimation != jumpupanim)
                                    player[i].jumpkeydown = 0;
                                if((player[i].collided > .8 && player[i].jumppower >= 5))
                                    player[i].jumpkeydown = 1;

                                if((tutoriallevel != 1 || cananger) && hostile && !player[0].dead &&
                                   findDistancefast(&player[i].coords, &player[0].coords) < 400 &&
                                   player[i].occluded < 25) {
                                    if(findDistancefast(&player[i].coords, &player[0].coords) < 12 &&
                                       animation[player[0].targetanimation].height != lowheight && !editorenabled &&
                                       (player[0].coords.y < player[i].coords.y + 5 || player[0].onterrain))
                                        player[i].aitype = attacktypecutoff;
                                    if(findDistancefast(&player[i].coords, &player[0].coords) < 30 &&
                                       animation[player[0].targetanimation].height == highheight && !editorenabled)
                                        player[i].aitype = attacktypecutoff;

                                    if(player[i].losupdatedelay < 0 && !editorenabled && player[i].occluded < 2) {
                                        player[i].losupdatedelay = .2;
                                        for(j = 0; j < numplayers; j++) {
                                            if(j == 0 || player[j].skeleton.free || player[j].aitype != passivetype) {
                                                if(abs(rand() % 2) ||
                                                   animation[player[j].targetanimation].height != lowheight || j != 0)
                                                    if(findDistancefast(&player[i].coords, &player[j].coords) < 400)
                                                        if(normaldotproduct(player[i].facing,
                                                                            player[j].coords - player[i].coords) > 0)
                                                            if(player[j].coords.y < player[i].coords.y + 5 ||
                                                               player[j].onterrain)
                                                                if((-1 ==
                                                                        checkcollide(
                                                                            DoRotation(
                                                                                player[i]
                                                                                    .skeleton
                                                                                    .joints[player[i]
                                                                                                .skeleton.jointlabels[head]]
                                                                                    .position,
                                                                                0,
                                                                                player[i].rotation,
                                                                                0) *
                                                                                    player[i].scale +
                                                                                player[i].coords,
                                                                            DoRotation(
                                                                                player[j]
                                                                                    .skeleton
                                                                                    .joints[player[j]
                                                                                                .skeleton.jointlabels[head]]
                                                                                    .position,
                                                                                0,
                                                                                player[j].rotation,
                                                                                0) *
                                                                                    player[j].scale +
                                                                                player[j].coords) &&
                                                                    !player[j].isWallJump()) ||
                                                                   (player[j].targetanimation == hanganim &&
                                                                    normaldotproduct(player[j].facing,
                                                                                     player[i].coords - player[j].coords) <
                                                                        0)) {
                                                                    player[i].aitype = searchtype;
                                                                    player[i].lastchecktime = 12;
                                                                    player[i].lastseen = player[j].coords;
                                                                    player[i].lastseentime = 12;
                                                                }
                                            }
                                        }
                                    }
                                }
                                if(player[i].aitype == attacktypecutoff && musictype != 2) {
                                    if(player[i].creature != wolftype) {
                                        player[i].stunned = .6;
                                        player[i].surprised = .6;
                                    }
                                }
                            }

                            if(player[i].aitype != passivetype && leveltime > .5) {
                                player[i].howactive = typeactive;
                            }

                            if(player[i].aitype == passivetype) {
                                player[i].aiupdatedelay -= multiplier;
                                player[i].losupdatedelay -= multiplier;
                                player[i].lastseentime += multiplier;
                                player[i].pausetime -= multiplier;
                                if(player[i].lastseentime > 1)
                                    player[i].lastseentime = 1;

                                if(player[i].aiupdatedelay < 0) {
                                    if(player[i].numwaypoints > 1 && player[i].howactive == typeactive &&
                                       player[i].pausetime <= 0) {
                                        rotatetarget = player[i].waypoints[player[i].waypoint] - player[i].coords;
                                        Normalise(&rotatetarget);
                                        player[i].targetrotation = -asin(0 - rotatetarget.x);
                                        player[i].targetrotation *= 360 / 6.28;
                                        if(rotatetarget.z < 0)
                                            player[i].targetrotation = 180 - player[i].targetrotation;
                                        player[i].lookrotation = player[i].targetrotation;
                                        player[i].aiupdatedelay = .05;

                                        if(findDistancefastflat(&player[i].coords,
                                                                &player[i].waypoints[player[i].waypoint]) < 1) {
                                            if(player[i].waypointtype[player[i].waypoint] == wppause)
                                                player[i].pausetime = 4;
                                            player[i].waypoint++;
                                            if(player[i].waypoint > player[i].numwaypoints - 1)
                                                player[i].waypoint = 0;
                                        }
                                    }

                                    if(player[i].numwaypoints > 1 && player[i].howactive == typeactive &&
                                       player[i].pausetime <= 0)
                                        player[i].forwardkeydown = 1;
                                    else
                                        player[i].forwardkeydown = 0;
                                    player[i].leftkeydown = 0;
                                    player[i].backkeydown = 0;
                                    player[i].rightkeydown = 0;
                                    player[i].crouchkeydown = 0;
                                    player[i].attackkeydown = 0;
                                    player[i].throwkeydown = 0;

                                    if(player[i].avoidcollided > .8 && !player[i].jumpkeydown && player[i].collided < .8) {
                                        if(!player[i].avoidsomething)
                                            player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);
                                        else {
                                            XYZ leftpos, rightpos;
                                            float leftdist, rightdist;
                                            leftpos = player[i].coords + DoRotation(player[i].facing, 0, 90, 0);
                                            rightpos = player[i].coords - DoRotation(player[i].facing, 0, 90, 0);
                                            leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                                            rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                                            if(leftdist < rightdist)
                                                player[i].targetrotation += 90;
                                            else
                                                player[i].targetrotation -= 90;
                                        }
                                    }
                                }
                                if(player[i].collided < 1 || player[i].targetanimation != jumpupanim)
                                    player[i].jumpkeydown = 0;
                                if((player[i].collided > .8 && player[i].jumppower >= 5))
                                    player[i].jumpkeydown = 1;

                                if(!editorenabled) {
                                    if(player[i].howactive < typesleeping)
                                        if(numenvsounds > 0 && (tutoriallevel != 1 || cananger) && hostile)
                                            for(j = 0; j < numenvsounds; j++) {
                                                if(findDistancefast(&player[i].coords, &envsound[j]) <
                                                   2 * (envsoundvol[j] +
                                                        envsoundvol[j] * (player[i].creature == rabbittype) * 3)) {
                                                    player[i].aitype = attacktypecutoff;
                                                }
                                            }

                                    if(player[i].howactive == typesleeping)
                                        if(numenvsounds > 0 && (tutoriallevel != 1 || cananger) && hostile)
                                            for(j = 0; j < numenvsounds; j++) {
                                                if(envsoundvol[j] > 14)
                                                    if(findDistancefast(&player[i].coords, &envsound[j]) <
                                                       2 * ((envsoundvol[j] - 14) +
                                                            (envsoundvol[j] - 14) * (player[i].creature == rabbittype) *
                                                                3)) {
                                                        player[i].aitype = attacktypecutoff;
                                                    }
                                            }

                                    if(player[i].aitype != passivetype) {
                                        if(player[i].howactive == typesleeping) {
                                            player[i].targetanimation = getupfromfrontanim;
                                            player[i].targetframe = 0;
                                            player[i].target = 0;
                                        }

                                        player[i].howactive = typeactive;
                                    }
                                }

                                if(player[i].howactive < typesleeping && ((tutoriallevel != 1 || cananger) && hostile) &&
                                   !player[0].dead && findDistancefast(&player[i].coords, &player[0].coords) < 400 &&
                                   player[i].occluded < 25) {
                                    if(findDistancefast(&player[i].coords, &player[0].coords) < 12 &&
                                       animation[player[0].targetanimation].height != lowheight && !editorenabled)
                                        player[i].aitype = attacktypecutoff;
                                    if(findDistancefast(&player[i].coords, &player[0].coords) < 30 &&
                                       animation[player[0].targetanimation].height == highheight && !editorenabled)
                                        player[i].aitype = attacktypecutoff;

                                    if(player[i].creature == wolftype) {
                                        XYZ windsmell;
                                        float smelldistance;
                                        smelldistance = 50;
                                        for(j = 0; j < numplayers; j++) {
                                            if(j == 0 || (player[j].dead && player[j].bloodloss > 0)) {
                                                if(j == 0 && player[j].numweapons > 0) {
                                                    if(weapons.bloody[player[j].weaponids[0]])
                                                        smelldistance = 100;
                                                    if(player[j].numweapons == 2)
                                                        if(weapons.bloody[player[j].weaponids[1]])
                                                            smelldistance = 100;
                                                }
                                                if(j != 0) {
                                                    smelldistance = 100;
                                                }
                                                windsmell = windvector;
                                                Normalise(&windsmell);
                                                windsmell = windsmell * 2 + player[j].coords;
                                                if(findDistancefast(&player[i].coords, &windsmell) < smelldistance &&
                                                   !editorenabled)
                                                    player[i].aitype = attacktypecutoff;
                                            }
                                        }
                                    }

                                    if(player[i].howactive < typesleeping && player[i].losupdatedelay < 0 &&
                                       !editorenabled && player[i].occluded < 2) {
                                        player[i].losupdatedelay = .2;
                                        for(j = 0; j < numplayers; j++) {
                                            if(j == 0 || player[j].skeleton.free || player[j].aitype != passivetype) {
                                                if(abs(rand() % 2) ||
                                                   animation[player[j].targetanimation].height != lowheight || j != 0)
                                                    if(findDistancefast(&player[i].coords, &player[j].coords) < 400)
                                                        if(normaldotproduct(player[i].facing,
                                                                            player[j].coords - player[i].coords) > 0)
                                                            if((-1 == checkcollide(
                                                                          DoRotation(
                                                                              player[i]
                                                                                  .skeleton.joints
                                                                                      [player[i].skeleton.jointlabels[head]]
                                                                                  .position,
                                                                              0,
                                                                              player[i].rotation,
                                                                              0) *
                                                                                  player[i].scale +
                                                                              player[i].coords,
                                                                          DoRotation(
                                                                              player[j]
                                                                                  .skeleton.joints
                                                                                      [player[j].skeleton.jointlabels[head]]
                                                                                  .position,
                                                                              0,
                                                                              player[j].rotation,
                                                                              0) *
                                                                                  player[j].scale +
                                                                              player[j].coords) &&
                                                                !player[j].isWallJump()) ||
                                                               (player[j].targetanimation == hanganim &&
                                                                normaldotproduct(player[j].facing,
                                                                                 player[i].coords - player[j].coords) <
                                                                    0)) {
                                                                player[i].lastseentime -= .2;
                                                                if(j == 0 &&
                                                                   animation[player[j].targetanimation].height == lowheight)
                                                                    player[i].lastseentime -= .4;
                                                                else
                                                                    player[i].lastseentime -= .6;
                                                            }
                                                if(player[i].lastseentime <= 0) {
                                                    player[i].aitype = searchtype;
                                                    player[i].lastchecktime = 12;
                                                    player[i].lastseen = player[j].coords;
                                                    player[i].lastseentime = 12;
                                                }
                                            }
                                        }
                                    }
                                }
                                if(player[i].aitype == attacktypecutoff && musictype != 2) {
                                    if(player[i].creature != wolftype) {
                                        player[i].stunned = .6;
                                        player[i].surprised = .6;
                                    }
                                    if(player[i].creature == wolftype) {
                                        player[i].stunned = .47;
                                        player[i].surprised = .47;
                                    }
                                    numseen++;
                                }
                            }

                            if(player[i].aitype == searchtype) {
                                player[i].aiupdatedelay -= multiplier;
                                player[i].losupdatedelay -= multiplier;
                                if(!player[i].pause)
                                    player[i].lastseentime -= multiplier;
                                player[i].lastchecktime -= multiplier;

                                if(player[i].isRun() && !player[i].onground) {
                                    if(player[i].coords.y >
                                       terrain.getHeight(player[i].coords.x, player[i].coords.z) + 10) {
                                        test2 = player[i].coords + player[i].facing;
                                        test2.y += 5;
                                        test = player[i].coords + player[i].facing;
                                        test.y -= 10;
                                        j = checkcollide(test2, test, player[i].laststanding);
                                        if(j == -1)
                                            j = checkcollide(test2, test);
                                        if(j == -1) {
                                            player[i].velocity = 0;
                                            player[i].targetanimation = player[i].getStop();
                                            player[i].targetframe = 0;
                                            player[i].target = 0;
                                            player[i].targetrotation += 180;
                                            player[i].stunned = .5;
                                            // player[i].aitype=passivetype;
                                            player[i].aitype = pathfindtype;
                                            player[i].finalfinaltarget = player[i].waypoints[player[i].waypoint];
                                            player[i].finalpathfindpoint = -1;
                                            player[i].targetpathfindpoint = -1;
                                            player[i].lastpathfindpoint = -1;
                                            player[i].lastpathfindpoint2 = -1;
                                            player[i].lastpathfindpoint3 = -1;
                                            player[i].lastpathfindpoint4 = -1;
                                        } else
                                            player[i].laststanding = j;
                                    }
                                }
                                if(player[i].aiupdatedelay < 0) {
                                    rotatetarget = player[i].lastseen - player[i].coords;
                                    Normalise(&rotatetarget);
                                    player[i].targetrotation = -asin(0 - rotatetarget.x);
                                    player[i].targetrotation *= 360 / 6.28;
                                    if(rotatetarget.z < 0)
                                        player[i].targetrotation = 180 - player[i].targetrotation;
                                    player[i].lookrotation = player[i].targetrotation;
                                    player[i].aiupdatedelay = .05;
                                    player[i].forwardkeydown = 1;

                                    if(findDistancefastflat(&player[i].coords, &player[i].lastseen) <
                                           1 * (player[i].scale * 5) * (player[i].scale * 5) ||
                                       player[i].lastchecktime < 0) {
                                        player[i].forwardkeydown = 0;
                                        player[i].aiupdatedelay = 1;
                                        player[i].lastseen.x += (float(rand() % 100) - 50) / 25;
                                        player[i].lastseen.z += (float(rand() % 100) - 50) / 25;
                                        player[i].lastchecktime = 3;
                                    }

                                    player[i].leftkeydown = 0;
                                    player[i].backkeydown = 0;
                                    player[i].rightkeydown = 0;
                                    player[i].crouchkeydown = 0;
                                    player[i].attackkeydown = 0;
                                    player[i].throwkeydown = 0;

                                    if(player[i].avoidcollided > .8 && !player[i].jumpkeydown && player[i].collided < .8) {
                                        if(!player[i].avoidsomething)
                                            player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);
                                        else {
                                            XYZ leftpos, rightpos;
                                            float leftdist, rightdist;
                                            leftpos = player[i].coords + DoRotation(player[i].facing, 0, 90, 0);
                                            rightpos = player[i].coords - DoRotation(player[i].facing, 0, 90, 0);
                                            leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                                            rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                                            if(leftdist < rightdist)
                                                player[i].targetrotation += 90;
                                            else
                                                player[i].targetrotation -= 90;
                                        }
                                    }
                                }
                                if(player[i].collided < 1 || player[i].targetanimation != jumpupanim)
                                    player[i].jumpkeydown = 0;
                                if((player[i].collided > .8 && player[i].jumppower >= 5))
                                    player[i].jumpkeydown = 1;

                                if(numenvsounds > 0 && ((tutoriallevel != 1 || cananger) && hostile))
                                    for(j = 0; j < numenvsounds; j++) {
                                        if(findDistancefast(&player[i].coords, &envsound[j]) <
                                           2 * (envsoundvol[j] + envsoundvol[j] * (player[i].creature == rabbittype) * 3)) {
                                            player[i].aitype = attacktypecutoff;
                                        }
                                    }

                                if(!player[0].dead && player[i].losupdatedelay < 0 && !editorenabled &&
                                   player[i].occluded < 2 && ((tutoriallevel != 1 || cananger) && hostile)) {
                                    player[i].losupdatedelay = .2;
                                    if(findDistancefast(&player[i].coords, &player[0].coords) < 4 &&
                                       animation[player[i].targetanimation].height != lowheight) {
                                        player[i].aitype = attacktypecutoff;
                                        player[i].lastseentime = 1;
                                    }
                                    if(abs(rand() % 2) || animation[player[i].targetanimation].height != lowheight)
                                        if(findDistancefast(&player[i].coords, &player[0].coords) < 400)
                                            if(normaldotproduct(player[i].facing, player[0].coords - player[i].coords) > 0)
                                                if((-1 == checkcollide(
                                                              DoRotation(
                                                                  player[i]
                                                                      .skeleton.joints[player[i].skeleton.jointlabels[head]]
                                                                      .position,
                                                                  0,
                                                                  player[i].rotation,
                                                                  0) *
                                                                      player[i].scale +
                                                                  player[i].coords,
                                                              DoRotation(
                                                                  player[0]
                                                                      .skeleton.joints[player[0].skeleton.jointlabels[head]]
                                                                      .position,
                                                                  0,
                                                                  player[0].rotation,
                                                                  0) *
                                                                      player[0].scale +
                                                                  player[0].coords)) ||
                                                   (player[j].targetanimation == hanganim &&
                                                    normaldotproduct(player[j].facing,
                                                                     player[i].coords - player[j].coords) < 0)) {
                                                    player[i].aitype = attacktypecutoff;
                                                    player[i].lastseentime = 1;
                                                }
                                }
                                if(player[i].lastseentime < 0) {
                                    // player[i].aitype=passivetype;
                                    numescaped++;
                                    player[i].aitype = pathfindtype;
                                    player[i].finalfinaltarget = player[i].waypoints[player[i].waypoint];
                                    player[i].finalpathfindpoint = -1;
                                    player[i].targetpathfindpoint = -1;
                                    player[i].lastpathfindpoint = -1;
                                    player[i].lastpathfindpoint2 = -1;
                                    player[i].lastpathfindpoint3 = -1;
                                    player[i].lastpathfindpoint4 = -1;
                                }
                            }

                            if(player[i].aitype != gethelptype) {
                                player[i].runninghowlong = 0;
                            }

                            if(player[i].aitype == gethelptype) {
                                player[i].runninghowlong += multiplier;
                                player[i].aiupdatedelay -= multiplier;

                                if(player[i].aiupdatedelay < 0 || player[i].ally == 0) {
                                    player[i].aiupdatedelay = .2;

                                    int closest;
                                    float closestdist;
                                    closest = -1;
                                    closestdist = -1;
                                    float distance;

                                    if(!player[i].ally) {
                                        for(j = 0; j < numplayers; j++) {
                                            if(j != i && j != 0 && !player[j].dead && player[j].howactive < typedead1 &&
                                               !player[j].skeleton.free && player[j].aitype == passivetype) {
                                                distance = findDistancefast(&player[i].coords, &player[j].coords);
                                                if(closestdist == -1 || distance < closestdist) {
                                                    closestdist = distance;
                                                    closest = j;
                                                }
                                                closest = j;
                                            }
                                        }
                                        if(closest != -1)
                                            player[i].ally = closest;
                                        else
                                            player[i].ally = 0;
                                        player[i].lastseen = player[0].coords;
                                        player[i].lastseentime = 12;
                                    }

                                    player[i].lastchecktime = 12;
                                    // player[i].lastseentime-=.5;

                                    facing = player[i].coords;
                                    flatfacing = player[player[i].ally].coords;
                                    facing.y += player[i].skeleton.joints[player[i].skeleton.jointlabels[head]].position.y *
                                                player[i].scale;
                                    flatfacing.y += player[player[i].ally]
                                                        .skeleton.joints[player[player[i].ally].skeleton.jointlabels[head]]
                                                        .position.y *
                                                    player[player[i].ally].scale;
                                    if(-1 != checkcollide(facing, flatfacing))
                                        player[i].lastseentime -= .1;

                                    if(player[i].ally <= 0 || player[player[i].ally].skeleton.free ||
                                       player[player[i].ally].aitype != passivetype || player[i].lastseentime <= 0) {
                                        player[i].aitype = searchtype;
                                        player[i].lastseentime = 12;
                                    }

                                    if(player[i].ally > 0) {
                                        rotatetarget = player[player[i].ally].coords - player[i].coords;
                                        Normalise(&rotatetarget);
                                        player[i].targetrotation = -asin(0 - rotatetarget.x);
                                        player[i].targetrotation *= 360 / 6.28;
                                        if(rotatetarget.z < 0)
                                            player[i].targetrotation = 180 - player[i].targetrotation;
                                        player[i].lookrotation = player[i].targetrotation;
                                        player[i].aiupdatedelay = .05;
                                        player[i].forwardkeydown = 1;

                                        if(findDistancefastflat(&player[i].coords, &player[player[i].ally].coords) < 3) {
                                            player[i].aitype = searchtype;
                                            player[i].lastseentime = 12;
                                            player[player[i].ally].aitype = searchtype;
                                            if(player[player[i].ally].lastseentime < player[i].lastseentime) {
                                                player[player[i].ally].lastseen = player[i].lastseen;
                                                player[player[i].ally].lastseentime = player[i].lastseentime;
                                                player[player[i].ally].lastchecktime = player[i].lastchecktime;
                                            }
                                        }

                                        if(player[i].avoidcollided > .8 && !player[i].jumpkeydown &&
                                           player[i].collided < .8) {
                                            if(!player[i].avoidsomething)
                                                player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);
                                            else {
                                                XYZ leftpos, rightpos;
                                                float leftdist, rightdist;
                                                leftpos = player[i].coords + DoRotation(player[i].facing, 0, 90, 0);
                                                rightpos = player[i].coords - DoRotation(player[i].facing, 0, 90, 0);
                                                leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                                                rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                                                if(leftdist < rightdist)
                                                    player[i].targetrotation += 90;
                                                else
                                                    player[i].targetrotation -= 90;
                                            }
                                        }
                                    }

                                    player[i].leftkeydown = 0;
                                    player[i].backkeydown = 0;
                                    player[i].rightkeydown = 0;
                                    player[i].crouchkeydown = 0;
                                    player[i].attackkeydown = 0;
                                }
                                if(player[i].collided < 1 || player[i].targetanimation != jumpupanim)
                                    player[i].jumpkeydown = 0;
                                if((player[i].collided > .8 && player[i].jumppower >= 5))
                                    player[i].jumpkeydown = 1;
                            }

                            if(player[i].aitype == getweapontype) {
                                player[i].aiupdatedelay -= multiplier;
                                player[i].lastchecktime -= multiplier;

                                if(player[i].aiupdatedelay < 0) {
                                    player[i].aiupdatedelay = .2;

                                    int closest;
                                    float closestdist;
                                    closest = -1;
                                    closestdist = -1;
                                    float distance;

                                    if(player[i].ally < 0) {
                                        for(j = 0; j < weapons.numweapons; j++) {
                                            if(weapons.owner[j] == -1) {
                                                distance = findDistancefast(&player[i].coords, &weapons.position[j]);
                                                if(closestdist == -1 || distance < closestdist) {
                                                    closestdist = distance;
                                                    closest = j;
                                                }
                                                closest = j;
                                            }
                                        }
                                        if(closest != -1)
                                            player[i].ally = closest;
                                        else
                                            player[i].ally = -1;
                                    }

                                    player[i].lastseentime = 12;

                                    if(!player[0].dead && ((tutoriallevel != 1 || cananger) && hostile))
                                        if(player[i].ally < 0 || player[i].weaponactive != -1 ||
                                           player[i].lastchecktime <= 0) {
                                            player[i].aitype = attacktypecutoff;
                                            player[i].lastseentime = 1;
                                        }
                                    if(!player[0].dead)
                                        if(player[i].ally >= 0) {
                                            if(weapons.owner[player[i].ally] != -1 ||
                                               findDistancefast(&player[i].coords, &weapons.position[player[i].ally]) >
                                                   16) {
                                                player[i].aitype = attacktypecutoff;
                                                player[i].lastseentime = 1;
                                            }
                                            rotatetarget = weapons.position[player[i].ally] - player[i].coords;
                                            Normalise(&rotatetarget);
                                            player[i].targetrotation = -asin(0 - rotatetarget.x);
                                            player[i].targetrotation *= 360 / 6.28;
                                            if(rotatetarget.z < 0)
                                                player[i].targetrotation = 180 - player[i].targetrotation;
                                            player[i].lookrotation = player[i].targetrotation;
                                            player[i].aiupdatedelay = .05;
                                            player[i].forwardkeydown = 1;

                                            if(player[i].avoidcollided > .8 && !player[i].jumpkeydown &&
                                               player[i].collided < .8) {
                                                if(!player[i].avoidsomething)
                                                    player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);
                                                else {
                                                    XYZ leftpos, rightpos;
                                                    float leftdist, rightdist;
                                                    leftpos = player[i].coords + DoRotation(player[i].facing, 0, 90, 0);
                                                    rightpos = player[i].coords - DoRotation(player[i].facing, 0, 90, 0);
                                                    leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                                                    rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                                                    if(leftdist < rightdist)
                                                        player[i].targetrotation += 90;
                                                    else
                                                        player[i].targetrotation -= 90;
                                                }
                                            }
                                            /*if(findDistancefast(&player[i].coords,&weapons.position[player[i].ally])<3){
                                            if(abs(rand()%6)){
                                            player[i].crouchkeydown=1;
                                            if(!findDistancefast(&player[i].coords,&weapons.position[player[i].ally])<1){
                                            if(player[i].isRun()){
                                            player[i].targetframe=0;
                                            player[i].target=0;
                                            player[i].targetanimation=sneakanim;
                                            }
                                            }
                                            else player[i].forwardkeydown=0;
                                            }
                                            else player[i].crouchkeydown=0;
                                            }
                                            else player[i].crouchkeydown=0;*/
                                        }

                                    player[i].leftkeydown = 0;
                                    player[i].backkeydown = 0;
                                    player[i].rightkeydown = 0;
                                    player[i].attackkeydown = 0;
                                    player[i].throwkeydown = 1;
                                    player[i].crouchkeydown = 0;
                                    if(player[i].targetanimation != crouchremoveknifeanim &&
                                       player[i].targetanimation != removeknifeanim)
                                        player[i].throwtogglekeydown = 0;
                                    player[i].drawkeydown = 0;
                                }
                                if(player[i].collided < 1 || player[i].targetanimation != jumpupanim)
                                    player[i].jumpkeydown = 0;
                                if((player[i].collided > .8 && player[i].jumppower >= 5))
                                    player[i].jumpkeydown = 1;
                            }

                            if(player[i].aitype == attacktypecutoff) {
                                player[i].aiupdatedelay -= multiplier;
                                if(player[i].damage < player[i].damagetolerance * 2 / 3)
                                    if((player[0].targetanimation == rabbitkickanim ||
                                        player[0].targetanimation == knifethrowanim ||
                                        (player[0].isFlip() &&
                                         normaldotproduct(player[0].facing, player[0].coords - player[i].coords) < 0)) &&
                                       !player[0].skeleton.free && (player[i].aiupdatedelay < .1)) {
                                        player[i].attackkeydown = 0;
                                        if(player[i].isIdle())
                                            player[i].crouchkeydown = 1;
                                        if(player[0].targetanimation != rabbitkickanim && player[0].weaponactive != -1) {
                                            if(weapons.type[player[0].weaponids[0]] == knife) {
                                                if(player[i].isIdle() || player[i].isCrouch() || player[i].isRun() ||
                                                   player[i].isFlip()) {
                                                    if(abs(rand() % 2 == 0))
                                                        player[i].targetanimation = backhandspringanim;
                                                    else
                                                        player[i].targetanimation = rollanim;
                                                    player[i].target = 0;
                                                    player[i].targetframe = 0;
                                                    player[i].targetrotation += 90 * (abs(rand() % 2) * 2 - 1);
                                                    player[i].wentforweapon = 0;
                                                }
                                                if(player[i].targetanimation == jumpupanim ||
                                                   player[i].targetanimation == jumpdownanim) {
                                                    player[i].targetanimation = flipanim;
                                                    player[i].target = 0;
                                                    player[i].targetframe = 0;
                                                }
                                            }
                                        }
                                        player[i].forwardkeydown = 0;
                                        player[i].aiupdatedelay = .02;
                                    }
                                if(player[0].isFlip() && !player[0].skeleton.free &&
                                   player[0].targetanimation != walljumprightkickanim &&
                                   player[0].targetanimation != walljumpleftkickanim) {
                                    if(findDistancefast(&player[0].coords, &player[i].coords) < 25)
                                        if((1 - player[i].damage / player[i].damagetolerance) > .5)
                                            player[i].stunned = 1;
                                }
                                if(player[i].wentforweapon < 3)
                                    for(j = 0; j < weapons.numweapons; j++) {
                                        if(player[i].creature != wolftype)
                                            if(player[i].numweapons == 0 && weapons.owner[j] == -1 &&
                                               weapons.velocity[i].x == 0 && weapons.velocity[i].z == 0 &&
                                               weapons.velocity[i].y == 0) {
                                                if(findDistancefast(&player[i].coords, &weapons.position[j]) < 16) {
                                                    player[i].wentforweapon++;
                                                    player[i].lastchecktime = 6;
                                                    player[i].aitype = getweapontype;
                                                    player[i].ally = -1;
                                                }
                                            }
                                    }
                                if(player[i].damage < player[i].damagetolerance / 2)
                                    if(animation[player[i].targetanimation].height != highheight)
                                        if(player[i].damage < player[i].damagetolerance * .5 &&
                                           ((player[0].targetanimation == walljumprightkickanim ||
                                             player[0].targetanimation == walljumpleftkickanim) &&
                                            ((player[i].aiupdatedelay < .15 && config.game_difficulty == 2) ||
                                             (player[i].aiupdatedelay < .08 && config.game_difficulty != 2)))) {
                                            player[i].crouchkeydown = 1;
                                        }
                                if(player[i].isRun() && !player[i].onground) {
                                    if(player[i].coords.y >
                                       terrain.getHeight(player[i].coords.x, player[i].coords.z) + 10) {
                                        test2 = player[i].coords + player[i].facing;
                                        test2.y += 5;
                                        test = player[i].coords + player[i].facing;
                                        test.y -= 10;
                                        j = checkcollide(test2, test, player[i].laststanding);
                                        if(j == -1)
                                            j = checkcollide(test2, test);
                                        if(j == -1) {
                                            player[i].velocity = 0;
                                            player[i].targetanimation = player[i].getStop();
                                            player[i].targetframe = 0;
                                            player[i].target = 0;
                                            player[i].targetrotation += 180;
                                            player[i].stunned = .5;
                                            // player[i].aitype=passivetype;
                                            player[i].aitype = pathfindtype;
                                            player[i].finalfinaltarget = player[i].waypoints[player[i].waypoint];
                                            player[i].finalpathfindpoint = -1;
                                            player[i].targetpathfindpoint = -1;
                                            player[i].lastpathfindpoint = -1;
                                            player[i].lastpathfindpoint2 = -1;
                                            player[i].lastpathfindpoint3 = -1;
                                            player[i].lastpathfindpoint4 = -1;
                                        } else
                                            player[i].laststanding = j;
                                    }
                                }
                                if(player[0].coords.y > player[i].coords.y + 5 &&
                                   animation[player[0].targetanimation].height != highheight && !player[0].onterrain) {
                                    player[i].aitype = pathfindtype;
                                    player[i].finalfinaltarget = player[i].waypoints[player[i].waypoint];
                                    player[i].finalpathfindpoint = -1;
                                    player[i].targetpathfindpoint = -1;
                                    player[i].lastpathfindpoint = -1;
                                    player[i].lastpathfindpoint2 = -1;
                                    player[i].lastpathfindpoint3 = -1;
                                    player[i].lastpathfindpoint4 = -1;
                                }
                                if(player[i].aiupdatedelay < 0 && !animation[player[i].targetanimation].attack &&
                                   player[i].targetanimation != staggerbackhighanim &&
                                   player[i].targetanimation != staggerbackhardanim &&
                                   player[i].targetanimation != backhandspringanim &&
                                   player[i].targetanimation != dodgebackanim) {
                                    if(player[i].weaponactive == -1 && player[i].numweapons > 0)
                                        player[i].drawkeydown = rand() % 2;
                                    else
                                        player[i].drawkeydown = 0;
                                    player[i].rabbitkickenabled = rand() % 2;
                                    rotatetarget = player[player[i].aitarget].coords + player[player[i].aitarget].velocity;
                                    if(findDistancefast(&player[player[i].aitarget].coords, &player[i].coords) <
                                       findDistancefast(&rotatetarget, &player[i].coords))
                                        rotatetarget =
                                            player[player[i].aitarget].coords +
                                            player[player[i].aitarget].velocity *
                                                findDistance(&player[player[i].aitarget].coords, &player[i].coords) /
                                                findLength(&player[i].velocity) -
                                            player[i].coords;
                                    else
                                        rotatetarget = player[player[i].aitarget].coords - player[i].coords;
                                    Normalise(&rotatetarget);
                                    player[i].targetrotation = -asin(0 - rotatetarget.x);
                                    player[i].targetrotation *= 360 / 6.28;
                                    if(rotatetarget.z < 0)
                                        player[i].targetrotation = 180 - player[i].targetrotation;
                                    player[i].lookrotation = player[i].targetrotation;
                                    player[i].aiupdatedelay = .2 + abs((float)(rand() % 100) / 1000);

                                    oldkey = player[i].forwardkeydown;
                                    if(findDistancefast(&player[i].coords, &player[0].coords) > 5 &&
                                       (player[0].weaponactive == -1 || player[i].weaponactive != -1))
                                        player[i].forwardkeydown = 1;
                                    else if((findDistancefast(&player[i].coords, &player[0].coords) > 16 ||
                                             findDistancefast(&player[i].coords, &player[0].coords) < 9) &&
                                            player[0].weaponactive != -1)
                                        player[i].forwardkeydown = 1;
                                    else if(rand() % 6 == 0 || (player[i].creature == wolftype && rand() % 3 == 0))
                                        player[i].forwardkeydown = 1;
                                    else
                                        player[i].forwardkeydown = 0;
                                    if(player[0].dead) {
                                        player[i].forwardkeydown = 0;
                                        if(rand() % 10 == 0)
                                            player[i].forwardkeydown = 1;
                                        if(rand() % 100 == 0) {
                                            player[i].aitype = pathfindtype;
                                            player[i].finalfinaltarget = player[i].waypoints[player[i].waypoint];
                                            player[i].finalpathfindpoint = -1;
                                            player[i].targetpathfindpoint = -1;
                                            player[i].lastpathfindpoint = -1;
                                            player[i].lastpathfindpoint2 = -1;
                                            player[i].lastpathfindpoint3 = -1;
                                            player[i].lastpathfindpoint4 = -1;
                                        }
                                    }
                                    player[i].leftkeydown = 0;
                                    player[i].backkeydown = 0;
                                    player[i].rightkeydown = 0;
                                    player[i].crouchkeydown = 0;
                                    player[i].throwkeydown = 0;

                                    if(player[i].avoidcollided > .8 && !player[i].jumpkeydown && player[i].collided < .8)
                                        player[i].targetrotation += 90 * (player[i].whichdirection * 2 - 1);
                                    /*for(j=0;j<numplayers;j++){
                                    if(player[j].victim->id==i&&(player[j].targetanimation==spinkickanim&&player[j].targetframe<3)){
                                    player[i].crouchkeydown=1;
                                    }
                                    }*/
                                    if(rand() % 2 == 0 /*||player[0].weaponactive!=-1*/ || player[i].weaponactive != -1 ||
                                       player[i].creature == wolftype)
                                        player[i].attackkeydown = 1;
                                    else
                                        player[i].attackkeydown = 0;
                                    if((player[i].isRun()) && rand() % 6 &&
                                       findDistancefast(&player[i].coords, &player[0].coords) > 7)
                                        player[i].attackkeydown = 0;
                                    // if(player[i].attackkeydown&&findDistancefast(&player[i].coords,&player[0].coords)<3&&player[i].targetanimation!=runanim&&!player[0].skeleton.free)player[i].crouchkeydown=1;
                                    /*if(player[0].targetanimation==rabbitkickanim&&!player[0].skeleton.free){
                                    player[i].attackkeydown=0;
                                    if(player[i].isIdle())player[i].crouchkeydown=1;
                                    player[i].forwardkeydown=0;
                                    player[i].aiupdatedelay=.02;
                                    }*/

                                    if(player[i].aitype != playercontrolled &&
                                       (player[i].isIdle() || player[i].isCrouch() || player[i].isRun())) {
                                        target = -2;
                                        for(j = 0; j < numplayers; j++) {
                                            if(j != i && !player[j].skeleton.free && player[j].hasvictim &&
                                               ((tutoriallevel == 1 && reversaltrain) ||
                                                (rand() % 2 == 0 && config.game_difficulty == 2) ||
                                                (rand() % 4 == 0 && config.game_difficulty == 1) ||
                                                (rand() % 8 == 0 && config.game_difficulty == 0) ||
                                                (player[j].lastattack2 == player[j].targetanimation &&
                                                 player[j].lastattack3 == player[j].targetanimation &&
                                                 (rand() % 2 == 0 || config.game_difficulty == 2)) ||
                                                ((player[i].isIdle() || player[i].isRun()) &&
                                                 player[j].weaponactive != -1) ||
                                                (player[j].targetanimation == swordslashanim &&
                                                 player[i].weaponactive != -1) ||
                                                player[j].targetanimation == staffhitanim ||
                                                player[j].targetanimation == staffspinhitanim)) {
                                                if(findDistancefast(&player[j].coords, &player[j].victim->coords) < 4 &&
                                                   player[j].victim == &player[i] &&
                                                   (player[j].targetanimation == sweepanim ||
                                                    player[j].targetanimation == spinkickanim ||
                                                    player[j].targetanimation == staffhitanim ||
                                                    player[j].targetanimation == staffspinhitanim ||
                                                    player[j].targetanimation == winduppunchanim ||
                                                    player[j].targetanimation == upunchanim ||
                                                    player[j].targetanimation == wolfslapanim ||
                                                    player[j].targetanimation == knifeslashstartanim ||
                                                    ((player[j].targetanimation == swordslashanim) &&
                                                     (findDistancefast(&player[j].coords, &player[i].coords) < 2 ||
                                                      (player[i].weaponactive != -1))))) {
                                                    if(target >= 0)
                                                        target = -1;
                                                    else
                                                        target = j;
                                                }
                                            }
                                        }
                                        if(target >= 0)
                                            player[target].Reverse();
                                    }

                                    if(player[i].collided < 1)
                                        player[i].jumpkeydown = 0;
                                    if((player[i].collided > .8 && player[i].jumppower >= 5) ||
                                       (findDistancefast(&player[i].coords, &player[0].coords) > 400 &&
                                        player[i].onterrain && player[i].creature == rabbittype))
                                        player[i].jumpkeydown = 1;
                                    if(normaldotproduct(player[i].facing, player[0].coords - player[i].coords) > 0)
                                        player[0].jumpkeydown = 0;
                                    if(player[0].targetanimation == jumpdownanim &&
                                       findDistancefast(&player[0].coords, &player[i].coords) < 40)
                                        player[i].crouchkeydown = 1;
                                    if(player[i].jumpkeydown)
                                        player[i].attackkeydown = 0;
                                    // if(animation[player[i].targetanimation].attack==reversed)player[i].crouchkeydown=1;

                                    if(tutoriallevel == 1) {
                                        if(!canattack)
                                            player[i].attackkeydown = 0;
                                    }

                                    facing = player[i].coords;
                                    flatfacing = player[0].coords;
                                    facing.y += player[i].skeleton.joints[player[i].skeleton.jointlabels[head]].position.y *
                                                player[i].scale;
                                    flatfacing.y +=
                                        player[0].skeleton.joints[player[0].skeleton.jointlabels[head]].position.y *
                                        player[0].scale;
                                    if(player[i].occluded >= 2) {
                                        if(-1 != checkcollide(facing, flatfacing)) {
                                            if(!player[i].pause)
                                                player[i].lastseentime -= .2;
                                            if(player[i].lastseentime <= 0 &&
                                               (player[i].creature != wolftype || player[i].weaponstuck == -1)) {
                                                player[i].aitype = searchtype;
                                                player[i].lastchecktime = 12;
                                                player[i].lastseen = player[0].coords;
                                                player[i].lastseentime = 12;
                                            }
                                        } else
                                            player[i].lastseentime = 1;
                                    }
                                }
                            }
                            if(animation[player[0].targetanimation].height == highheight &&
                               (player[i].aitype == attacktypecutoff || player[i].aitype == searchtype)) {
                                if(player[0].coords.y > terrain.getHeight(player[0].coords.x, player[0].coords.z) + 10) {
                                    test = player[0].coords;
                                    test.y -= 40;
                                    if(-1 == checkcollide(player[0].coords, test))
                                        player[i].stunned = 1;
                                }
                            }
                            // NOTE: Ask about logic of this call : NOTE
                            if((player[i].aitype == passivetype && !(player[i].numwaypoints > 1)) ||
                               player[i].stunned > 0 ||
                               (player[i].pause && (player[i].damage > player[i].superpermanentdamage))) {
                                if(/*player[i].aitype==attacktypecutoff&&*/ player[i].pause)
                                    player[i].lastseentime = 1;
                                player[i].targetrotation = player[i].rotation;
                                player[i].forwardkeydown = 0;
                                player[i].leftkeydown = 0;
                                player[i].backkeydown = 0;
                                player[i].rightkeydown = 0;
                                player[i].jumpkeydown = 0;
                                player[i].attackkeydown = 0;
                                player[i].crouchkeydown = 0;
                                player[i].throwkeydown = 0;
                            }

                            facing = 0;
                            facing.z = -1;

                            flatfacing = DoRotation(facing, 0, player[i].rotation + 180, 0);
                            facing = flatfacing;

                            if(player[i].aitype == attacktypecutoff) {
                                rotatetarget = player[0].coords - player[i].coords;
                                Normalise(&rotatetarget);
                                player[i].targetheadrotation = -asin(0 - rotatetarget.x);
                                player[i].targetheadrotation *= 360 / 6.28;
                                if(rotatetarget.z < 0)
                                    player[i].targetheadrotation = 180 - player[i].targetheadrotation;

                                player[i].targetheadrotation *= -1;
                                player[i].targetheadrotation += 180;
                                // player[i].targetheadrotation2=0;
                                player[i].targetheadrotation2 = -asin(rotatetarget.y) * 360 / 6.28;
                            } else if(player[i].howactive >= typesleeping) {
                                player[i].targetheadrotation = player[i].targetrotation;
                                player[i].targetheadrotation2 = 0;
                            } else {
                                if(player[i].interestdelay <= 0) {
                                    player[i].interestdelay = .7 + (float)(abs(rand() % 100)) / 100;
                                    player[i].headtarget = player[i].coords;
                                    player[i].headtarget.x += (float)(abs(rand() % 200) - 100) / 100;
                                    player[i].headtarget.z += (float)(abs(rand() % 200) - 100) / 100;
                                    player[i].headtarget.y += (float)(abs(rand() % 200) - 100) / 300;
                                    player[i].headtarget += player[i].facing * 1.5;
                                }
                                rotatetarget = player[i].headtarget - player[i].coords;
                                Normalise(&rotatetarget);
                                player[i].targetheadrotation = -asin(0 - rotatetarget.x);
                                player[i].targetheadrotation *= 360 / 6.28;
                                if(rotatetarget.z < 0)
                                    player[i].targetheadrotation = 180 - player[i].targetheadrotation;

                                player[i].targetheadrotation *= -1;
                                player[i].targetheadrotation += 180;
                                player[i].targetheadrotation2 = -asin(rotatetarget.y) * 360 / 6.28;
                            }
                            // if(whichlevel==2)player[i].jumpkeydown=0;
                        }
                        if(animation[player[i].targetanimation].attack == reversed) {
                            // player[i].targetrotation=player[i].rotation;
                            player[i].forwardkeydown = 0;
                            player[i].leftkeydown = 0;
                            player[i].backkeydown = 0;
                            player[i].rightkeydown = 0;
                            player[i].jumpkeydown = 0;
                            player[i].attackkeydown = 0;
                            // player[i].crouchkeydown=0;
                            player[i].throwkeydown = 0;
                        }

                        if(indialogue != -1) {
                            player[i].forwardkeydown = 0;
                            player[i].leftkeydown = 0;
                            player[i].backkeydown = 0;
                            player[i].rightkeydown = 0;
                            player[i].jumpkeydown = 0;
                            player[i].crouchkeydown = 0;
                            player[i].drawkeydown = 0;
                            player[i].throwkeydown = 0;
                        }

                        if(player[i].collided < -.3)
                            player[i].collided = -.3;
                        if(player[i].collided > 1)
                            player[i].collided = 1;
                        player[i].collided -= multiplier * 4;
                        player[i].whichdirectiondelay -= multiplier;
                        if(player[i].avoidcollided < -.3 || player[i].whichdirectiondelay <= 0) {
                            player[i].avoidcollided = -.3;
                            player[i].whichdirection = abs(rand() % 2);
                            player[i].whichdirectiondelay = .4;
                        }
                        if(player[i].avoidcollided > 1)
                            player[i].avoidcollided = 1;
                        player[i].avoidcollided -= multiplier / 4;
                        if(!player[i].skeleton.free)
                            player[i].stunned -= multiplier;
                        if(!player[i].skeleton.free)
                            player[i].surprised -= multiplier;
                        if(player[i].surprised <= 0 && player[i].aitype == attacktypecutoff && i != 0 && !player[i].dead &&
                           !player[i].skeleton.free && animation[player[i].targetanimation].attack == neutral)
                            numresponded = 1;

                        if(!player[i].throwkeydown) {
                            player[i].throwtogglekeydown = 0;
                        }
                        if(player[i].throwkeydown && !player[i].throwtogglekeydown) {
                            if(player[i].weaponactive == -1 && player[i].numweapons < 2 &&
                               (player[i].isIdle() || player[i].isCrouch() || player[i].targetanimation == sneakanim ||
                                player[i].targetanimation == rollanim || player[i].targetanimation == backhandspringanim ||
                                player[i].isFlip() || player[i].isFlip() || player[i].aitype != playercontrolled)) {
                                for(j = 0; j < weapons.numweapons; j++) {
                                    if(((weapons.velocity[j].x == 0 && weapons.velocity[j].y == 0 &&
                                         weapons.velocity[j].z == 0) ||
                                        player[i].aitype == playercontrolled) &&
                                       weapons.owner[j] == -1 && player[i].weaponactive == -1)
                                        if(findDistancefastflat(&player[i].coords, &weapons.position[j]) < 2) {
                                            if(findDistancefast(&player[i].coords, &weapons.position[j]) < 2) {
                                                if(player[i].isCrouch() || player[i].targetanimation == sneakanim ||
                                                   player[i].isRun() || player[i].isIdle() ||
                                                   player[i].aitype != playercontrolled) {
                                                    player[i].throwtogglekeydown = 1;
                                                    player[i].targetanimation = crouchremoveknifeanim;
                                                    player[i].target = 0;
                                                    player[i].targetframe = 0;
                                                    rotatetarget = weapons.position[j] - player[i].coords;
                                                    Normalise(&rotatetarget);
                                                    player[i].targetrotation = -asin(0 - rotatetarget.x);
                                                    player[i].targetrotation *= 360 / 6.28;
                                                    if(rotatetarget.z < 0)
                                                        player[i].targetrotation = 180 - player[i].targetrotation;
                                                    player[i].hasvictim = 0;
                                                }
                                                if(player[i].targetanimation == rollanim ||
                                                   player[i].targetanimation == backhandspringanim) {
                                                    player[i].throwtogglekeydown = 1;
                                                    player[i].hasvictim = 0;

                                                    // for(i=0;i<weapons.numweapons;i++){
                                                    if((((weapons.velocity[j].x == 0 && weapons.velocity[j].y == 0 &&
                                                          weapons.velocity[j].z == 0) ||
                                                         player[i].aitype == playercontrolled) &&
                                                        weapons.owner[j] == -1) ||
                                                       (player[i].victim && weapons.owner[j] == player[i].victim->id))
                                                        if(findDistancefastflat(&player[i].coords, &weapons.position[j]) <
                                                               2 &&
                                                           player[i].weaponactive == -1) {
                                                            if(findDistancefast(&player[i].coords, &weapons.position[j]) <
                                                                   1 ||
                                                               player[i].victim) {
                                                                float gLoc[3];
                                                                float vel[3];
                                                                gLoc[0] = player[i].coords.x;
                                                                gLoc[1] = player[i].coords.y;
                                                                gLoc[2] = player[i].coords.z;
                                                                vel[0] = player[i].velocity.x;
                                                                vel[1] = player[i].velocity.y;
                                                                vel[2] = player[i].velocity.z;
                                                                if(weapons.type[j] != staff) {
                                                                    PlaySoundEx(
                                                                        knifedrawsound, samp[knifedrawsound], NULL, true);
                                                                    OPENAL_3D_SetAttributes(
                                                                        channels[knifedrawsound], gLoc, vel);
                                                                    OPENAL_SetVolume(channels[knifedrawsound], 128);
                                                                    OPENAL_SetPaused(channels[knifedrawsound], false);
                                                                }

                                                                player[i].weaponactive = 0;
                                                                weapons.owner[j] = player[i].id;
                                                                if(player[i].numweapons > 0) {
                                                                    player[i].weaponids[player[i].numweapons] =
                                                                        player[i].weaponids[0];
                                                                }
                                                                player[i].numweapons++;
                                                                player[i].weaponids[0] = j;
                                                            }
                                                        }
                                                    //}
                                                }
                                            } else if((player[i].isIdle() || player[i].isFlip() ||
                                                       player[i].aitype != playercontrolled) &&
                                                      findDistancefast(&player[i].coords, &weapons.position[j]) < 5 &&
                                                      player[i].coords.y < weapons.position[j].y) {
                                                if(!player[i].isFlip()) {
                                                    player[i].throwtogglekeydown = 1;
                                                    player[i].targetanimation = removeknifeanim;
                                                    player[i].target = 0;
                                                    player[i].targetframe = 0;
                                                    rotatetarget = weapons.position[j] - player[i].coords;
                                                    Normalise(&rotatetarget);
                                                    player[i].targetrotation = -asin(0 - rotatetarget.x);
                                                    player[i].targetrotation *= 360 / 6.28;
                                                    if(rotatetarget.z < 0)
                                                        player[i].targetrotation = 180 - player[i].targetrotation;
                                                }
                                                if(player[i].isFlip()) {
                                                    player[i].throwtogglekeydown = 1;
                                                    player[i].hasvictim = 0;

                                                    for(k = 0; k < weapons.numweapons; k++) {
                                                        if(player[i].weaponactive == -1)
                                                            if((((weapons.velocity[k].x == 0 &&
                                                                  weapons.velocity[k].y == 0 &&
                                                                  weapons.velocity[k].z == 0) ||
                                                                 player[i].aitype == playercontrolled) &&
                                                                weapons.owner[k] == -1) ||
                                                               (player[i].victim &&
                                                                weapons.owner[k] == player[i].victim->id))
                                                                if(findDistancefastflat(&player[i].coords,
                                                                                        &weapons.position[k]) < 3 &&
                                                                   player[i].weaponactive == -1) {
                                                                    float gLoc[3];
                                                                    float vel[3];
                                                                    gLoc[0] = player[i].coords.x;
                                                                    gLoc[1] = player[i].coords.y;
                                                                    gLoc[2] = player[i].coords.z;
                                                                    vel[0] = player[i].velocity.x;
                                                                    vel[1] = player[i].velocity.y;
                                                                    vel[2] = player[i].velocity.z;
                                                                    if(weapons.type[k] != staff) {
                                                                        PlaySoundEx(knifedrawsound,
                                                                                    samp[knifedrawsound],
                                                                                    NULL,
                                                                                    true);
                                                                        OPENAL_3D_SetAttributes(
                                                                            channels[knifedrawsound], gLoc, vel);
                                                                        OPENAL_SetVolume(channels[knifedrawsound], 128);
                                                                        OPENAL_SetPaused(channels[knifedrawsound], false);
                                                                    }

                                                                    player[i].weaponactive = 0;
                                                                    weapons.owner[k] = player[i].id;
                                                                    if(player[i].numweapons > 0) {
                                                                        player[i].weaponids[player[i].numweapons] =
                                                                            player[i].weaponids[0];
                                                                    }
                                                                    player[i].numweapons++;
                                                                    player[i].weaponids[0] = k;
                                                                }
                                                    }
                                                }
                                            }
                                        }
                                }
                                if(player[i].isCrouch() || player[i].targetanimation == sneakanim || player[i].isRun() ||
                                   player[i].isIdle() || player[i].targetanimation == rollanim ||
                                   player[i].targetanimation == backhandspringanim) {
                                    if(numplayers > 1)
                                        for(j = 0; j < numplayers; j++) {
                                            if(player[i].weaponactive == -1)
                                                if(j != i)
                                                    if(player[j].numweapons && player[j].skeleton.free &&
                                                       findDistancefast(&player[i].coords, &player[j].coords) <
                                                           2 /*&&player[j].dead*/ &&
                                                       (((player[j].skeleton.forward.y < 0 &&
                                                          player[j].weaponstuckwhere == 0) ||
                                                         (player[j].skeleton.forward.y > 0 &&
                                                          player[j].weaponstuckwhere == 1)) ||
                                                        player[j].weaponstuck == -1 || player[j].numweapons > 1)) {
                                                        if(player[i].targetanimation != rollanim &&
                                                           player[i].targetanimation != backhandspringanim) {
                                                            player[i].throwtogglekeydown = 1;
                                                            player[i].victim = &player[j];
                                                            player[i].hasvictim = 1;
                                                            player[i].targetanimation = crouchremoveknifeanim;
                                                            player[i].target = 0;
                                                            player[i].targetframe = 0;
                                                            rotatetarget = player[j].coords - player[i].coords;
                                                            Normalise(&rotatetarget);
                                                            player[i].targetrotation = -asin(0 - rotatetarget.x);
                                                            player[i].targetrotation *= 360 / 6.28;
                                                            if(rotatetarget.z < 0)
                                                                player[i].targetrotation = 180 - player[i].targetrotation;
                                                        }
                                                        if(player[i].targetanimation == rollanim ||
                                                           player[i].targetanimation == backhandspringanim) {
                                                            player[i].throwtogglekeydown = 1;
                                                            player[i].victim = &player[j];
                                                            player[i].hasvictim = 1;
                                                            int k = player[j].weaponids[0];
                                                            if(player[i].hasvictim) {
                                                                float gLoc[3];
                                                                float vel[3];
                                                                gLoc[0] = player[i].coords.x;
                                                                gLoc[1] = player[i].coords.y;
                                                                gLoc[2] = player[i].coords.z;
                                                                vel[0] = player[i].velocity.x;
                                                                vel[1] = player[i].velocity.y;
                                                                vel[2] = player[i].velocity.z;
                                                                bool fleshstuck;
                                                                fleshstuck = 0;
                                                                if(player[i].victim->weaponstuck != -1) {
                                                                    if(player[i]
                                                                           .victim
                                                                           ->weaponids[player[i].victim->weaponstuck] ==
                                                                       k) {
                                                                        fleshstuck = 1;
                                                                    }
                                                                }
                                                                if(!fleshstuck) {
                                                                    if(weapons.type[k] != staff) {
                                                                        PlaySoundEx(knifedrawsound,
                                                                                    samp[knifedrawsound],
                                                                                    NULL,
                                                                                    true);
                                                                        OPENAL_3D_SetAttributes(
                                                                            channels[knifedrawsound], gLoc, vel);
                                                                        OPENAL_SetVolume(channels[knifedrawsound], 128);
                                                                        OPENAL_SetPaused(channels[knifedrawsound], false);
                                                                    }
                                                                }
                                                                if(fleshstuck) {
                                                                    PlaySoundEx(fleshstabremovesound,
                                                                                samp[fleshstabremovesound],
                                                                                NULL,
                                                                                true);
                                                                    OPENAL_3D_SetAttributes(
                                                                        channels[fleshstabremovesound], gLoc, vel);
                                                                    OPENAL_SetVolume(channels[fleshstabremovesound], 128);
                                                                    OPENAL_SetPaused(channels[fleshstabremovesound], false);
                                                                }

                                                                player[i].weaponactive = 0;
                                                                if(weapons.owner[k] != -1) {
                                                                    if(player[i].victim->numweapons == 1)
                                                                        player[i].victim->numweapons = 0;
                                                                    else
                                                                        player[i].victim->numweapons = 1;

                                                                    player[i].victim->skeleton.longdead = 0;
                                                                    player[i].victim->skeleton.free = 1;
                                                                    player[i].victim->skeleton.broken = 0;

                                                                    for(int l = 0;
                                                                        l < player[i].victim->skeleton.num_joints;
                                                                        l++) {
                                                                        player[i].victim->skeleton.joints[l].velchange = 0;
                                                                        player[i].victim->skeleton.joints[l].locked = 0;
                                                                    }

                                                                    XYZ relative;
                                                                    relative = 0;
                                                                    relative.y = 10;
                                                                    Normalise(&relative);
                                                                    XYZ footvel, footpoint;
                                                                    footvel = 0;
                                                                    footpoint = weapons.position[k];
                                                                    if(player[i].victim->weaponstuck != -1) {
                                                                        if(player[i]
                                                                               .victim
                                                                               ->weaponids[player[i].victim->weaponstuck] ==
                                                                           k) {
                                                                            if(config.video_blood)
                                                                                sprites.MakeSprite(cloudimpactsprite,
                                                                                                   footpoint,
                                                                                                   footvel,
                                                                                                   1,
                                                                                                   0,
                                                                                                   0,
                                                                                                   .8,
                                                                                                   .3);
                                                                            weapons.bloody[k] = 2;
                                                                            weapons.blooddrip[k] = 5;
                                                                            player[i].victim->weaponstuck = -1;
                                                                            player[i].victim->bloodloss += 2000;
                                                                            player[i].victim->DoDamage(2000);
                                                                        }
                                                                    }
                                                                    if(player[i].victim->numweapons > 0) {
                                                                        if(player[i].victim->weaponstuck != 0 &&
                                                                           player[i].victim->weaponstuck != -1)
                                                                            player[i].victim->weaponstuck = 0;
                                                                        if(player[i].victim->weaponids[0] == k)
                                                                            player[i].victim->weaponids[0] =
                                                                                player[i].victim->weaponids
                                                                                    [player[i].victim->numweapons];
                                                                    }

                                                                    player[i].victim->weaponactive = -1;

                                                                    player[i]
                                                                        .victim->skeleton
                                                                        .joints[player[i]
                                                                                    .victim->skeleton.jointlabels[abdomen]]
                                                                        .velocity += relative * 6;
                                                                    player[i]
                                                                        .victim->skeleton.joints
                                                                            [player[i].victim->skeleton.jointlabels[neck]]
                                                                        .velocity += relative * 6;
                                                                    player[i]
                                                                        .victim->skeleton
                                                                        .joints[player[i].victim->skeleton.jointlabels
                                                                                    [rightshoulder]]
                                                                        .velocity += relative * 6;
                                                                    player[i]
                                                                        .victim->skeleton
                                                                        .joints[player[i].victim->skeleton.jointlabels
                                                                                    [leftshoulder]]
                                                                        .velocity += relative * 6;
                                                                }
                                                                weapons.owner[k] = i;
                                                                if(player[i].numweapons > 0) {
                                                                    player[i].weaponids[player[i].numweapons] =
                                                                        player[i].weaponids[0];
                                                                }
                                                                player[i].numweapons++;
                                                                player[i].weaponids[0] = k;
                                                            }
                                                        }
                                                    }
                                        }
                                }
                            }
                            if(player[i].weaponactive != -1 && player[i].aitype == playercontrolled) {
                                if(weapons.type[player[i].weaponids[0]] == knife) {
                                    if(player[i].isIdle() || player[i].isRun() || player[i].isCrouch() ||
                                       player[i].targetanimation == sneakanim || player[i].isFlip())
                                        if(numplayers > 1)
                                            for(j = 0; j < numplayers; j++) {
                                                if(i != j)
                                                    if(tutoriallevel != 1 || tutorialstage == 49)
                                                        if(hostile)
                                                            if(normaldotproduct(player[i].facing,
                                                                                player[i].coords - player[j].coords) < 0 &&
                                                               findDistancefast(&player[i].coords, &player[j].coords) <
                                                                   100 &&
                                                               findDistancefast(&player[i].coords, &player[j].coords) >
                                                                   1.5 &&
                                                               !player[j].skeleton.free &&
                                                               -1 ==
                                                                   checkcollide(
                                                                       DoRotation(
                                                                           player[j]
                                                                               .skeleton
                                                                               .joints[player[j].skeleton.jointlabels[head]]
                                                                               .position,
                                                                           0,
                                                                           player[j].rotation,
                                                                           0) *
                                                                               player[j].scale +
                                                                           player[j].coords,
                                                                       DoRotation(
                                                                           player[i]
                                                                               .skeleton
                                                                               .joints[player[i].skeleton.jointlabels[head]]
                                                                               .position,
                                                                           0,
                                                                           player[i].rotation,
                                                                           0) *
                                                                               player[i].scale +
                                                                           player[i].coords)) {
                                                                if(!player[i].isFlip()) {
                                                                    player[i].throwtogglekeydown = 1;
                                                                    player[i].victim = &player[j];
                                                                    player[i].targetanimation = knifethrowanim;
                                                                    player[i].target = 0;
                                                                    player[i].targetframe = 0;
                                                                    rotatetarget = player[j].coords - player[i].coords;
                                                                    Normalise(&rotatetarget);
                                                                    player[i].targetrotation = -asin(0 - rotatetarget.x);
                                                                    player[i].targetrotation *= 360 / 6.28;
                                                                    if(rotatetarget.z < 0)
                                                                        player[i].targetrotation =
                                                                            180 - player[i].targetrotation;

                                                                    player[i].targettilt2 =
                                                                        -asin(rotatetarget.y) * 360 / 6.28;
                                                                }
                                                                if(player[i].isFlip()) {
                                                                    if(player[i].weaponactive != -1) {
                                                                        player[i].throwtogglekeydown = 1;
                                                                        player[i].victim = &player[j];
                                                                        XYZ aim;
                                                                        weapons.owner[player[i].weaponids[0]] = -1;
                                                                        aim =
                                                                            player[i].victim->coords +
                                                                            DoRotation(
                                                                                player[i]
                                                                                    .victim->skeleton
                                                                                    .joints[player[i]
                                                                                                .victim->skeleton
                                                                                                .jointlabels[abdomen]]
                                                                                    .position,
                                                                                0,
                                                                                player[i].victim->rotation,
                                                                                0) *
                                                                                player[i].victim->scale +
                                                                            player[i].victim->velocity *
                                                                                findDistance(&player[i].victim->coords,
                                                                                             &player[i].coords) /
                                                                                50 -
                                                                            (player[i].coords +
                                                                             DoRotation(
                                                                                 player[i]
                                                                                     .skeleton
                                                                                     .joints[player[i].skeleton.jointlabels
                                                                                                 [righthand]]
                                                                                     .position,
                                                                                 0,
                                                                                 player[i].rotation,
                                                                                 0) *
                                                                                 player[i].scale);
                                                                        Normalise(&aim);

                                                                        aim = DoRotation(aim,
                                                                                         (float)abs(rand() % 30) - 15,
                                                                                         (float)abs(rand() % 30) - 15,
                                                                                         0);

                                                                        weapons.velocity[player[i].weaponids[0]] = aim * 50;
                                                                        weapons.tipvelocity[player[i].weaponids[0]] =
                                                                            aim * 50;
                                                                        weapons.missed[player[i].weaponids[0]] = 0;
                                                                        weapons.freetime[player[i].weaponids[0]] = 0;
                                                                        weapons.firstfree[player[i].weaponids[0]] = 1;
                                                                        weapons.physics[player[i].weaponids[0]] = 0;
                                                                        player[i].numweapons--;
                                                                        if(player[i].numweapons) {
                                                                            player[i].weaponids[0] =
                                                                                player[i].weaponids[player[i].numweapons];
                                                                        }
                                                                        player[i].weaponactive = -1;
                                                                    }
                                                                }
                                                            }
                                            }
                                }
                            }
                            if(player[i].weaponactive != -1 && player[i].aitype == playercontrolled) {
                                if(player[i].isCrouch() || player[i].targetanimation == sneakanim) {
                                    player[i].throwtogglekeydown = 1;
                                    weapons.owner[player[i].weaponids[0]] = -1;
                                    weapons.velocity[player[i].weaponids[0]] = player[i].velocity * .2;
                                    if(weapons.velocity[player[i].weaponids[0]].x == 0)
                                        weapons.velocity[player[i].weaponids[0]].x = .1;
                                    weapons.tipvelocity[player[i].weaponids[0]] = weapons.velocity[player[i].weaponids[0]];
                                    weapons.missed[player[i].weaponids[0]] = 1;
                                    weapons.freetime[player[i].weaponids[0]] = 0;
                                    weapons.firstfree[player[i].weaponids[0]] = 1;
                                    weapons.physics[player[i].weaponids[0]] = 1;
                                    player[i].numweapons--;
                                    if(player[i].numweapons) {
                                        player[i].weaponids[0] = player[i].weaponids[player[i].numweapons];
                                        if(player[i].weaponstuck == player[i].numweapons)
                                            player[i].weaponstuck = 0;
                                    }

                                    player[i].weaponactive = -1;
                                    for(j = 0; j < numplayers; j++) {
                                        player[j].wentforweapon = 0;
                                    }
                                }
                            }
                        }

                        if(i == 0 || !player[0].dead || player[i].weaponactive != -1)
                            if((player[i].drawkeydown && !player[i].drawtogglekeydown) ||
                               (player[i].numweapons == 2 && player[i].weaponactive == -1 && player[i].isIdle()) ||
                               (player[0].dead && player[i].weaponactive != -1 && i != 0)) {
                                // Setenvironment(1-environment);
                                bool isgood;
                                isgood = 1;
                                if(player[i].weaponactive != -1) {
                                    if(weapons.type[player[i].weaponids[player[i].weaponactive]] == staff)
                                        isgood = 0;
                                }
                                if(/*(player[i].weaponactive==-1||player[i].numweapons==1)&&*/ isgood &&
                                   player[i].creature != wolftype) {
                                    if(player[i].isIdle() && player[i].numweapons &&
                                       weapons.type[player[i].weaponids[0]] == knife) {
                                        player[i].targetanimation = drawrightanim;
                                        player[i].targetframe = 0;
                                        player[i].target = 0;
                                        player[i].drawtogglekeydown = 1;
                                    }
                                    if((player[i].isIdle() || (player[i].aitype != playercontrolled &&
                                                               player[0].weaponactive != -1 && player[i].isRun())) &&
                                       player[i].numweapons && weapons.type[player[i].weaponids[0]] == sword) {
                                        player[i].targetanimation = drawleftanim;
                                        player[i].targetframe = 0;
                                        player[i].target = 0;
                                        player[i].drawtogglekeydown = 1;
                                    }
                                    if(player[i].isCrouch() && player[i].numweapons &&
                                       weapons.type[player[i].weaponids[0]] == knife) {
                                        player[i].targetanimation = crouchdrawrightanim;
                                        player[i].targetframe = 0;
                                        player[i].target = 0;
                                        player[i].drawtogglekeydown = 1;
                                    }
                                }
                            }
                        if(player[i].isCrouch() && weapons.bloody[player[i].weaponids[player[i].weaponactive]] &&
                           config.video_blood && player[i].onterrain && player[i].numweapons && player[i].weaponactive != -1 &&
                           player[i].attackkeydown) {
                            if(weapons.bloody[player[i].weaponids[player[i].weaponactive]] && player[i].onterrain &&
                               config.video_blood && musictype != stream_music2) {
                                if(weapons.type[player[i].weaponids[player[i].weaponactive]] == knife)
                                    player[i].targetanimation = crouchstabanim;
                                if(weapons.type[player[i].weaponids[player[i].weaponactive]] == sword)
                                    player[i].targetanimation = swordgroundstabanim;
                                player[i].targetframe = 0;
                                player[i].target = 0;
                                player[i].hasvictim = 0;
                                // player[i].attacktogglekeydown=1;
                            }
                        }

                        if(!player[i].drawkeydown) {
                            player[i].drawtogglekeydown = 0;
                        }

                        if(i == 0) {
                            absflatfacing = 0;
                            absflatfacing.z = -1;

                            absflatfacing = DoRotation(absflatfacing, 0, -rotation, 0);
                        } else
                            absflatfacing = flatfacing;

                        if(indialogue != -1) {
                            player[i].forwardkeydown = 0;
                            player[i].leftkeydown = 0;
                            player[i].backkeydown = 0;
                            player[i].rightkeydown = 0;
                            player[i].jumpkeydown = 0;
                            player[i].crouchkeydown = 0;
                            player[i].drawkeydown = 0;
                            player[i].throwkeydown = 0;
                        }
                        movekey = 0;
                        // Do controls
                        if(!animation[player[i].targetanimation].attack &&
                           player[i].targetanimation != staggerbackhighanim &&
                           player[i].targetanimation != staggerbackhardanim &&
                           player[i].targetanimation != backhandspringanim && player[i].targetanimation != dodgebackanim) {
                            if(!player[i].forwardkeydown) {
                                player[i].forwardstogglekeydown = 0;
                            }
                            if(player[i].crouchkeydown) {
                                // Crouch
                                target = -2;
                                if(i == 0) {
                                    player[i].superruntoggle = 1;
                                    if(numplayers > 1)
                                        for(j = 0; j < numplayers; j++) {
                                            if(j != i && !player[j].skeleton.free && player[j].aitype == passivetype) {
                                                if(findDistancefast(&player[j].coords, &player[i].coords) < 16) {
                                                    player[i].superruntoggle = 0;
                                                }
                                            }
                                        }
                                }

                                if(numplayers > 1)
                                    for(j = 0; j < numplayers; j++) {
                                        if(j != i && !player[j].skeleton.free && player[j].victim &&
                                           player[i].lowreversaldelay <= 0) {
                                            if(findDistancefast(&player[j].coords, &player[j].victim->coords) < 3 &&
                                               player[j].victim == &player[i] &&
                                               (player[j].targetanimation == sweepanim ||
                                                player[j].targetanimation == upunchanim ||
                                                player[j].targetanimation == wolfslapanim ||
                                                ((player[j].targetanimation == swordslashanim ||
                                                  player[j].targetanimation == knifeslashstartanim ||
                                                  player[j].targetanimation == staffhitanim ||
                                                  player[j].targetanimation == staffspinhitanim) &&
                                                 findDistancefast(&player[j].coords, &player[i].coords) < 2))) {
                                                if(target >= 0)
                                                    target = -1;
                                                else
                                                    target = j;
                                            }
                                        }
                                    }
                                if(target >= 0)
                                    player[target].Reverse();
                                player[i].lowreversaldelay = .5;

                                if(player[i].isIdle()) {
                                    player[i].targetanimation = player[i].getCrouch();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                    player[i].transspeed = 10;
                                }
                                if(player[i].isRun() ||
                                   (player[i].isStop() && (player[i].leftkeydown || player[i].rightkeydown ||
                                                           player[i].forwardkeydown || player[i].backkeydown))) {
                                    player[i].targetanimation = rollanim;
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                    player[i].transspeed = 20;
                                }
                            }
                            if(!player[i].crouchkeydown) {
                                // Uncrouch
                                if(!player[i].isRun() && player[i].targetanimation != sneakanim && i == 0)
                                    player[i].superruntoggle = 0;
                                target = -2;
                                if(player[i].isCrouch()) {
                                    if(numplayers > 1)
                                        for(j = 0; j < numplayers; j++) {
                                            if(j != i && !player[j].skeleton.free && player[j].victim &&
                                               player[i].highreversaldelay <= 0) {
                                                if(findDistancefast(&player[j].coords, &player[j].victim->coords) < 3 &&
                                                   player[j].victim == &player[i] &&
                                                   (player[j].targetanimation == spinkickanim) && player[i].isCrouch()) {
                                                    if(target >= 0)
                                                        target = -1;
                                                    else
                                                        target = j;
                                                }
                                            }
                                        }
                                    if(target >= 0)
                                        player[target].Reverse();
                                    player[i].highreversaldelay = .5;

                                    if(player[i].isCrouch()) {
                                        if(!player[i].wasCrouch()) {
                                            player[i].currentanimation = player[i].getCrouch();
                                            player[i].currentframe = 0;
                                        }
                                        player[i].target = 0;
                                        player[i].targetanimation = player[i].getIdle();
                                        player[i].targetframe = 0;
                                        player[i].transspeed = 10;
                                    }
                                }
                                if(player[i].targetanimation == sneakanim) {
                                    player[i].targetanimation = player[i].getIdle();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                    player[i].transspeed = 10;
                                }
                            }
                            if(player[i].forwardkeydown) {
                                if(player[i].isIdle() ||
                                   (player[i].isStop() && player[i].targetrotation == player[i].rotation) ||
                                   (player[i].isLanding() && player[i].targetframe > 0 && !player[i].jumpkeydown) ||
                                   (player[i].isLandhard() && player[i].targetframe > 0 && !player[i].jumpkeydown &&
                                    player[i].crouchkeydown)) {
                                    if(player[i].aitype == passivetype)
                                        player[i].targetanimation = walkanim;
                                    else
                                        player[i].targetanimation = player[i].getRun();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].isCrouch()) {
                                    player[i].targetanimation = sneakanim;
                                    if(player[i].wasCrouch())
                                        player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].targetanimation ==
                                   hanganim /*&&(!player[i].forwardstogglekeydown||player[i].aitype!=playercontrolled)*/) {
                                    player[i].targetanimation = climbanim;
                                    player[i].target = 0;
                                    player[i].targetframe = 1;
                                    player[i].jumpclimb = 1;
                                }
                                if(player[i].targetanimation == jumpupanim || player[i].targetanimation == jumpdownanim ||
                                   player[i].isFlip()) {
                                    player[i].velocity += absflatfacing * 5 * multiplier;
                                }
                                player[i].forwardstogglekeydown = 1;
                                movekey = 1;
                            }
                            if(player[i].rightkeydown) {
                                if(player[i].isIdle() ||
                                   (player[i].isStop() && player[i].targetrotation == player[i].rotation) ||
                                   (player[i].isLanding() && player[i].targetframe > 0 && !player[i].jumpkeydown) ||
                                   (player[i].isLandhard() && player[i].targetframe > 0 && !player[i].jumpkeydown &&
                                    player[i].crouchkeydown)) {
                                    player[i].targetanimation = player[i].getRun();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].isCrouch()) {
                                    player[i].targetanimation = sneakanim;
                                    if(player[i].wasCrouch())
                                        player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].targetanimation == jumpupanim || player[i].targetanimation == jumpdownanim ||
                                   player[i].isFlip()) {
                                    player[i].velocity += DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                                }
                                player[i].targetrotation -= 90;
                                if(player[i].forwardkeydown)
                                    player[i].targetrotation += 45;
                                if(player[i].backkeydown)
                                    player[i].targetrotation -= 45;
                                movekey = 1;
                            }
                            if(player[i].leftkeydown) {
                                if(player[i].isIdle() ||
                                   (player[i].isStop() && player[i].targetrotation == player[i].rotation) ||
                                   (player[i].isLanding() && player[i].targetframe > 0 && !player[i].jumpkeydown) ||
                                   (player[i].isLandhard() && player[i].targetframe > 0 && !player[i].jumpkeydown &&
                                    player[i].crouchkeydown)) {
                                    player[i].targetanimation = player[i].getRun();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].isCrouch()) {
                                    player[i].targetanimation = sneakanim;
                                    if(player[i].wasCrouch())
                                        player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].targetanimation == jumpupanim || player[i].targetanimation == jumpdownanim ||
                                   player[i].isFlip()) {
                                    player[i].velocity -= DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                                }
                                player[i].targetrotation += 90;
                                if(player[i].forwardkeydown)
                                    player[i].targetrotation -= 45;
                                if(player[i].backkeydown)
                                    player[i].targetrotation += 45;
                                movekey = 1;
                            }
                            if(player[i].backkeydown) {
                                if(player[i].isIdle() ||
                                   (player[i].isStop() && player[i].targetrotation == player[i].rotation) ||
                                   (player[i].isLanding() && player[i].targetframe > 0 && !player[i].jumpkeydown) ||
                                   (player[i].isLandhard() && player[i].targetframe > 0 && !player[i].jumpkeydown &&
                                    player[i].crouchkeydown)) {
                                    player[i].targetanimation = player[i].getRun();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].isCrouch()) {
                                    player[i].targetanimation = sneakanim;
                                    if(player[i].wasCrouch())
                                        player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].targetanimation == jumpupanim || player[i].targetanimation == jumpdownanim ||
                                   player[i].isFlip()) {
                                    player[i].velocity -= absflatfacing * 5 * multiplier;
                                }
                                if(player[i].targetanimation == hanganim) {
                                    player[i].currentanimation = jumpdownanim;
                                    player[i].targetanimation = jumpdownanim;
                                    player[i].target = 0;
                                    player[i].currentframe = 0;
                                    player[i].targetframe = 1;
                                    player[i].velocity = 0;
                                    player[i].velocity.y += gravity;
                                    player[i].coords.y -= 1.4;
                                    player[i].grabdelay = 1;
                                }
                                if(!player[i].leftkeydown && !player[i].rightkeydown)
                                    player[i].targetrotation += 180;
                                movekey = 1;
                            }
                            if((player[i].jumpkeydown && !player[i].jumpclimb) || player[i].jumpstart) {
                                if((((player[i].isLanding() && player[i].targetframe >= 3) || player[i].isRun() ||
                                     player[i].targetanimation == walkanim || player[i].isCrouch() ||
                                     player[i].targetanimation == sneakanim) &&
                                    player[i].jumppower > 1) &&
                                   ((player[i].targetanimation != rabbitrunninganim &&
                                     player[i].targetanimation != wolfrunninganim) ||
                                    i != 0)) {
                                    player[i].jumpstart = 0;
                                    player[i].targetanimation = jumpupanim;
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                    player[i].rotation = player[i].targetrotation;
                                    player[i].transspeed = 20;
                                    player[i].FootLand(0, 1);
                                    player[i].FootLand(1, 1);

                                    facing = 0;
                                    facing.z = -1;
                                    flatfacing = DoRotation(facing, 0, player[i].targetrotation + 180, 0);

                                    if(movekey)
                                        player[i].velocity = flatfacing * player[i].speed * 45 * player[i].scale;
                                    if(!movekey)
                                        player[i].velocity = 0;

                                    // Dodge sweep?
                                    target = -2;
                                    if(numplayers > 1)
                                        for(j = 0; j < numplayers; j++) {
                                            if(j != i && !player[j].skeleton.free && player[j].victim) {
                                                if(findDistancefast(&player[j].coords, &player[j].victim->coords) < 3 &&
                                                   player[j].victim == &player[i] &&
                                                   (player[j].targetanimation == sweepanim)) {
                                                    if(target >= 0)
                                                        target = -1;
                                                    else
                                                        target = j;
                                                }
                                            }
                                        }
                                    if(target >= 0)
                                        player[i].velocity.y = 1;
                                    else if(player[i].crouchkeydown || player[i].aitype != playercontrolled) {
                                        player[i].velocity.y = 7;
                                        player[i].crouchtogglekeydown = 1;
                                    } else
                                        player[i].velocity.y = 5;

                                    if(config.input_mouse_jump && i == 0 && config.game_debug) {
                                        if(!player[i].isLanding())
                                            player[i].tempdeltav = deltav;
                                        if(player[i].tempdeltav < 0)
                                            player[i].velocity.y -= (float)(player[i].tempdeltav) / multiplier / 1000;
                                    }

                                    player[i].coords.y += .2;
                                    player[i].jumppower -= 1;

                                    static float gLoc[3];
                                    static float vel[3];
                                    gLoc[0] = player[i].coords.x;
                                    gLoc[1] = player[i].coords.y;
                                    gLoc[2] = player[i].coords.z;
                                    vel[0] = player[i].velocity.x;
                                    vel[1] = player[i].velocity.y;
                                    vel[2] = player[i].velocity.z;

                                    if(i == 0) {
                                        PlaySoundEx(whooshsound, samp[whooshsound], NULL, true);
                                        OPENAL_3D_SetAttributes(channels[whooshsound], gLoc, vel);
                                        OPENAL_SetVolume(channels[whooshsound], 128);
                                        OPENAL_SetPaused(channels[whooshsound], false);
                                    }

                                    PlaySoundEx(jumpsound, samp[jumpsound], NULL, true);
                                    OPENAL_3D_SetAttributes(channels[jumpsound], gLoc, vel);
                                    OPENAL_SetVolume(channels[jumpsound], 128);
                                    OPENAL_SetPaused(channels[jumpsound], false);
                                }
                                if((player[i].isIdle()) && player[i].jumppower > 1) {
                                    player[i].targetanimation = player[i].getLanding();
                                    player[i].landhard = 0;
                                    player[i].target = 0;
                                    player[i].targetframe = 2;
                                    player[i].jumpstart = 1;
                                    player[i].tempdeltav = deltav;
                                }
                                if(player[i].targetanimation == jumpupanim &&
                                   (((!config.input_floatjump && !editorenabled) || !config.game_debug) ||
                                    player[i].aitype != playercontrolled)) {
                                    if(player[i].jumppower > multiplier * 6) {
                                        player[i].velocity.y += multiplier * 6;
                                        player[i].jumppower -= multiplier * 6;
                                    }
                                    if(player[i].jumppower <= multiplier * 6) {
                                        player[i].velocity.y += player[i].jumppower;
                                        player[i].jumppower = 0;
                                    }
                                }
                                if(((config.input_floatjump || editorenabled) && config.game_debug) && i == 0)
                                    player[i].velocity.y += multiplier * 30;
                            }

                            if(!movekey) {
                                if(player[i].isRun() || player[i].targetanimation == walkanim) {
                                    player[i].targetanimation = player[i].getStop();
                                    player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                                if(player[i].targetanimation == sneakanim) {
                                    player[i].targetanimation = player[i].getCrouch();
                                    if(player[i].currentanimation == sneakanim)
                                        player[i].target = 0;
                                    player[i].targetframe = 0;
                                }
                            }
                            if(player[i].targetanimation == walkanim &&
                               (player[i].aitype == attacktypecutoff || player[i].aitype == searchtype ||
                                (player[i].aitype == passivetype && player[i].numwaypoints <= 1))) {
                                player[i].targetanimation = player[i].getStop();
                                player[i].target = 0;
                                player[i].targetframe = 0;
                            }
                            if(player[i].isRun() && (player[i].aitype == passivetype)) {
                                player[i].targetanimation = player[i].getStop();
                                player[i].target = 0;
                                player[i].targetframe = 0;
                            }
                        }
                    }
                    if(player[i].targetanimation == rollanim)
                        player[i].targetrotation = oldtargetrotation;
                }

                // Rotation
                for(k = 0; k < numplayers; k++) {
                    if(abs(player[k].rotation - player[k].targetrotation) > 180) {
                        if(player[k].rotation > player[k].targetrotation)
                            player[k].rotation -= 360;
                        else
                            player[k].rotation += 360;
                    }

                    if(abs(player[k].rotation - player[k].targetrotation) > 90 &&
                       (player[k].isRun() || player[k].targetanimation == walkanim)) {
                        player[k].targetanimation = player[k].getStop();
                        player[k].targetframe = 0;
                        player[k].target = 0;
                    }

                    if(player[k].targetanimation == backhandspringanim || player[k].targetanimation == dodgebackanim) {
                        player[k].targettilt = 0;
                    }
                    if(player[k].targetanimation != jumpupanim && player[k].targetanimation != backhandspringanim &&
                       player[k].targetanimation != jumpdownanim && !player[k].isFlip()) {
                        player[k].targettilt = 0;
                        if(player[k].jumppower < 0 && !player[k].jumpkeydown)
                            player[k].jumppower = 0;
                        player[k].jumppower += multiplier * 7;
                        if(player[k].isCrouch())
                            player[k].jumppower += multiplier * 7;
                        //*(1-(player[k].damage/player[k].damagetolerance))
                        if(player[k].jumppower > 5)
                            player[k].jumppower = 5;
                    }

                    if(player[k].isRun()) {
                        player[k].targettilt = (player[k].rotation - player[k].targetrotation) / 4;
                    }

                    if(abs(player[k].tilt - player[k].targettilt) < multiplier * 150)
                        player[k].tilt = player[k].targettilt;
                    else if(player[k].tilt > player[k].targettilt) {
                        player[k].tilt -= multiplier * 150;
                    } else if(player[k].tilt < player[k].targettilt) {
                        player[k].tilt += multiplier * 150;
                    }

                    player[k].grabdelay -= multiplier;
                }

                for(k = 0; k < numplayers; k++) {
                    player[k].DoAnimations();
                    player[k].whichpatchx =
                        player[k].coords.x / (terrain.size / subdivision * terrain.scale * terraindetail);
                    player[k].whichpatchz =
                        player[k].coords.z / (terrain.size / subdivision * terrain.scale * terraindetail);
                }

                objects.DoStuff();
                /*
                player[0].righthandmorphstart=0;
                player[0].righthandmorphend=1;
                player[0].lefthandmorphstart=0;
                player[0].lefthandmorphend=1;
                player[0].headmorphstart=0;
                player[0].headmorphend=2;*/

                /*
                if(IsKeyDown( theKeyMap, MAC_P_KEY )){
                if(player[0].righthandmorphend!=1)player[0].righthandmorphness=0;
                player[0].righthandmorphend=1;
                player[0].targetrighthandmorphness=1;

                if(player[0].lefthandmorphend!=0)player[0].lefthandmorphness=0;
                player[0].lefthandmorphend=0;
                player[0].targetlefthandmorphness=1;

                if(player[0].headmorphend!=2)player[0].headmorphness=0;
                player[0].headmorphend=2;
                player[0].targetheadmorphness=1;
                }
                if(IsKeyDown( theKeyMap, MAC_L_KEY )){
                if(player[0].righthandmorphend!=0)player[0].righthandmorphness=0;
                player[0].righthandmorphend=0;
                player[0].targetrighthandmorphness=1;

                if(player[0].lefthandmorphend!=1)player[0].lefthandmorphness=0;
                player[0].lefthandmorphend=1;
                player[0].targetlefthandmorphness=1;

                if(player[0].headmorphend!=0)player[0].headmorphness=0;
                player[0].headmorphend=0;
                player[0].targetheadmorphness=1;
                }
                */
                if(numenvsounds != 0)
                    for(j = numenvsounds - 1; j >= 0; j--) {
                        envsoundlife[j] -= multiplier;
                        if(envsoundlife[j] < 0) {
                            numenvsounds--;
                            envsoundlife[j] = envsoundlife[numenvsounds];
                            envsound[j] = envsound[numenvsounds];
                        }
                    }
                if(!slomo)
                    OPENAL_SetFrequency(OPENAL_ALL, 22050);
                if(slomo)
                    OPENAL_SetFrequency(OPENAL_ALL, slomofreq);

                if(tutoriallevel == 1) {
                    XYZ temp;
                    XYZ temp2;
                    XYZ temp3;
                    XYZ oldtemp;
                    XYZ oldtemp2;
                    temp.x = 1011;
                    temp.y = 84;
                    temp.z = 491;
                    temp2.x = 1025;
                    temp2.y = 75;
                    temp2.z = 447;
                    temp3.x = 1038;
                    temp3.y = 76;
                    temp3.z = 453;
                    oldtemp = temp;
                    oldtemp2 = temp2;
                    if(tutorialstage >= 51)
                        if(findDistancefast(&temp, &player[0].coords) >= findDistancefast(&temp, &temp2) - 1 ||
                           findDistancefast(&temp3, &player[0].coords) < 4) {
                            OPENAL_StopSound(OPENAL_ALL); // hack...OpenAL renderer isn't stopping music after tutorial goes
                                                          // to level menu...
                            OPENAL_SetFrequency(OPENAL_ALL, 1);

                            PlayStreamEx(stream_music3, strm[stream_music3], NULL, true);
                            OPENAL_SetPaused(channels[stream_music3], false);
                            OPENAL_SetVolume(channels[stream_music3], 256);

                            gameon = 0;
                            mainmenu = 5;

                            float gLoc[3] = { 0, 0, 0 };
                            float vel[3] = { 0, 0, 0 };
                            OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                            PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                            OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                            OPENAL_SetVolume(channels[fireendsound], 256);
                            OPENAL_SetPaused(channels[fireendsound], false);
                            OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                            flashr = 1;
                            flashg = 0;
                            flashb = 0;
                            flashamount = 1;
                            flashdelay = 1;
                        }
                    if(tutorialstage < 51)
                        if(findDistancefast(&temp, &player[0].coords) >= findDistancefast(&temp, &temp2) - 1 ||
                           findDistancefast(&temp3, &player[0].coords) < 4) {
                            float gLoc[3];
                            float vel[3];
                            gLoc[0] = player[0].coords.x;
                            gLoc[1] = player[0].coords.y;
                            gLoc[2] = player[0].coords.z;
                            vel[0] = 0;
                            vel[1] = 0;
                            vel[2] = 0;
                            PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                            OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                            OPENAL_SetVolume(channels[fireendsound], 256);
                            OPENAL_SetPaused(channels[fireendsound], false);

                            player[0].coords = (oldtemp + oldtemp2) / 2;

                            flashr = 1;
                            flashg = 1;
                            flashb = 1;
                            flashamount = 1;
                            flashdelay = 1;
                        }
                    if(tutorialstage >= 14 && tutorialstage < 50)
                        if(findDistancefast(&temp, &player[1].coords) >= findDistancefast(&temp, &temp2) - 1 ||
                           findDistancefast(&temp3, &player[1].coords) < 4) {
                            float gLoc[3];
                            float vel[3];
                            gLoc[0] = player[1].coords.x;
                            gLoc[1] = player[1].coords.y;
                            gLoc[2] = player[1].coords.z;
                            vel[0] = 0;
                            vel[1] = 0;
                            vel[2] = 0;
                            PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                            OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                            OPENAL_SetVolume(channels[fireendsound], 256);
                            OPENAL_SetPaused(channels[fireendsound], false);

                            for(int i = 0; i < player[1].skeleton.num_joints; i++) {
                                if(rand() % 2 == 0) {
                                    if(!player[1].skeleton.free)
                                        temp2 = (player[1].coords - player[1].oldcoords) / multiplier / 2; // velocity/2;
                                    if(player[1].skeleton.free)
                                        temp2 = player[1].skeleton.joints[i].velocity * player[1].scale / 2;
                                    if(!player[1].skeleton.free)
                                        temp =
                                            DoRotation(
                                                DoRotation(
                                                    DoRotation(player[1].skeleton.joints[i].position, 0, 0, player[1].tilt),
                                                    player[1].tilt2,
                                                    0,
                                                    0),
                                                0,
                                                player[1].rotation,
                                                0) *
                                                player[1].scale +
                                            player[1].coords;
                                    if(player[1].skeleton.free)
                                        temp = player[1].skeleton.joints[i].position * player[1].scale + player[1].coords;
                                    sprites.MakeSprite(
                                        breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(rand() % 100) / 200 - .25, 1);
                                }
                            }

                            player[1].coords = (oldtemp + oldtemp2) / 2;
                            for(int i = 0; i < player[1].skeleton.num_joints; i++) {
                                player[1].skeleton.joints[i].velocity = 0;
                                if(rand() % 2 == 0) {
                                    if(!player[1].skeleton.free)
                                        temp2 = (player[1].coords - player[1].oldcoords) / multiplier / 2; // velocity/2;
                                    if(player[1].skeleton.free)
                                        temp2 = player[1].skeleton.joints[i].velocity * player[1].scale / 2;
                                    if(!player[1].skeleton.free)
                                        temp =
                                            DoRotation(
                                                DoRotation(
                                                    DoRotation(player[1].skeleton.joints[i].position, 0, 0, player[1].tilt),
                                                    player[1].tilt2,
                                                    0,
                                                    0),
                                                0,
                                                player[1].rotation,
                                                0) *
                                                player[1].scale +
                                            player[1].coords;
                                    if(player[1].skeleton.free)
                                        temp = player[1].skeleton.joints[i].position * player[1].scale + player[1].coords;
                                    sprites.MakeSprite(
                                        breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(rand() % 100) / 200 - .25, 1);
                                }
                            }
                        }
                }

                // 3d sound
                static float gLoc[3];
                gLoc[0] = viewer.x;
                gLoc[1] = viewer.y;
                gLoc[2] = viewer.z;
                static float vel[3];
                vel[0] = (viewer.x - oldviewer.x) / multiplier;
                vel[1] = (viewer.y - oldviewer.y) / multiplier;
                vel[2] = (viewer.z - oldviewer.z) / multiplier;

                // Set orientation with forward and up vectors
                static XYZ upvector;
                upvector = 0;
                upvector.z = -1;

                upvector = DoRotation(upvector, -rotation2 + 90, 0, 0);
                upvector = DoRotation(upvector, 0, 0 - rotation, 0);

                facing = 0;
                facing.z = -1;

                facing = DoRotation(facing, -rotation2, 0, 0);
                facing = DoRotation(facing, 0, 0 - rotation, 0);

                static float ori[6];
                ori[0] = -facing.x;
                ori[1] = facing.y;
                ori[2] = -facing.z;
                ori[3] = -upvector.x;
                ori[4] = upvector.y;
                ori[5] = -upvector.z;

                OPENAL_3D_Listener_SetAttributes(&gLoc[0], &vel[0], ori[0], ori[1], ori[2], ori[3], ori[4], ori[5]);
                OPENAL_Update();

                oldviewer = viewer;
            }
        }

        if(IsKeyDown(SDL_SCANCODE_F1) && !freezetogglekeydown) {
            Screenshot();
            freezetogglekeydown = 1;
        }
    }

    void Game::TickOnce()
    {
        // if(!console){
        if(!mainmenu)
            if(directing || indialogue == -1) {
                rotation += deltah * .7;
                if(!config.input_mouse_invert)
                    rotation2 += deltav * .7;
                if(config.input_mouse_invert)
                    rotation2 -= deltav * .7;
                if(rotation2 > 90)
                    rotation2 = 90;
                if(rotation2 < -70)
                    rotation2 = -70;
            }
        if(mainmenu)
            rotation += multiplier * 5;

        //}
    }

    void Game::TickOnceAfter()
    {
        static XYZ colviewer;
        static XYZ coltarget;
        static XYZ target;
        static XYZ col;
        static float brotate;
        static XYZ facing;
        static int i, j;
        static float changedelay;
        static bool alldead;
        static float unseendelay;
        static float cameraspeed;

        if(!mainmenu) {

            if(environment == snowyenvironment)
                music1 = stream_music1snow;
            if(environment == grassyenvironment)
                music1 = stream_music1grass;
            if(environment == desertenvironment)
                music1 = stream_music1desert;

            realthreat = 0;

            musictype = music1;
            for(i = 0; i < numplayers; i++) {
                if((player[i].aitype == attacktypecutoff || player[i].aitype == getweapontype ||
                    player[i].aitype == gethelptype || player[i].aitype == searchtype) &&
                   !player[i].dead /*&&player[i].surprised<=0*/ &&
                   (player[i].targetanimation != sneakattackedanim && player[i].targetanimation != knifesneakattackedanim &&
                    player[i].targetanimation != swordsneakattackedanim)) {
                    musictype = stream_music2;
                    realthreat = 1;
                }
            }
            if(player[0].dead)
                musictype = stream_music3;

            if(musictype == stream_music2) {
                unseendelay = 1;
            }

            if(oldmusictype == stream_music2 && musictype != stream_music2) {
                unseendelay -= multiplier;
                if(unseendelay > 0) {
                    musictype = stream_music2;
                }
            }

            if(loading == 2) {
                musictype = stream_music3;
                musicvolume[2] = 512;
                musicvolume[0] = 0;
                musicvolume[1] = 0;
                musicvolume[3] = 0;
            }

            if(config.sound_music) {
                if(musictype != oldmusictype && musictype == stream_music2) {
                    static float gLoc[3];
                    static float vel[3];
                    gLoc[0] = cameraloc.x;
                    gLoc[1] = cameraloc.y;
                    gLoc[2] = cameraloc.z;
                    vel[0] = 0;
                    vel[1] = 0;
                    vel[2] = 0;
                    PlaySoundEx(alarmsound, samp[alarmsound], NULL, true);
                    OPENAL_SetVolume(channels[alarmsound], 512);
                    OPENAL_SetPaused(channels[alarmsound], false);
                }
            }
            musicselected = musictype;

            if(musicselected == music1)
                musicvolume[0] += multiplier * 450;
            else
                musicvolume[0] -= multiplier * 450;
            if(musicselected == stream_music2)
                musicvolume[1] += multiplier * 450;
            else
                musicvolume[1] -= multiplier * 450;
            if(musicselected == stream_music3)
                musicvolume[2] += multiplier * 450;
            else
                musicvolume[2] -= multiplier * 450;
            /*
            if(musicselected==music1)musicvolume[0]+=multiplier*100;
            else musicvolume[0]-=multiplier*450;
            if(musicselected==music2)musicvolume[1]+=multiplier*150;
            else if(player[0].dead)musicvolume[1]-=multiplier*450;
            else musicvolume[1]-=multiplier*100;
            if(musicselected==music3)musicvolume[2]+=multiplier*450;
            else musicvolume[2]-=multiplier*450;*/

            for(i = 0; i < 3; i++) {
                if(musicvolume[i] < 0)
                    musicvolume[i] = 0;
                if(musicvolume[i] > 512)
                    musicvolume[i] = 512;
            }

            if(musicvolume[2] > 128 && !loading && !mainmenu)
                musicvolume[2] = 128;

            if(config.sound_music) {
                if(musicvolume[0] > 0 && oldmusicvolume[0] <= 0) {
                    PlayStreamEx(music1, strm[music1], NULL, true);
                    OPENAL_SetPaused(channels[music1], false);
                }
                if(musicvolume[1] > 0 && oldmusicvolume[1] <= 0) {
                    PlayStreamEx(stream_music2, strm[stream_music2], NULL, true);
                    OPENAL_SetPaused(channels[stream_music2], false);
                }
                if(musicvolume[2] > 0 && oldmusicvolume[2] <= 0) {
                    PlayStreamEx(stream_music3, strm[stream_music3], NULL, true);
                    OPENAL_SetPaused(channels[stream_music3], false);
                }
            } else {
                OPENAL_SetPaused(channels[music1], true);
                OPENAL_SetPaused(channels[stream_music2], true);
                OPENAL_SetPaused(channels[stream_music3], true);

                for(i = 0; i < 4; i++) {
                    oldmusicvolume[i] = 0;
                    musicvolume[i] = 0;
                }
            }

            if(config.sound_music) {
                if(musicvolume[0] <= 0 && oldmusicvolume[0] > 0) {
                    OPENAL_SetPaused(channels[music1], true);
                }
                if(musicvolume[1] <= 0 && oldmusicvolume[1] > 0) {
                    OPENAL_SetPaused(channels[stream_music2], true);
                }
                if(musicvolume[2] <= 0 && oldmusicvolume[2] > 0) {
                    OPENAL_SetPaused(channels[stream_music3], true);
                }

                if(musicvolume[0] != oldmusicvolume[0]) {
                    OPENAL_SetVolume(channels[music1], musicvolume[0]);
                }
                if(musicvolume[1] != oldmusicvolume[1]) {
                    OPENAL_SetVolume(channels[stream_music2], musicvolume[1]);
                }
                if(musicvolume[2] != oldmusicvolume[2]) {
                    OPENAL_SetVolume(channels[stream_music3], musicvolume[2]);
                }

                for(i = 0; i < 3; i++) {
                    oldmusicvolume[i] = musicvolume[i];
                }
            }

            killhotspot = 2;
            if(numhotspots)
                for(i = 0; i < numhotspots; i++) {
                    if(hotspottype[i] > 10 && hotspottype[i] < 20) {
                        if(player[hotspottype[i] - 10].dead == 0) {
                            killhotspot = 0;
                        } else if(killhotspot == 2)
                            killhotspot = 1;
                    }
                }
            if(killhotspot == 2)
                killhotspot = 0;

            winhotspot = 0;
            if(numhotspots)
                for(i = 0; i < numhotspots; i++) {
                    if(hotspottype[i] == -1) {
                        if(findDistancefast(&player[0].coords, &hotspot[i]) < hotspotsize[i])
                            winhotspot = 1;
                    }
                }

            int numalarmed = 0;
            if(numplayers > 1)
                for(i = 1; i < numplayers; i++) {
                    if(!player[i].dead && player[i].aitype == attacktypecutoff && player[i].surprised <= 0)
                        numalarmed++;
                }
            if(numalarmed > maxalarmed)
                maxalarmed = numalarmed;

            if(changedelay <= 0 && !loading && !editorenabled && gameon && !tutoriallevel && changedelay != -999 && !won) {
                if(player[0].dead && changedelay <= 0) {
                    changedelay = 1;
                    targetlevel = whichlevel;
                }
                alldead = 1;
                if(numplayers > 1)
                    for(i = 1; i < numplayers; i++) {
                        if(!player[i].dead && player[i].howactive < typedead1)
                            alldead = 0;
                    }

                if(alldead && !player[0].dead && maptype == mapkilleveryone) {
                    changedelay = 1;
                    targetlevel = whichlevel + 1;
                    if(targetlevel > numchallengelevels - 1)
                        targetlevel = 0;
                }
                if(winhotspot || windialogue) {
                    changedelay = 0.1;
                    targetlevel = whichlevel + 1;
                    if(targetlevel > numchallengelevels - 1)
                        targetlevel = 0;
                }

                if(killhotspot) {
                    changedelay = 1;
                    targetlevel = whichlevel + 1;
                    if(targetlevel > numchallengelevels - 1)
                        targetlevel = 0;
                }

                if(changedelay > 0 && !player[0].dead && !won) {
                    // high scores, awards, win
                    if(campaign) {
                        won = 1;
                        accountcampaignchoices[accountactive][accountcampaignchoicesmade[accountactive]] = whichchoice;
                        accountcampaignchoicesmade[accountactive]++;
                        accountcampaignscore[accountactive] += bonustotal;
                        scoreadded = 1;
                        accountcampaigntime[accountactive] += leveltime;
                        if(accountcampaignscore[accountactive] > accountcampaignhighscore[accountactive])
                            accountcampaignhighscore[accountactive] = accountcampaignscore[accountactive];

                        // if(accountprogress[accountactive]<whichlevel+1)accountprogress[accountactive]=whichlevel+1;
                    } else {
                        won = 1;
                        if(!config.game_debug) {
                            if(bonustotal - startbonustotal > accounthighscore[accountactive][whichlevel])
                                accounthighscore[accountactive][whichlevel] = bonustotal - startbonustotal;
                            if(accountfasttime[accountactive][whichlevel] == 0 ||
                               leveltime < accountfasttime[accountactive][whichlevel])
                                accountfasttime[accountactive][whichlevel] = leveltime;
                        }
                        if(accountprogress[accountactive] < whichlevel + 1)
                            accountprogress[accountactive] = whichlevel + 1;
                    }
                }
            }

            if(!winfreeze) {

                if(leveltime < 1) {
                    loading = 0;
                    changedelay = .1;
                    alldead = 0;
                    winhotspot = 0;
                    killhotspot = 0;
                }

                if(!editorenabled && gameon && !mainmenu) {
                    if(changedelay != -999)
                        changedelay -= multiplier / 7;
                    if(player[0].dead)
                        targetlevel = whichlevel;
                    if(loading == 2 && !campaign) {
                        flashr = 1;
                        flashg = 0;
                        flashb = 0;
                        flashamount = 1;
                        flashdelay = 1;
                        loadtime = 0;

                        float gLoc[3] = { 0, 0, 0 };
                        float vel[3] = { 0, 0, 0 };
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                        PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                        OPENAL_SetVolume(channels[firestartsound], 256);
                        OPENAL_SetPaused(channels[firestartsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                        if(!player[0].dead && targetlevel != whichlevel) {
                            startbonustotal = bonustotal;
                        }
                        if(!player[0].dead)
                            Loadlevel(targetlevel);
                        if(player[0].dead)
                            Loadlevel(whichlevel);

                        OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                        PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                        OPENAL_SetVolume(channels[fireendsound], 256);
                        OPENAL_SetPaused(channels[fireendsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                        loading = 3;
                    }
                    if(loading == 2 && targetlevel == whichlevel) {
                        flashr = 1;
                        flashg = 0;
                        flashb = 0;
                        flashamount = 1;
                        flashdelay = 1;
                        loadtime = 0;

                        float gLoc[3] = { 0, 0, 0 };
                        float vel[3] = { 0, 0, 0 };
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                        PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                        OPENAL_SetVolume(channels[firestartsound], 256);
                        OPENAL_SetPaused(channels[firestartsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                        for(i = 0; i < 255; i++) {
                            mapname[i] = '\0';
                        }
                        mapname[0] = ':';
                        mapname[1] = 'D';
                        mapname[2] = 'a';
                        mapname[3] = 't';
                        mapname[4] = 'a';
                        mapname[5] = ':';
                        mapname[6] = 'M';
                        mapname[7] = 'a';
                        mapname[8] = 'p';
                        mapname[9] = 's';
                        mapname[10] = ':';
                        strcat(mapname,
                               campaignmapname[levelorder[accountcampaignchoicesmade
                                                              [accountactive]]]); //[campaignchoicewhich[whichchoice]]);
                        Loadlevel(mapname);

                        OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
                        PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
                        OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
                        OPENAL_SetVolume(channels[fireendsound], 256);
                        OPENAL_SetPaused(channels[fireendsound], false);
                        OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

                        loading = 3;
                    }
                    if(changedelay <= -999 && whichlevel != -2 && !loading &&
                       (player[0].dead || (alldead && maptype == mapkilleveryone) || (winhotspot) || (killhotspot)) &&
                       !winfreeze)
                        loading = 1;
                    if((player[0].dead || (alldead && maptype == mapkilleveryone) || (winhotspot) || (windialogue) ||
                        (killhotspot)) &&
                       changedelay <= 0) {
                        {
                            if(whichlevel != -2 && !loading && !player[0].dead) {
                                winfreeze = 1;
                                changedelay = -999;
                            }
                            if(player[0].dead)
                                loading = 1;
                        }
                    }
                }

                if(campaign) {
                    if(mainmenu == 0 && winfreeze && (campaignchoosenext[campaignchoicewhich[whichchoice]]) == 1) {
                        if(campaignnumnext[campaignchoicewhich[whichchoice]] == 0) {
                            endgame = 1;
                        }
                    } else if(mainmenu == 0 && winfreeze) {
                        if(campaignchoosenext[campaignchoicewhich[whichchoice]] == 2)
                            stealthloading = 1;
                        else
                            stealthloading = 0;

                        if(!stealthloading) {
                            float gLoc[3] = { 0, 0, 0 };
                            float vel[3] = { 0, 0, 0 };
                            OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 9999.0f, 99999.0f);
                            PlaySoundEx(firestartsound, samp[firestartsound], NULL, true);
                            OPENAL_3D_SetAttributes(channels[firestartsound], gLoc, vel);
                            OPENAL_SetVolume(channels[firestartsound], 256);
                            OPENAL_SetPaused(channels[firestartsound], false);
                            OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

                            flashr = 1;
                            flashg = 0;
                            flashb = 0;
                            flashamount = 1;
                            flashdelay = 1;
                        }

                        startbonustotal = 0;

                        ifstream ipstream(locateDataFile("campaigns", "main.txt").c_str());
                        // campaignnumlevels=0;
                        // accountcampaignchoicesmade[accountactive]=0;
                        ipstream.ignore(256, ':');
                        ipstream >> campaignnumlevels;
                        for(i = 0; i < campaignnumlevels; i++) {
                            ipstream.ignore(256, ':');
                            ipstream.ignore(256, ':');
                            ipstream.ignore(256, ' ');
                            ipstream >> campaignmapname[i];
                            ipstream.ignore(256, ':');
                            ipstream >> campaigndescription[i];
                            for(j = 0; j < 256; j++) {
                                if(campaigndescription[i][j] == '_')
                                    campaigndescription[i][j] = ' ';
                            }
                            ipstream.ignore(256, ':');
                            ipstream >> campaignchoosenext[i];
                            ipstream.ignore(256, ':');
                            ipstream >> campaignnumnext[i];
                            if(campaignnumnext[i])
                                for(j = 0; j < campaignnumnext[i]; j++) {
                                    ipstream.ignore(256, ':');
                                    ipstream >> campaignnextlevel[i][j];
                                    campaignnextlevel[i][j] -= 1;
                                }
                            ipstream.ignore(256, ':');
                            ipstream >> campaignlocationx[i];
                            ipstream.ignore(256, ':');
                            ipstream >> campaignlocationy[i];
                        }
                        ipstream.close();

                        for(i = 0; i < campaignnumlevels; i++) {
                            levelvisible[i] = 0;
                            levelhighlight[i] = 0;
                        }

                        for(i = 0; i < campaignnumlevels; i++) {
                            levelvisible[i] = 0;
                            levelhighlight[i] = 0;
                        }

                        levelorder[0] = 0;
                        levelvisible[0] = 1;
                        if(accountcampaignchoicesmade[accountactive])
                            for(i = 0; i < accountcampaignchoicesmade[accountactive]; i++) {
                                levelorder[i + 1] =
                                    campaignnextlevel[levelorder[i]][accountcampaignchoices[accountactive][i]];
                                levelvisible[levelorder[i + 1]] = 1;
                            }
                        int whichlevelstart;
                        whichlevelstart = accountcampaignchoicesmade[accountactive] - 1;
                        if(whichlevelstart < 0) {
                            campaignchoicenum = 1;
                            campaignchoicewhich[0] = 0;
                        } else {
                            campaignchoicenum = campaignnumnext[levelorder[whichlevelstart]];
                            if(campaignchoicenum)
                                for(i = 0; i < campaignchoicenum; i++) {
                                    campaignchoicewhich[i] = campaignnextlevel[levelorder[whichlevelstart]][i];
                                    levelvisible[campaignnextlevel[levelorder[whichlevelstart]][i]] = 1;
                                    levelhighlight[campaignnextlevel[levelorder[whichlevelstart]][i]] = 1;
                                }
                        }

                        loading = 2;
                        loadtime = 0;
                        targetlevel = 7;
                        // if(firstload)TickOnceAfter();
                        if(!firstload)
                            LoadStuff();
                        // else {
                        for(i = 0; i < 255; i++) {
                            mapname[i] = '\0';
                        }
                        mapname[0] = 'm';
                        mapname[1] = 'a';
                        mapname[2] = 'p';
                        mapname[3] = 's';
                        mapname[4] = '/';

                        // accountcampaignchoices[accountactive][accountcampaignchoicesmade[accountactive]]=whichchoice;
                        // accountcampaignchoicesmade[accountactive]++;

                        strcat(mapname, campaignmapname[campaignchoicewhich[0]]);
                        whichchoice = 0;
                        visibleloading = 1;
                        stillloading = 1;
                        Loadlevel(mapname);
                        campaign = 1;
                        mainmenu = 0;
                        gameon = 1;
                        OPENAL_SetPaused(channels[stream_music3], true);

                        stealthloading = 0;
                    }
                }
                if(loading == 3)
                    loading = 0;
            }

            oldmusictype = musictype;
        }

        facing = 0;
        facing.z = -1;

        facing = DoRotation(facing, -rotation2, 0, 0);
        facing = DoRotation(facing, 0, 0 - rotation, 0);
        viewerfacing = facing;

        brotate = 0;
        if(!cameramode) {
            if((animation[player[0].targetanimation].attack != 3 && animation[player[0].currentanimation].attack != 3) ||
               player[0].skeleton.free)
                target = player[0].coords + player[0].currentoffset * (1 - player[0].target) * player[0].scale +
                         player[0].targetoffset * player[0].target * player[0].scale - player[0].facing * .05;
            else
                target = player[0].oldcoords + player[0].currentoffset * (1 - player[0].target) * player[0].scale +
                         player[0].targetoffset * player[0].target * player[0].scale - player[0].facing * .05;
            target.y += .1;
            if(player[0].skeleton.free) {
                for(i = 0; i < player[0].skeleton.num_joints; i++) {
                    if(player[0].skeleton.joints[i].position.y * player[0].scale + player[0].coords.y > target.y)
                        target.y = player[0].skeleton.joints[i].position.y * player[0].scale + player[0].coords.y;
                }
                target.y += .1;
            }
            if(player[0].skeleton.free != 2 && !autocam) {
                cameraspeed = 20;
                if(findLengthfast(&player[0].velocity) > 400) {
                    cameraspeed = 20 + (findLength(&player[0].velocity) - 20) * .96;
                }
                if(player[0].skeleton.free == 0 && player[0].targetanimation != hanganim &&
                   player[0].targetanimation != climbanim)
                    target.y += 1.4;
                coltarget = target - cameraloc;
                if(findLengthfast(&coltarget) < multiplier * multiplier * 400)
                    cameraloc = target;
                else {
                    Normalise(&coltarget);
                    if(player[0].targetanimation != hanganim && player[0].targetanimation != climbanim &&
                       player[0].currentanimation != climbanim && player[0].currentoffset.x == 0)
                        cameraloc = cameraloc + coltarget * multiplier * cameraspeed;
                    else
                        cameraloc = cameraloc + coltarget * multiplier * 8;
                }
                if(editorenabled)
                    cameraloc = target;
                cameradist += multiplier * 5;
                if(cameradist > 2.3)
                    cameradist = 2.3;
                viewer = cameraloc - facing * cameradist;
                colviewer = viewer;
                coltarget = cameraloc;
                objects.SphereCheckPossible(&colviewer, findDistance(&colviewer, &coltarget));
                if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                    for(j = 0; j < terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz]; j++) {
                        i = terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                        colviewer = viewer;
                        coltarget = cameraloc;
                        if(objects.model[i].LineCheckPossible(
                               &colviewer, &coltarget, &col, &objects.position[i], &objects.rotation[i]) != -1)
                            viewer = col;
                    }
                if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                    for(j = 0; j < terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz]; j++) {
                        i = terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                        colviewer = viewer;
                        if(objects.model[i].SphereCheck(
                               &colviewer, .15, &col, &objects.position[i], &objects.rotation[i]) != -1) {
                            viewer = colviewer;
                        }
                    }
                cameradist = findDistance(&viewer, &target);
                if(viewer.y < terrain.getHeight(viewer.x, viewer.z) + .6) {
                    viewer.y = terrain.getHeight(viewer.x, viewer.z) + .6;
                }
                if(cameraloc.y < terrain.getHeight(cameraloc.x, cameraloc.z)) {
                    cameraloc.y = terrain.getHeight(cameraloc.x, cameraloc.z);
                }
            }
            if(player[0].skeleton.free != 2 && autocam) {
                cameraspeed = 20;
                if(findLengthfast(&player[0].velocity) > 400) {
                    cameraspeed = 20 + (findLength(&player[0].velocity) - 20) * .96;
                }
                if(player[0].skeleton.free == 0 && player[0].targetanimation != hanganim &&
                   player[0].targetanimation != climbanim)
                    target.y += 1.4;
                cameradist += multiplier * 5;
                if(cameradist > 3.3)
                    cameradist = 3.3;
                coltarget = target - cameraloc;
                if(findLengthfast(&coltarget) < multiplier * multiplier * 400)
                    cameraloc = target;
                else if(findLengthfast(&coltarget) > 1) {
                    Normalise(&coltarget);
                    if(player[0].targetanimation != hanganim && player[0].targetanimation != climbanim &&
                       player[0].currentanimation != climbanim && player[0].currentoffset.x == 0)
                        cameraloc = cameraloc + coltarget * multiplier * cameraspeed;
                    else
                        cameraloc = cameraloc + coltarget * multiplier * 8;
                }
                if(editorenabled)
                    cameraloc = target;
                viewer = cameraloc;
                colviewer = viewer;
                coltarget = cameraloc;
                objects.SphereCheckPossible(&colviewer, findDistance(&colviewer, &coltarget));
                if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                    for(j = 0; j < terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz]; j++) {
                        i = terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                        colviewer = viewer;
                        coltarget = cameraloc;
                        if(objects.model[i].LineCheckPossible(
                               &colviewer, &coltarget, &col, &objects.position[i], &objects.rotation[i]) != -1)
                            viewer = col;
                    }
                if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                    for(j = 0; j < terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz]; j++) {
                        i = terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                        colviewer = viewer;
                        if(objects.model[i].SphereCheck(
                               &colviewer, .15, &col, &objects.position[i], &objects.rotation[i]) != -1) {
                            viewer = colviewer;
                        }
                    }
                cameradist = findDistance(&viewer, &target);
                if(viewer.y < terrain.getHeight(viewer.x, viewer.z) + .6) {
                    viewer.y = terrain.getHeight(viewer.x, viewer.z) + .6;
                }
                if(cameraloc.y < terrain.getHeight(cameraloc.x, cameraloc.z)) {
                    cameraloc.y = terrain.getHeight(cameraloc.x, cameraloc.z);
                }
            }
            if(camerashake > .8)
                camerashake = .8;
            // if(woozy>10)woozy=10;
            // woozy+=multiplier;
            woozy += multiplier;
            if(player[0].dead)
                camerashake = 0;
            if(player[0].dead)
                woozy = 0;
            camerashake -= multiplier * 2;
            blackout -= multiplier * 2;
            // if(player[0].isCrouch())woozy-=multiplier*8;
            if(camerashake < 0)
                camerashake = 0;
            if(blackout < 0)
                blackout = 0;
            // if(woozy<0)woozy=0;
            if(camerashake) {
                viewer.x += (float)(rand() % 100) * .0005 * camerashake;
                viewer.y += (float)(rand() % 100) * .0005 * camerashake;
                viewer.z += (float)(rand() % 100) * .0005 * camerashake;
            }
        }
    }
}
