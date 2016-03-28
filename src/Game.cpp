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

#include "FileIO.h"
#include "Globals.h"

extern "C" void PlayStreamEx(int chan, OPENAL_STREAM* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused);

namespace Ragaru
{
    Game::TextureList Game::textures;

    Game::Game()
    {
        campaignchoicesmade = 0;
        terraintexture      = 0;
        terraintexture2     = 0;
        terraintexture3     = 0;
        screentexture       = 0;
        screentexture2      = 0;
        logotexture         = 0;
        loadscreentexture   = 0;
        Maparrowtexture     = 0;
        Mapboxtexture       = 0;
        Mapcircletexture    = 0;
        cursortexture       = 0;
        screenWidth         = 0;
        screenHeight        = 0;
        textureSize         = 0;

        memset(Mainmenuitems, 0, sizeof(Mainmenuitems));

        nummenuitems = 0;

        memset(startx, 0, sizeof(startx));
        memset(starty, 0, sizeof(starty));
        memset(endx, 0, sizeof(endx));
        memset(endy, 0, sizeof(endy));

        memset(selectedlong, 0, sizeof(selectedlong));
        memset(offsetx, 0, sizeof(offsetx));
        memset(offsety, 0, sizeof(offsety));
        memset(movex, 0, sizeof(movex));
        memset(movey, 0, sizeof(movey));
        memset(endy, 0, sizeof(endy));

        transition = 0;
        anim = 0;
        selected = 0;
        loaddistrib = 0;
        keyselect = 0;
        indemo = 0;

        won = 0;

        entername = 0;

        memset(menustring, 0, sizeof(menustring));
        memset(registrationname, 0, sizeof(registrationname));
        registrationnumber = 0;

        newdetail = 0;
        newscreenwidth = 0;
        newscreenheight = 0;

        gameon = 0;
        deltah = 0,deltav = 0;
        mousecoordh = 0,mousecoordv = 0;
        oldmousecoordh = 0,oldmousecoordv = 0;
        rotation = 0,rotation2 = 0;

        // SkyBox skybox;

        cameramode = 0;
        cameratogglekeydown = 0;
        chattogglekeydown = 0;
        olddrawmode = 0;
        drawmode = 0;
        drawmodetogglekeydown = 0;
        explodetogglekeydown = 0;
        detailtogglekeydown = 0;
        firstload = 0;
        oldbutton = 0;

        leveltime = 0;
        loadtime = 0;

        // Model hawk;

        //  XYZ hawkcoords;
        //  XYZ realhawkcoords;

        hawktexture = 0;
        hawkrotation = 0;
        hawkcalldelay = 0;

        stealthloading = 0;

        campaignnumlevels = 0;

        memset(campaignmapname, 0, sizeof(campaignmapname));
        memset(campaigndescription, 0, sizeof(campaigndescription));
        memset(campaignchoosenext, 0, sizeof(campaignchoosenext));
        memset(campaignnumnext, 0, sizeof(campaignnumnext));
        memset(campaignnextlevel, 0, sizeof(campaignnextlevel));
        memset(campaignchoices, 0, sizeof(campaignchoices));
        memset(campaignlocationx, 0, sizeof(campaignlocationx));
        memset(campaignlocationy, 0, sizeof(campaignlocationy));
        memset(campaignlocationy, 0, sizeof(campaignlocationy));

        campaignchoicenum = 0;

        memset(campaignchoicewhich, 0, sizeof(campaignchoicewhich));

        whichchoice = 0;

        numlevelspassed = 0;

        memset(levelorder, 0, sizeof(levelorder));
        memset(levelvisible, 0, sizeof(levelvisible));
        memset(levelhighlight, 0, sizeof(levelhighlight));

        minimap = 0;

        musictype = 0,oldmusictype = 0,oldoldmusictype = 0;
        realthreat = 0;

        // Model rabbit;
        // - XYZ rabbitcoords;
        // - XYZ mapcenter;
        mapradius = 0;

        // Text text;
        fps = 0;

        // XYZ cameraloc;
        cameradist = 0;

        envtogglekeydown = 0;
        slomotogglekeydown = 0;
        texturesizetogglekeydown = 0;
        freezetogglekeydown = 0;
        drawtoggle = 0;

        editorenabled = 0;
        editortype = 0;
        editorsize = 0;
        editorrotation = 0;
        editorrotation2 = 0;

        brightness = 0;

        quit = 0;
        tryquit = 0;

        // XYZ pathpoint[30];
        numpathpoints = 0;
        memset(numpathpointconnect, 0, sizeof(numpathpointconnect));
        memset(pathpointconnect, 0, sizeof(pathpointconnect));
        pathpointselected = 0;

        endgame = 0;
        scoreadded = 0;
        numchallengelevels = 0;

        console = 0;
        archiveselected = 0;

        memset(consoletext, 0, sizeof(consoletext));
        memset(consolechars, 0, sizeof(consolechars));
        chatting = 0;
        memset(displaytext, 0, sizeof(displaytext));
        memset(displaychars, 0, sizeof(displaychars));
        memset(displaytime, 0, sizeof(displaytime));
        displayblinkdelay = 0;
        displayblink = 0;
        displayselected = 0;
        consolekeydown = 0;
        consoletogglekeydown = 0;
        consoleblinkdelay = 0;
        consoleblink = 0;
        consoleselected = 0;
        memset(togglekey, 0, sizeof(togglekey));
        memset(togglekeydelay, 0, sizeof(togglekeydelay));
        registernow = 0;
        autocam = 0;

        keys.crouch  = SDL_SCANCODE_UNKNOWN;
        keys.jump    = SDL_SCANCODE_UNKNOWN;
        keys.forward = SDL_SCANCODE_UNKNOWN;
        keys.chat    = SDL_SCANCODE_UNKNOWN;
        keys.back    = SDL_SCANCODE_UNKNOWN;
        keys.left    = SDL_SCANCODE_UNKNOWN;
        keys.right   = SDL_SCANCODE_UNKNOWN;
        keys.draw    = SDL_SCANCODE_UNKNOWN;
        keys.throww  = SDL_SCANCODE_UNKNOWN;

        keys.attack    = 0;
        keys.oldattack = 0;

        loading = 0;
        talkdelay = 0;

        numboundaries = 0;

        whichlevel = 0;
        oldenvironment = 0;
        targetlevel = 0;
        changedelay = 0;

        memset(musicvolume, 0, sizeof(musicvolume));
        memset(oldmusicvolume, 0, sizeof(oldmusicvolume));
        musicselected = 0;
        change = 0;
    }

    void Game::InitGame()
    {
        autocam = 0;
        numchallengelevels = 14;

        accountactive = -1;

        std::string usersfile = locateConfigFile("users");

        FILE* tfile = fopen(usersfile.c_str(), "rb");

        if (tfile)
        {
            funpackf(tfile, "Bi", &numaccounts);
            funpackf(tfile, "Bi", &accountactive);

            if (numaccounts > 0)
            {
                for (int i = 0; i < numaccounts; i++)
                {
                    funpackf(tfile, "Bf", &accountcampaigntime[i]);
                    funpackf(tfile, "Bf", &accountcampaignscore[i]);
                    funpackf(tfile, "Bf", &accountcampaignfasttime[i]);
                    funpackf(tfile, "Bf", &accountcampaignhighscore[i]);
                    funpackf(tfile, "Bi", &accountdifficulty[i]);
                    funpackf(tfile, "Bi", &accountprogress[i]);
                    funpackf(tfile, "Bi", &accountcampaignchoicesmade[i]);

                    for (int j = 0;j < accountcampaignchoicesmade[i]; j++)
                    {
                        funpackf(tfile, "Bi", &accountcampaignchoices[i][j]);
                        if (accountcampaignchoices[i][j] >= 10)
                        {
                            accountcampaignchoices[i][j] = 0;
                        }
                    }
                    funpackf(tfile, "Bf", &accountpoints[i]);

                    for (int j = 0; j < 50; j++)
                    {
                        funpackf(tfile, "Bf", &accounthighscore[i][j]);
                        funpackf(tfile, "Bf", &accountfasttime[i][j]);
                    }

                    for (int j = 0; j < 60; j++)
                    {
                        funpackf(tfile, "Bb",  &accountunlocked[i][j]);
                    }

                    int temp;
                    funpackf(tfile, "Bi",  &temp);
                    if (temp > 0)
                    {
                        for (int j = 0;j < temp; j++)
                        {
                            funpackf(tfile, "Bb",  &accountname[i][j]);
                        }
                    }
                }
            }
            fclose(tfile);
        }

        tintr = 1;
        tintg = 1;
        tintb = 1;

        whichjointstartarray[0]  = righthip;
        whichjointendarray[0]    = rightfoot;

        whichjointstartarray[1]  = righthip;
        whichjointendarray[1]    = rightankle;

        whichjointstartarray[2]  = righthip;
        whichjointendarray[2]    = rightknee;

        whichjointstartarray[3]  = rightknee;
        whichjointendarray[3]    = rightankle;

        whichjointstartarray[4]  = rightankle;
        whichjointendarray[4]    = rightfoot;

        whichjointstartarray[5]  = lefthip;
        whichjointendarray[5]    = leftfoot;

        whichjointstartarray[6]  = lefthip;
        whichjointendarray[6]    = leftankle;

        whichjointstartarray[7]  = lefthip;
        whichjointendarray[7]    = leftknee;

        whichjointstartarray[8]  = leftknee;
        whichjointendarray[8]    = leftankle;

        whichjointstartarray[9]  = leftankle;
        whichjointendarray[9]    = leftfoot;

        whichjointstartarray[10] = abdomen;
        whichjointendarray[10]   = rightshoulder;

        whichjointstartarray[11] = abdomen;
        whichjointendarray[11]   = rightelbow;

        whichjointstartarray[12] = abdomen;
        whichjointendarray[12]   = rightwrist;

        whichjointstartarray[13] = abdomen;
        whichjointendarray[13]   = righthand;

        whichjointstartarray[14] = rightshoulder;
        whichjointendarray[14]   = rightelbow;

        whichjointstartarray[15] = rightelbow;
        whichjointendarray[15]   = rightwrist;

        whichjointstartarray[16] = rightwrist;
        whichjointendarray[16]   = righthand;

        whichjointstartarray[17] = abdomen;
        whichjointendarray[17]   = leftshoulder;

        whichjointstartarray[18] = abdomen;
        whichjointendarray[18]   = leftelbow;

        whichjointstartarray[19] = abdomen;
        whichjointendarray[19]   = leftwrist;

        whichjointstartarray[20] = abdomen;
        whichjointendarray[20]   = lefthand;

        whichjointstartarray[21] = leftshoulder;
        whichjointendarray[21]   = leftelbow;

        whichjointstartarray[22] = leftelbow;
        whichjointendarray[22]   = leftwrist;

        whichjointstartarray[23] = leftwrist;
        whichjointendarray[23]   = lefthand;

        whichjointstartarray[24] = abdomen;
        whichjointendarray[24]   = neck;

        whichjointstartarray[25] = neck;
        whichjointendarray[25]   = head;

        FadeLoadingScreen(0);

        stillloading = 1;

        texture.data = (GLubyte*) malloc(1024 * 1024 * 4);

        int temptexdetail = texdetail;
        texdetail = 1;
        text.LoadFontTexture("font.png");
        text.BuildFont();
        texdetail = temptexdetail;

        FadeLoadingScreen(10);

        if (config.video_detail == 2)
        {
            texdetail     = 1;
            terraindetail = 1;
        }
        if (config.video_detail == 1)
        {
            texdetail     = 2;
            terraindetail = 1;
        }
        if (config.video_detail == 0)
        {
            texdetail     = 4;
            terraindetail = 1;
        }

        for (int it = 0; it < 100; ++it)
        {
            channels[it] = -1;
            samp[it] = NULL;
        }
        for (int it = 0; it < 20; ++it)
        {
            strm[it] = NULL;
        }

        LOG->LOG("Initializing sound system...");


        OPENAL_SetOutput(OPENAL_OUTPUT_ALSA);

        // OPENAL_Init(44100, 32, 0);

        OPENAL_SetSFXMasterVolume((float) config.sound_volume);

        strm[stream_music3] = OPENAL_Stream_Open(locateDataFile("sounds", "music3.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 0;
        }
        OPENAL_Stream_SetMode(strm[stream_music3], OPENAL_LOOP_NORMAL);

        if (config.sound_music)
        {
            PlayStreamEx(stream_music3, strm[stream_music3], 0, true);
            OPENAL_SetPaused(channels[stream_music3], false);
            OPENAL_SetVolume(channels[stream_music3], 256);
        }

        FadeLoadingScreen(20);

        if (config.sound_ambient)
        {
            strm[stream_wind] = OPENAL_Stream_Open(locateDataFile("sounds", "wind.ogg"), OPENAL_2D, 0, 0);
            if (visibleloading)
            {
                LoadingScreen();
                loadscreencolor = 5;
            }
            OPENAL_Stream_SetMode(strm[stream_wind], OPENAL_LOOP_NORMAL);

            FadeLoadingScreen(30);

            strm[stream_desertambient] = OPENAL_Stream_Open(locateDataFile("sounds", "desertambient.ogg"), OPENAL_2D, 0, 0);
            if (visibleloading)
            {
                LoadingScreen();
                loadscreencolor = 5;
            }
            OPENAL_Stream_SetMode(strm[stream_desertambient], OPENAL_LOOP_NORMAL);
        }

        FadeLoadingScreen(40);

        samp[firestartsound] = OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "firestart.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 5;
        }
        OPENAL_Sample_SetMinMaxDistance(samp[firestartsound], 8.0f, 2000.0f);

        strm[stream_firesound] = OPENAL_Stream_Open(locateDataFile("sounds", "fire.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading) {
            LoadingScreen();
            loadscreencolor = 5;
        }
        OPENAL_Stream_SetMode(strm[stream_firesound], OPENAL_LOOP_NORMAL);

        FadeLoadingScreen(50);

        samp[fireendsound] = OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", "fireend.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 5;
        }
        OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);

        strm[stream_music1grass] = OPENAL_Stream_Open(locateDataFile("sounds", "music1grass.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 1;
        }
        OPENAL_Stream_SetMode(strm[stream_music1grass], OPENAL_LOOP_NORMAL);

        strm[stream_music1snow] = OPENAL_Stream_Open(locateDataFile("sounds", "music1snow.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 2;
        }
        OPENAL_Stream_SetMode(strm[stream_music1snow], OPENAL_LOOP_NORMAL);

        FadeLoadingScreen(60);

        strm[stream_music1desert] = OPENAL_Stream_Open(locateDataFile("sounds", "music1desert.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 3;
        }
        OPENAL_Stream_SetMode(strm[stream_music1desert], OPENAL_LOOP_NORMAL);

        FadeLoadingScreen(80);
        strm[stream_music2] = OPENAL_Stream_Open(locateDataFile("sounds", "music2.ogg"), OPENAL_2D, 0, 0);
        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 4;
        }
        OPENAL_Stream_SetMode(strm[stream_music2], OPENAL_LOOP_NORMAL);

        FadeLoadingScreen(90);


        LoadTexture("cursor.png",    &cursortexture,    0, true);
        LoadTexture("mapcircle.png", &Mapcircletexture, 0, true);
        LoadTexture("mapbox.png",    &Mapboxtexture,    0, true);
        LoadTexture("maparrow.png",  &Maparrowtexture,  0, true);

        temptexdetail = texdetail;
        if (texdetail > 2) texdetail = 2;

        LoadTexture("lugaru.png",  &Mainmenuitems[0], 0, false);
        LoadTexture("newgame.png", &Mainmenuitems[1], 0, false);
        LoadTexture("options.png", &Mainmenuitems[2], 0, false);
        LoadTexture("quit.png",    &Mainmenuitems[3], 0, false);
        LoadTexture("world.png",   &Mainmenuitems[7], 0, false);
        LoadTexture("eyelid.png",  &Mainmenuitems[4], 0, true);

        texdetail = temptexdetail;

        loaddistrib = 0;
        anim        = 0;

        FadeLoadingScreen(95);

        gameon   = 0;
        mainmenu = 1;

        stillloading = 0;
        firstload    = 0;
        oldmainmenu  = 0;

        newdetail       = config.video_detail;
        newscreenwidth  = screenWidth;
        newscreenheight = screenHeight;
    }

    void Game::Dispose()
    {
        if (endgame == 2)
        {
            accountcampaignchoicesmade[accountactive] = 0;
            accountcampaignscore[accountactive]       = 0;
            accountcampaigntime[accountactive]        = 0;
            endgame                                   = 0;
        }


        std::string usersfile = locateConfigFile("users");

        FILE* tfile;
        tfile = fopen(usersfile.c_str(), "wb");

        if (tfile)
        {
            fpackf(tfile, "Bi", numaccounts);
            fpackf(tfile, "Bi", accountactive);

            if (numaccounts > 0)
            {
                for (int i = 0; i < numaccounts; i++)
                {
                    fpackf(tfile, "Bf", accountcampaigntime[i]);
                    fpackf(tfile, "Bf", accountcampaignscore[i]);
                    fpackf(tfile, "Bf", accountcampaignfasttime[i]);
                    fpackf(tfile, "Bf", accountcampaignhighscore[i]);
                    fpackf(tfile, "Bi", accountdifficulty[i]);
                    fpackf(tfile, "Bi", accountprogress[i]);
                    fpackf(tfile, "Bi", accountcampaignchoicesmade[i]);

                    for(int j = 0; j < accountcampaignchoicesmade[i]; j++)
                    {
                        fpackf(tfile, "Bi", accountcampaignchoices[i][j]);
                    }
                    fpackf(tfile, "Bf", accountpoints[i]);

                    for (int j = 0; j < 50; j++)
                    {
                        fpackf(tfile, "Bf", accounthighscore[i][j]);
                        fpackf(tfile, "Bf", accountfasttime[i][j]);
                    }

                    for (int j = 0; j < 60; j++)
                    {
                        fpackf(tfile, "Bb",  accountunlocked[i][j]);
                    }
                    fpackf(tfile, "Bi",  strlen(accountname[i]));

                    if (strlen(accountname[i]) > 0)
                    {
                        for (int j = 0; j < (int) strlen(accountname[i]); j++)
                        {
                            fpackf(tfile, "Bb",  accountname[i][j]);
                        }
                    }
                }
            }
            fclose(tfile);
        }

        TexIter it = textures.begin();
        for (; it != textures.end(); ++it)
        {
            if (glIsTexture(it->second)) glDeleteTextures(1, &it->second);
        }
        textures.clear();

        LOG->LOG("Shutting down sound system...");

        OPENAL_StopSound(OPENAL_ALL);

        for (int i = 0; i < 100; ++i)
        {
            OPENAL_Sample_Free(samp[i]);
        }

        for (int i = 0; i < 20; ++i)
        {
            OPENAL_Stream_Close(strm[i]);
        }

        OPENAL_Close();

        if (texture.data)
        {
            free(texture.data);
        }
        texture.data = 0;
    }

    void Game::LoadSound(const int sample, const char* filename, float min, float max)
    {
        LOG->DBG("Loading sound '%s' ...", filename);

        samp[sample] = OPENAL_Sample_Load(OPENAL_FREE, locateDataFile("sounds", filename), OPENAL_HW3D, 0, 0);

        if (visibleloading)
        {
            LoadingScreen();
            loadscreencolor = 5;
        }

        OPENAL_Sample_SetMinMaxDistance(samp[sample], min, max);
    }

    void Game::LoadSounds()
    {
        LOG->LOG("Loading sounds...");

        OPENAL_3D_SetDopplerFactor(0);
        OPENAL_SetSFXMasterVolume((float) config.sound_volume / 100.0f);

        LoadSound(footstepsound,        "footstepsnow1.ogg",     4.0f, 1000.0f);
        LoadSound(footstepsound2,       "footstepsnow2.ogg",     4.0f, 1000.0f);
        LoadSound(footstepsound3,       "footstepstone1.ogg",    4.0f, 1000.0f);
        LoadSound(footstepsound4,       "footstepstone2.ogg",    4.0f, 1000.0f);
        LoadSound(landsound,            "land.ogg",              4.0f, 1000.0f);
        LoadSound(jumpsound,            "jump.ogg",              4.0f, 1000.0f);
        LoadSound(hawksound,            "hawk.ogg",              4.0f, 1000.0f);
        LoadSound(whooshsound,          "whoosh.ogg",            4.0f, 1000.0f);
        LoadSound(landsound1,           "land1.ogg",             4.0f, 1000.0f);
        LoadSound(landsound2,           "land2.ogg",             4.0f, 1000.0f);
        LoadSound(breaksound,           "broken.ogg",            8.0f, 2000.0f);
        LoadSound(lowwhooshsound,       "lowwhoosh.ogg",         8.0f, 2000.0f);
        LoadSound(midwhooshsound,       "midwhoosh.ogg",         8.0f, 2000.0f);
        LoadSound(highwhooshsound,      "highwhoosh.ogg",        8.0f, 2000.0f);
        LoadSound(movewhooshsound,      "movewhoosh.ogg",        8.0f, 2000.0f);
        LoadSound(heavyimpactsound,     "heavyimpact.ogg",       8.0f, 2000.0f);
        LoadSound(whooshhitsound,       "whooshhit.ogg",         8.0f, 2000.0f);
        LoadSound(thudsound,            "thud.ogg",              8.0f, 2000.0f);
        LoadSound(alarmsound,           "alarm.ogg",             8.0f, 2000.0f);
        LoadSound(breaksound2,          "break.ogg",             8.0f, 2000.0f);
        LoadSound(knifedrawsound,       "knifedraw.ogg",         8.0f, 2000.0f);
        LoadSound(knifesheathesound,    "knifesheathe.ogg",      8.0f, 2000.0f);
        LoadSound(fleshstabsound,       "fleshstab.ogg",         8.0f, 2000.0f);
        LoadSound(fleshstabremovesound, "fleshstabremove.ogg",   8.0f, 2000.0f);
        LoadSound(knifeswishsound,      "knifeswish.ogg",        8.0f, 2000.0f);
        LoadSound(knifeslicesound,      "knifeslice.ogg",        8.0f, 2000.0f);
        LoadSound(swordslicesound,      "swordslice.ogg",        8.0f, 2000.0f);
        LoadSound(skidsound,            "skid.ogg",              8.0f, 2000.0f);
        LoadSound(snowskidsound,        "snowskid.ogg",          8.0f, 2000.0f);
        LoadSound(bushrustle,           "bushrustle.ogg",        4.0f, 1000.0f);
        LoadSound(clank1sound,          "clank1.ogg",            8.0f, 2000.0f);
        LoadSound(clank2sound,          "clank2.ogg",            8.0f, 2000.0f);
        LoadSound(clank3sound,          "clank3.ogg",            8.0f, 2000.0f);
        LoadSound(clank4sound,          "clank4.ogg",            8.0f, 2000.0f);
        LoadSound(consolesuccesssound,  "consolesuccess.ogg",    4.0f, 1000.0f);
        LoadSound(consolefailsound,     "consolefail.ogg",       4.0f, 1000.0f);
        LoadSound(metalhitsound,        "metalhit.ogg",          8.0f, 2000.0f);
        LoadSound(clawslicesound,       "clawslice.ogg",         8.0f, 2000.0f);
        LoadSound(splattersound,        "splatter.ogg",          8.0f, 2000.0f);
        LoadSound(growlsound,           "growl.ogg",          1000.0f, 2000.0f);
        LoadSound(growl2sound,          "growl2.ogg",         1000.0f, 2000.0f);
        LoadSound(barksound,            "bark.ogg",           1000.0f, 2000.0f);
        LoadSound(bark2sound,           "bark2.ogg",          1000.0f, 2000.0f);
        LoadSound(bark3sound,           "bark3.ogg",          1000.0f, 2000.0f);
        LoadSound(snarlsound,           "snarl.ogg",          1000.0f, 2000.0f);
        LoadSound(snarl2sound,          "snarl2.ogg",         1000.0f, 2000.0f);
        LoadSound(barkgrowlsound,       "barkgrowl.ogg",      1000.0f, 2000.0f);
        LoadSound(rabbitattacksound,    "rabbitattack.ogg",   1000.0f, 2000.0f);
        LoadSound(rabbitattack2sound,   "rabbitattack2.ogg",  1000.0f, 2000.0f);
        LoadSound(rabbitattack3sound,   "rabbitattack3.ogg",  1000.0f, 2000.0f);
        LoadSound(rabbitattack4sound,   "rabbitattack4.ogg",  1000.0f, 2000.0f);
        LoadSound(rabbitpainsound,      "rabbitpain.ogg",     1000.0f, 2000.0f);
        LoadSound(rabbitpain1sound,     "rabbitpain2.ogg",    1000.0f, 2000.0f);
        LoadSound(rabbitchitter,        "rabbitchitter.ogg",  1000.0f, 2000.0f);
        LoadSound(rabbitchitter2,       "rabbitchitter2.ogg", 1000.0f, 2000.0f);
        LoadSound(swordstaffsound,      "swordstaff.ogg",        8.0f, 2000.0f);
        LoadSound(staffbodysound,       "staffbody.ogg",         8.0f, 2000.0f);
        LoadSound(staffheadsound,       "staffhead.ogg",         8.0f, 2000.0f);
        LoadSound(staffbreaksound,      "staffbreak.ogg",        8.0f, 2000.0f);

        OPENAL_Sample_SetMode(samp[whooshsound], OPENAL_LOOP_NORMAL);
    }

    void Game::LoadTexture(const char* filename, GLuint* textureid, int mipmap, bool hasalpha)
    {
        GLuint type;

        LOG->LOG("Loading texture... %s", filename);

        std::string name = locateDataFile("textures", filename);

        upload_image(name.c_str(), hasalpha);

        // Alpha channel?
        type = texture.bpp == 24 ? GL_RGB : GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (!*textureid) glGenTextures(1, textureid);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, *textureid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (config.video_trilinear) if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if (!config.video_trilinear) if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        if (!mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        gluBuild2DMipmaps(GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data);
    }

    void Game::LoadTextureSave(const char* filename, GLuint* textureid, int mipmap, GLubyte* array, int* skinsize)
    {
        GLuint type;
        int bytesPerPixel;

        LOG->LOG("Loading texture... %s", filename);

        std::string name = locateDataFile("textures", filename);

        upload_image(name.c_str(), 0);

        bytesPerPixel = texture.bpp / 8;

        // Alpha channel?
        type = texture.bpp == 24 ? GL_RGB : GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (!*textureid) glGenTextures(1, textureid);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glBindTexture(GL_TEXTURE_2D, *textureid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (config.video_trilinear) if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if (!config.video_trilinear) if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        if (!mipmap) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        int tempnum = 0;
        for (int i = 0; i < (int)(texture.sizeY * texture.sizeX * bytesPerPixel); i++)
        {
            if ((i + 1) % 4 || type == GL_RGB)
            {
                array[tempnum] = texture.data[i];
                tempnum++;
            }
        }

        *skinsize = texture.sizeX;

        gluBuild2DMipmaps(GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, GL_RGB, GL_UNSIGNED_BYTE, array);
    }

    void Game::LoadSave(const char* filename, GLuint* textureid, bool mipmap, GLubyte* array, int *skinsize)
    {
        int bytesPerPixel;

        LOG->LOG("LoadSave: Loading... %s", filename);

        // Load Image
        float temptexdetail = texdetail;
        texdetail = 1;

        std::string name = locateDataFile("textures", filename);

        upload_image(name.c_str(), 0);

        texdetail = temptexdetail;

        bytesPerPixel = texture.bpp / 8;

        int tempnum = 0;
        for (int i = 0; i < (int)(texture.sizeY * texture.sizeX * bytesPerPixel); i++)
        {
            if ((i + 1) % 4 || bytesPerPixel == 3)
            {
                array[tempnum] = texture.data[i];
                tempnum++;
            }
        }
    }

    void Game::LoadStuff()
    {
        static float temptexdetail;
        static float viewdistdetail;
        float megascale = 1;

        visibleloading = 1;
        loadtime       = 0;
        stillloading   = 1;

        for (int i = 0; i < maxplayers; i++)
        {
            if (glIsTexture(player[i].skeleton.drawmodel.textureptr))
            {
                glDeleteTextures(1, &player[i].skeleton.drawmodel.textureptr);
            }
            player[i].skeleton.drawmodel.textureptr = 0;
        }

        LoadTexture("fire.png", &loadscreentexture, 1, 0);

        temptexdetail = texdetail;
        texdetail     = 1;

        text.LoadFontTexture("font.png");
        text.BuildFont();

        texdetail = temptexdetail;

        numsounds = 71;

        viewdistdetail = 2;
        viewdistance   = 50 * megascale * viewdistdetail;

        brightness = 100;

        if (config.video_detail == 2)
        {
            texdetail     = 1;
            terraindetail = 1;
        }
        if (config.video_detail == 1)
        {
            texdetail     = 2;
            terraindetail = 1;
        }
        if (config.video_detail == 0)
        {
            texdetail     = 4;
            terraindetail = 1;
        }

        realtexdetail = texdetail;

        numplayers = 1;

        LOG->LOG("Loading weapon data...");

        LoadTexture("knife.png",           &weapons.knifetextureptr,           0, 1);
        LoadTexture("bloodknife.png",      &weapons.bloodknifetextureptr,      0, 1);
        LoadTexture("lightbloodknife.png", &weapons.lightbloodknifetextureptr, 0, 1);
        LoadTexture("sword.png",           &weapons.swordtextureptr,           1, 0);
        LoadTexture("swordblood.png",      &weapons.bloodswordtextureptr,      1, 0);
        LoadTexture("swordbloodlight.png", &weapons.lightbloodswordtextureptr, 1, 0);
        LoadTexture("staff.png",           &weapons.stafftextureptr,           1, 0);

        weapons.throwingknifemodel.load("throwingknife.solid", true);
        weapons.throwingknifemodel.Scale(.001, .001, .001);
        weapons.throwingknifemodel.Rotate(90, 0, 0);
        weapons.throwingknifemodel.Rotate(0, 90, 0);
        weapons.throwingknifemodel.flat = 0;
        weapons.throwingknifemodel.CalculateNormals(1);

        weapons.swordmodel.load("sword.solid", true);
        weapons.swordmodel.Scale(.001, .001, .001);
        weapons.swordmodel.Rotate(90, 0, 0);
        weapons.swordmodel.Rotate(0, 90, 0);
        weapons.swordmodel.Rotate(0, 0, 90);
        weapons.swordmodel.flat = 1;
        weapons.swordmodel.CalculateNormals(1);

        weapons.staffmodel.load("staff.solid", true);
        weapons.staffmodel.Scale(.005, .005, .005);
        weapons.staffmodel.Rotate(90, 0, 0);
        weapons.staffmodel.Rotate(0, 90, 0);
        weapons.staffmodel.Rotate(0, 0, 90);
        weapons.staffmodel.flat = 1;
        weapons.staffmodel.CalculateNormals(1);

        LoadTexture("shadow.png",        &terrain.shadowtexture,      0, 1);
        LoadTexture("blood.png",         &terrain.bloodtexture,       0, 1);
        LoadTexture("break.png",         &terrain.breaktexture,       0, 1);
        LoadTexture("blood.png",         &terrain.bloodtexture2,      0, 1);
        LoadTexture("footprint.png",     &terrain.footprinttexture,   0, 1);
        LoadTexture("bodyprint.png",     &terrain.bodyprinttexture,   0, 1);
        LoadTexture("hawk.png",          &hawktexture,                0, 1);
        LoadTexture("logo.png",          &logotexture,                0, 1);
        LoadTexture("cloud.png",         &sprites.cloudtexture,       1, 1);
        LoadTexture("cloudimpact.png",   &sprites.cloudimpacttexture, 1, 1);
        LoadTexture("bloodparticle.png", &sprites.bloodtexture,       1, 1);
        LoadTexture("snowflake.png",     &sprites.snowflaketexture,   1, 1);
        LoadTexture("flame.png",         &sprites.flametexture,       1, 1);
        LoadTexture("bloodflame.png",    &sprites.bloodflametexture,  1, 1);
        LoadTexture("smoke.png",         &sprites.smoketexture,       1, 1);
        LoadTexture("shine.png",         &sprites.shinetexture,       1, 0);
        LoadTexture("splinter.png",      &sprites.splintertexture,    1, 1);
        LoadTexture("leaf.png",          &sprites.leaftexture,        1, 1);
        LoadTexture("tooth.png",         &sprites.toothtexture,       1, 1);

        rotation  = 0;
        rotation2 = 0;

        ReSizeGLScene(90, .01);

        viewer = 0;

        if (config.video_detail == 2) textureSize = 1024;
        if (config.video_detail == 1) textureSize = 512;
        if (config.video_detail == 0) textureSize = 256;

        // Set up distant light
        light.color[0]   = .95;
        light.color[1]   = .95;
        light.color[2]   = 1;
        light.ambient[0] = .2;
        light.ambient[1] = .2;
        light.ambient[2] = .24;
        light.location.x = 1;
        light.location.y = 1;
        light.location.z = -.2;

        Normalise(&light.location);

        LoadingScreen();

        SetUpLighting();

        fadestart = .6;
        gravity   = -10;

        texscale      = .2 / megascale / viewdistdetail;
        terrain.scale = 3 * megascale * terraindetail * viewdistdetail;

        viewer.x = terrain.size / 2 * terrain.scale;
        viewer.z = terrain.size / 2 * terrain.scale;

        hawk.load("hawk.solid", true);
        hawk.Scale(.03, .03, .03);
        hawk.Rotate(90, 1, 1);
        hawk.CalculateNormals(0);
        hawk.ScaleNormals(-1, -1, -1);
        hawkcoords.x = terrain.size / 2 * terrain.scale - 5 - 7;
        hawkcoords.z = terrain.size / 2 * terrain.scale - 5 - 7;
        hawkcoords.y = terrain.getHeight(hawkcoords.x, hawkcoords.z) + 25;

        eye.load("eye.solid", true);
        eye.Scale(.03, .03, .03);
        eye.CalculateNormals(0);

        cornea.load("cornea.solid", true);
        cornea.Scale(.03, .03, .03);
        cornea.CalculateNormals(0);

        iris.load("iris.solid", true);
        iris.Scale(.03, .03, .03);
        iris.CalculateNormals(0);

        LoadSave("bloodfur.png",     0, 1, &bloodText[0],     0);
        LoadSave("wolfbloodfur.png", 0, 1, &wolfbloodText[0], 0);

        oldenvironment = -4;

        gameon    = 1;
        mainmenu  = 0;
        firstload = 0;

        Loadlevel(targetlevel);

        rabbitcoords   = player[0].coords;
        rabbitcoords.y = terrain.getHeight(rabbitcoords.x, rabbitcoords.z);

        animation[runanim].Load("run.anim", middleheight, neutral);

        animation[bounceidleanim].Load("idle.anim", middleheight, neutral);
        animation[stopanim].Load("stop.anim", middleheight, neutral);

        animation[jumpupanim].Load("jumpup.anim", highheight, neutral);
        animation[jumpdownanim].Load("jumpdown.anim", highheight, neutral);

        animation[landanim].Load("landing.anim", lowheight, neutral);
        animation[landhardanim].Load("landhard.anim", lowheight, neutral);
        animation[climbanim].Load("climb.anim", lowheight, neutral);
        animation[hanganim].Load("hangon.anim", lowheight, neutral);
        animation[spinkickanim].Load("spinkick.anim", middleheight, normalattack);

        animation[getupfromfrontanim].Load("getupfromfront.anim", lowheight, neutral);
        animation[getupfrombackanim].Load("getupfromback.anim", lowheight, neutral);
        animation[crouchanim].Load("crouch.anim", lowheight, neutral);
        animation[sneakanim].Load("sneak.anim", lowheight, neutral);
        animation[rollanim].Load("roll.anim", lowheight, neutral);
        animation[flipanim].Load("flip.anim", highheight, neutral);
        animation[frontflipanim].Load("flip.anim", highheight, neutral);
        animation[spinkickreversedanim].Load("spinkickcaught.anim", middleheight, reversed);

        animation[spinkickreversalanim].Load("spinkickcatch.anim", middleheight, reversal);
        animation[lowkickanim].Load("lowkick.anim", middleheight, normalattack);
        animation[sweepanim].Load("sweep.anim", lowheight, normalattack);
        animation[sweepreversedanim].Load("sweepcaught.anim", lowheight, reversed);
        animation[sweepreversalanim].Load("sweepcatch.anim", middleheight, reversal);
        animation[rabbitkickanim].Load("rabbitkick.anim", middleheight, normalattack);
        animation[rabbitkickreversedanim].Load("rabbitkickcaught.anim", middleheight, reversed);
        animation[rabbitkickreversalanim].Load("rabbitkickcatch.anim", lowheight, reversal);
        animation[upunchanim].Load("upunch.anim", middleheight, normalattack);
        animation[staggerbackhighanim].Load("staggerbackhigh.anim", middleheight, neutral);
        animation[upunchreversedanim].Load("upunchcaught.anim", middleheight, reversed);

        animation[upunchreversalanim].Load("upunchcatch.anim", middleheight, reversal);
        animation[hurtidleanim].Load("hurtidle.anim", middleheight, neutral);
        animation[backhandspringanim].Load("backhandspring.anim", middleheight, neutral);
        animation[fightidleanim].Load("fightidle.anim", middleheight, neutral);
        animation[walkanim].Load("walk.anim", middleheight, neutral);

        animation[fightsidestep].Load("fightsidestep.anim", middleheight, neutral);
        animation[killanim].Load("kill.anim", middleheight, normalattack);
        animation[sneakattackanim].Load("sneakattack.anim", middleheight, reversal);
        animation[sneakattackedanim].Load("sneakattacked.anim", middleheight, reversed);
        animation[drawrightanim].Load("drawright.anim", middleheight, neutral);
        animation[knifeslashstartanim].Load("slashstart.anim", middleheight, normalattack);
        animation[crouchdrawrightanim].Load("crouchdrawright.anim", lowheight, neutral);
        animation[crouchstabanim].Load("crouchstab.anim", lowheight, normalattack);

        animation[knifefollowanim].Load("slashfollow.anim", middleheight, reversal);
        animation[knifefollowedanim].Load("slashfollowed.anim", middleheight, reversed);
        animation[knifethrowanim].Load("knifethrow.anim", middleheight, normalattack);
        animation[removeknifeanim].Load("removeknife.anim", middleheight, neutral);
        animation[crouchremoveknifeanim].Load("crouchremoveknife.anim", lowheight, neutral);
        animation[jumpreversedanim].Load("jumpcaught.anim", middleheight, reversed);
        animation[jumpreversalanim].Load("jumpcatch.anim", middleheight, reversal);
        animation[staggerbackhardanim].Load("staggerbackhard.anim", middleheight, neutral);

        animation[dropkickanim].Load("dropkick.anim", middleheight, normalattack);
        animation[winduppunchanim].Load("winduppunch.anim", middleheight, normalattack);
        animation[winduppunchblockedanim].Load("winduppunchblocked.anim", middleheight, normalattack);
        animation[blockhighleftanim].Load("blockhighleft.anim", middleheight, normalattack);
        animation[blockhighleftstrikeanim].Load("blockhighleftstrike.anim", middleheight, normalattack);
        animation[backflipanim].Load("backflip.anim", highheight, neutral);
        animation[walljumpbackanim].Load("walljumpback.anim", highheight, neutral);
        animation[walljumpfrontanim].Load("walljumpfront.anim", highheight, neutral);
        animation[rightflipanim].Load("rightflip.anim", highheight, neutral);
        animation[walljumprightanim].Load("walljumpright.anim", highheight, neutral);
        animation[leftflipanim].Load("leftflip.anim", highheight, neutral);
        animation[walljumpleftanim].Load("walljumpleft.anim", highheight, neutral);
        animation[walljumprightkickanim].Load("walljumprightkick.anim", highheight, neutral);
        animation[walljumpleftkickanim].Load("walljumpleftkick.anim", highheight, neutral);
        animation[knifefightidleanim].Load("knifefightidle.anim", middleheight, neutral);
        animation[knifesneakattackanim].Load("knifesneakattack.anim", middleheight, reversal);
        animation[knifesneakattackedanim].Load("knifesneakattacked.anim", middleheight, reversed);
        animation[swordfightidleanim].Load("swordfightidle.anim", middleheight, neutral);
        animation[drawleftanim].Load("drawleft.anim", middleheight, neutral);
        animation[swordslashanim].Load("swordslash.anim", middleheight, normalattack);
        animation[swordgroundstabanim].Load("swordgroundstab.anim", lowheight, normalattack);
        animation[dodgebackanim].Load("dodgeback.anim", middleheight, neutral);
        animation[swordsneakattackanim].Load("swordsneakattack.anim", middleheight, reversal);
        animation[swordsneakattackedanim].Load("swordsneakattacked.anim", middleheight, reversed);
        animation[swordslashreversedanim].Load("swordslashcaught.anim", middleheight, reversed);
        animation[swordslashreversalanim].Load("swordslashcatch.anim", middleheight, reversal);
        animation[knifeslashreversedanim].Load("knifeslashcaught.anim", middleheight, reversed);
        animation[knifeslashreversalanim].Load("knifeslashcatch.anim", middleheight, reversal);
        animation[swordfightidlebothanim].Load("swordfightidleboth.anim", middleheight, neutral);
        animation[swordslashparryanim].Load("sworduprightparry.anim", middleheight, normalattack);
        animation[swordslashparriedanim].Load("swordslashparried.anim", middleheight, normalattack);
        animation[wolfidle].Load("wolfidle.anim", middleheight, neutral);
        animation[wolfcrouchanim].Load("wolfcrouch.anim", lowheight, neutral);
        animation[wolflandanim].Load("wolflanding.anim", lowheight, neutral);
        animation[wolflandhardanim].Load("wolflandhard.anim", lowheight, neutral);
        animation[wolfrunanim].Load("wolfrun.anim", middleheight, neutral);
        animation[wolfrunninganim].Load("wolfrunning.anim", middleheight, neutral);
        animation[rabbitrunninganim].Load("rabbitrunning.anim", middleheight, neutral);
        animation[wolfstopanim].Load("wolfstop.anim", middleheight, neutral);
        animation[rabbittackleanim].Load("rabbittackle.anim", middleheight, neutral);
        animation[rabbittacklinganim].Load("rabbittackling.anim", middleheight, reversal);
        animation[rabbittackledbackanim].Load("rabbittackledback.anim", middleheight, reversed);
        animation[rabbittackledfrontanim].Load("rabbittackledfront.anim", middleheight, reversed);
        animation[wolfslapanim].Load("wolfslap.anim", middleheight, normalattack);
        animation[staffhitanim].Load("staffhit.anim", middleheight, normalattack);
        animation[staffgroundsmashanim].Load("staffgroundsmash.anim", lowheight, normalattack);
        animation[staffspinhitanim].Load("spinwhack.anim", middleheight, normalattack);
        animation[staffhitreversedanim].Load("staffhitcaught.anim", middleheight, reversed);
        animation[staffhitreversalanim].Load("staffhitcatch.anim", middleheight, reversal);
        animation[staffspinhitreversedanim].Load("spinwhackcaught.anim", middleheight, reversed);
        animation[staffspinhitreversalanim].Load("spinwhackcatch.anim", middleheight, reversal);

        animation[sitanim].Load("sit.anim", lowheight, neutral);
        animation[sleepanim].Load("sleep.anim", lowheight, neutral);
        animation[talkidleanim].Load("talkidle.anim", middleheight, neutral);

        animation[sitwallanim].Load("dying.anim", lowheight, neutral);
        animation[dead1anim].Load("dead1.anim", lowheight, neutral);
        animation[dead2anim].Load("dead2.anim", lowheight, neutral);
        animation[dead3anim].Load("dead3.anim", lowheight, neutral);
        animation[dead4anim].Load("dead4.anim", lowheight, neutral);

        // Fix knife stab, too lazy to do it manually
        XYZ moveamount;
        moveamount   = 0;
        moveamount.z = 2;

        for (int i = 0; i < player[0].skeleton.num_joints; i++)
        {
            for (int j = 0; j < animation[knifesneakattackanim].numframes; j++)
            {
                animation[knifesneakattackanim].position[i][j] += moveamount;
            }
        }

        loadscreencolor = 4;
        LoadingScreen();

        for (int i = 0; i < player[0].skeleton.num_joints; i++)
        {
            for (int j = 0; j < animation[knifesneakattackedanim].numframes; j++)
            {
                animation[knifesneakattackedanim].position[i][j] += moveamount;
            }
        }

        loadscreencolor = 4;
        LoadingScreen();

        for (int i = 0; i < player[0].skeleton.num_joints; i++)
        {
            animation[dead1anim].position[i][1] = animation[dead1anim].position[i][0];
            animation[dead2anim].position[i][1] = animation[dead2anim].position[i][0];
            animation[dead3anim].position[i][1] = animation[dead3anim].position[i][0];
            animation[dead4anim].position[i][1] = animation[dead4anim].position[i][0];
        }

        animation[dead1anim].speed[0] = 0.001;
        animation[dead2anim].speed[0] = 0.001;
        animation[dead3anim].speed[0] = 0.001;
        animation[dead4anim].speed[0] = 0.001;

        animation[dead1anim].speed[1] = 0.001;
        animation[dead2anim].speed[1] = 0.001;
        animation[dead3anim].speed[1] = 0.001;
        animation[dead4anim].speed[1] = 0.001;

        for (int i = 0; i < player[0].skeleton.num_joints; i++)
        {
            for (int j = 0; j < animation[swordsneakattackanim].numframes; j++)
            {
                animation[swordsneakattackanim].position[i][j] += moveamount;
            }
        }

        loadscreencolor = 4;
        LoadingScreen();

        for (int j = 0; j < animation[swordsneakattackanim].numframes; j++)
        {
            animation[swordsneakattackanim].weapontarget[j] += moveamount;
        }

        loadscreencolor = 4;
        LoadingScreen();

        for (int i = 0; i < player[0].skeleton.num_joints; i++)
        {
            for (int j = 0; j < animation[swordsneakattackedanim].numframes; j++)
            {
                animation[swordsneakattackedanim].position[i][j] += moveamount;
            }
        }

        loadscreencolor = 4;
        LoadingScreen();

        temptexdetail = texdetail;
        texdetail     = 1;
        texdetail     = temptexdetail;

        loadscreencolor = 4;
        LoadingScreen();

        if (!screentexture)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glGenTextures(1, &screentexture);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, screentexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, textureSize, textureSize, 0);
        }

        LoadSounds();

        if (targetlevel != 7)
        {
            float gLoc[3] = {0, 0, 0};
            float vel[3]  = {0, 0, 0};

            OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 9999.0f, 99999.0f);
            OPENAL_PlaySoundEx(fireendsound, samp[fireendsound], NULL, true);
            OPENAL_3D_SetAttributes(channels[fireendsound], gLoc, vel);
            OPENAL_SetVolume(channels[fireendsound], 256);
            OPENAL_SetPaused(channels[fireendsound], false);
            OPENAL_Sample_SetMinMaxDistance(samp[fireendsound], 8.0f, 2000.0f);
        }

        stillloading = 0;
        loading      = 0;
        changedelay  = 1;

        visibleloading = 0;
    }

    bool Game::AddClothes(const char* filename, GLuint* textureid, bool mipmap, GLubyte* array, int* skinsize)
    {
        std::string name = locateDataFile("textures", filename);

        // Load Image
        bool opened = upload_image(name.c_str(), 1);

        if (opened)
        {
            int bytesPerPixel;

            if (tintr > 1) tintr = 1;
            if (tintg > 1) tintg = 1;
            if (tintb > 1) tintb = 1;

            if (tintr < 0) tintr = 0;
            if (tintg < 0) tintg = 0;
            if (tintb < 0) tintb = 0;

            bytesPerPixel = texture.bpp / 8;

            int tempnum    = 0;
            float alphanum = 255;

            for (int i = 0; i < (int)(texture.sizeY * texture.sizeX * bytesPerPixel); i++)
            {
                if (bytesPerPixel == 3) alphanum = 255;
                else if ((i + 1) % 4 == 0) alphanum = texture.data[i];

                if ((i + 1) % 4 || bytesPerPixel == 3)
                {
                    if ((i % 4) == 0) texture.data[i] *= tintr;
                    if ((i % 4) == 1) texture.data[i] *= tintg;
                    if ((i % 4) == 2) texture.data[i] *= tintb;

                    array[tempnum] = (float) array[tempnum] * (1 - alphanum / 255) + (float) texture.data[i] * (alphanum / 255);
                    tempnum++;
                }
            }
        }
        else return false;

        return true;
    }

    GLvoid Game::ReSizeGLScene(float fov, float pnear)
    {
        if (screenHeight == 0) screenHeight = 1;

        glViewport(0, 0, screenWidth, screenHeight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(fov, (GLfloat) screenWidth / (GLfloat) screenHeight, pnear, viewdistance);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void Game::LoadingScreen()
    {
        static AbsoluteTime frametime = {0, 0};
        AbsoluteTime currTime = UpTime();
        double deltaTime = (float) AbsoluteDeltaToDuration (currTime, frametime);

        if (deltaTime < 0) deltaTime /= -1000000.0; // if negative microseconds
        else deltaTime /= 1000.0;

        multiplier = deltaTime;

        if (multiplier < .001) multiplier = .001;
        if (multiplier > 10) multiplier = 10;

        if (multiplier > .05)
        {
            static float loadprogress;

            frametime = currTime;   // reset for next time interval

            glLoadIdentity();

            // Clear to black
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            loadtime += multiplier * 4;

            loadprogress = loadtime;
            if (loadprogress > 100) loadprogress = 100;

            // Background
            glEnable(GL_TEXTURE_2D);
            glBindTexture( GL_TEXTURE_2D, loadscreentexture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glDisable(GL_DEPTH_TEST);                               // Disables Depth Testing
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);                            // Select The Projection Matrix
            glPushMatrix();                                         // Store The Projection Matrix
            glLoadIdentity();                                       // Reset The Projection Matrix
            glOrtho(0, screenWidth, 0, screenHeight, -100, 100);    // Set Up An Ortho Screen
            glMatrixMode(GL_MODELVIEW);                             // Select The Modelview Matrix
            glPushMatrix();                                         // Store The Modelview Matrix
            glLoadIdentity();                                       // Reset The Modelview Matrix
            glTranslatef(screenWidth / 2, screenHeight / 2, 0);
            glScalef((float) screenWidth / 2, (float) screenHeight / 2, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_BLEND);
            glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, 1);
            glPushMatrix();
            glBegin(GL_QUADS);
            glTexCoord2f(.1 - loadprogress / 100, 0 + loadprogress / 100 + .3);
            glVertex3f(-1, -1, 0.0f);
            glTexCoord2f(.1 - loadprogress / 100, 0 + loadprogress / 100 + .3);
            glVertex3f(1, -1, 0.0f);
            glTexCoord2f(.1 - loadprogress / 100, 1 + loadprogress / 100 + .3);
            glVertex3f(1,   1, 0.0f);
            glTexCoord2f(.1 - loadprogress / 100, 1 + loadprogress / 100 + .3);
            glVertex3f(-1, 1, 0.0f);
            glEnd();
            glPopMatrix();
            glEnable(GL_BLEND);
            glPushMatrix();
            glBegin(GL_QUADS);
            glTexCoord2f(.4 + loadprogress / 100, 0 + loadprogress / 100);
            glVertex3f(-1, -1, 0.0f);
            glTexCoord2f(.4 + loadprogress / 100, 0 + loadprogress / 100);
            glVertex3f(1, -1, 0.0f);
            glTexCoord2f(.4 + loadprogress / 100, 1 + loadprogress / 100);
            glVertex3f(1, 1, 0.0f);
            glTexCoord2f(.4 + loadprogress / 100, 1 + loadprogress / 100);
            glVertex3f(-1, 1, 0.0f);
            glEnd();
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glDisable(GL_BLEND);
            glDepthMask(1);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, loadscreentexture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glDisable(GL_DEPTH_TEST);                               // Disables Depth Testing
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);                            // Select The Projection Matrix
            glPushMatrix();                                         // Store The Projection Matrix
            glLoadIdentity();                                       // Reset The Projection Matrix
            glOrtho(0, screenWidth, 0, screenHeight, -100, 100);    // Set Up An Ortho Screen
            glMatrixMode(GL_MODELVIEW);                             // Select The Modelview Matrix
            glPushMatrix();                                         // Store The Modelview Matrix
            glLoadIdentity();                                       // Reset The Modelview Matrix
            glTranslatef(screenWidth / 2, screenHeight / 2, 0);
            glScalef((float) screenWidth / 2 * (1.5 - loadprogress / 200), (float) screenHeight / 2 * (1.5 - loadprogress / 200), 1);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE);
            glEnable(GL_BLEND);
            glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, 1);
            glPushMatrix();
            glBegin(GL_QUADS);
            glTexCoord2f(0 + .5, 0 + .5);
            glVertex3f(-1, -1, 0.0f);
            glTexCoord2f(1 + .5, 0 + .5);
            glVertex3f(1, -1, 0.0f);
            glTexCoord2f(1 + .5, 1 + .5);
            glVertex3f(1, 1, 0.0f);
            glTexCoord2f(0 + .5, 1 + .5);
            glVertex3f(-1, 1, 0.0f);
            glEnd();
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glDisable(GL_BLEND);
            glDepthMask(1);

            glEnable(GL_TEXTURE_2D);
            glBindTexture( GL_TEXTURE_2D, loadscreentexture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glDisable(GL_DEPTH_TEST);                               // Disables Depth Testing
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);                            // Select The Projection Matrix
            glPushMatrix();                                         // Store The Projection Matrix
            glLoadIdentity();                                       // Reset The Projection Matrix
            glOrtho(0, screenWidth, 0, screenHeight, -100, 100);    // Set Up An Ortho Screen
            glMatrixMode(GL_MODELVIEW);                             // Select The Modelview Matrix
            glPushMatrix();                                         // Store The Modelview Matrix
            glLoadIdentity();                                       // Reset The Modelview Matrix
            glTranslatef(screenWidth / 2, screenHeight / 2, 0);
            glScalef((float) screenWidth / 2 * (100 + loadprogress) / 100, (float) screenHeight / 2 * (100 + loadprogress) / 100, 1);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE);
            glEnable(GL_BLEND);
            glColor4f(loadprogress / 100, loadprogress / 100, loadprogress / 100, .4);
            glPushMatrix();
            glBegin(GL_QUADS);
            glTexCoord2f(0 + .2, 0 + .8);
            glVertex3f(-1, -1,  0.0f);
            glTexCoord2f(1 + .2, 0 + .8);
            glVertex3f(1, -1, 0.0f);
            glTexCoord2f(1 + .2, 1 + .8);
            glVertex3f(1, 1, 0.0f);
            glTexCoord2f(0 + .2, 1 + .8);
            glVertex3f(-1, 1, 0.0f);
            glEnd();
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
            glPopMatrix();                                      // Restore The Old Projection Matrix
            glDisable(GL_BLEND);
            glDepthMask(1);

            if (flashamount > 0)
            {
                if (flashamount > 1) flashamount = 1;
                if (flashdelay <= 0) flashamount -= multiplier;
                flashdelay--;
                if (flashamount < 0) flashamount = 0;

                glDisable(GL_DEPTH_TEST);                               // Disables Depth Testing
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glDepthMask(0);
                glMatrixMode(GL_PROJECTION);                            // Select The Projection Matrix
                glPushMatrix();                                         // Store The Projection Matrix
                glLoadIdentity();                                       // Reset The Projection Matrix
                glOrtho(0, screenWidth, 0, screenHeight, -100, 100);    // Set Up An Ortho Screen
                glMatrixMode(GL_MODELVIEW);                             // Select The Modelview Matrix
                glPushMatrix();                                         // Store The Modelview Matrix
                glLoadIdentity();                                       // Reset The Modelview Matrix
                glScalef(screenWidth, screenHeight, 1);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                glColor4f(flashr, flashg, flashb, flashamount);
                glBegin(GL_QUADS);
                glVertex3f(0, 0, 0.0f);
                glVertex3f(256, 0, 0.0f);
                glVertex3f(256, 256, 0.0f);
                glVertex3f(0, 256, 0.0f);
                glEnd();
                glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
                glPopMatrix();                                      // Restore The Old Projection Matrix
                glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
                glPopMatrix();                                      // Restore The Old Projection Matrix
                glEnable(GL_DEPTH_TEST);                            // Enables Depth Testing
                glEnable(GL_CULL_FACE);
                glDisable(GL_BLEND);
                glDepthMask(1);
            }

            SDL_GL_SwapWindow(window);
            loadscreencolor = 0;
        }
    }

    void Game::FadeLoadingScreen(float howmuch)
    {
        static float loadprogress;

        glLoadIdentity();

        // Clear to black
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        loadprogress = howmuch;

        // Background
        glDisable(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glDisable(GL_DEPTH_TEST);                               // Disables Depth Testing
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDepthMask(0);
        glMatrixMode(GL_PROJECTION);                            // Select The Projection Matrix
        glPushMatrix();                                         // Store The Projection Matrix
        glLoadIdentity();                                       // Reset The Projection Matrix
        glOrtho(0, screenWidth, 0, screenHeight, -100, 100);    // Set Up An Ortho Screen
        glMatrixMode(GL_MODELVIEW);                             // Select The Modelview Matrix
        glPushMatrix();                                         // Store The Modelview Matrix
        glLoadIdentity();                                       // Reset The Modelview Matrix
        glTranslatef(screenWidth / 2, screenHeight / 2, 0);
        glScalef((float) screenWidth / 2, (float) screenHeight / 2, 1);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        glColor4f(loadprogress / 100, 0, 0, 1);
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-1, -1, 0.0f);
        glTexCoord2f(1, 0);
        glVertex3f(1, -1, 0.0f);
        glTexCoord2f(1, 1);
        glVertex3f(1, 1, 0.0f);
        glTexCoord2f(0, 1);
        glVertex3f(-1, 1, 0.0f);
        glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
        glPopMatrix();                                      // Restore The Old Projection Matrix
        glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
        glPopMatrix();                                      // Restore The Old Projection Matrix
        glDisable(GL_BLEND);
        glDepthMask(1);

        SDL_GL_SwapWindow(window);

        loadscreencolor = 0;
    }
}