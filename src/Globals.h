/*
 * Copyright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "Game.h"
#include "Person.h"
#include "Options.h"
#include "Log.h"
#include "Config.h"
#include "openal_wrapper.h"
#include <SDL.h>

extern SDL_Window *window;
extern Ragaru::Log *LOG;
extern Ragaru::Config config;
extern MainGroup options;
extern float multiplier;
extern float sps;

extern int slomo;
extern float texdetail;
extern bool osx;
extern bool stillloading;
extern bool showpoints;
extern XYZ viewer;
extern int environment;
extern float texscale;
extern Terrain terrain;
extern OPENAL_SAMPLE* samp[100];
extern int channels[100];
extern Sprites sprites;
extern float gravity;
extern Objects objects;
extern float slomodelay;
extern Animation animation[animation_count];
extern Light light;
extern GLubyte bloodText[512 * 512 * 3];
extern GLubyte wolfbloodText[512 * 512 * 3];
extern float terraindetail;
extern float camerashake;
extern float woozy;
extern float blackout;
extern bool cellophane;
extern Weapons weapons;
extern float windvar;
extern float precipdelay;
extern XYZ viewerfacing;
extern float viewdistance;
extern bool freeze;
extern bool keyboardfrozen;
extern int netdatanew;
extern bool loadingstuff;
extern char mapname[256];
extern XYZ windvector;
extern bool visibleloading;
extern int loadscreencolor;
extern float flashamount, flashr, flashg, flashb;
extern int flashdelay;
extern XYZ envsound[30];
extern float envsoundvol[30];
extern int numenvsounds;
extern float envsoundlife[30];
extern int bonus;
extern int oldbonus;
extern float bonusvalue;
extern float bonustotal;
extern float bonustime;
extern float startbonustotal;
extern float tintr, tintg, tintb;
extern float bonusnum[100];
extern int tutoriallevel;
extern float smoketex;
extern float tutorialstagetime;
extern int tutorialstage;
extern float tutorialmaxtime;
extern float tutorialsuccess;
extern bool againbonus;
extern bool reversaltrain;
extern bool canattack;
extern bool cananger;
extern float damagedealt;
extern float damagetaken;
extern int maptype;
extern int editoractive;
extern int editorpathtype;


/* Map */
extern int hostile;
extern float hostiletime;

/* Hotspots */
extern int numhotspots;
extern int killhotspot;
extern XYZ hotspot[40];
extern int hotspottype[40];
extern float hotspotsize[40];
extern char hotspottext[40][256];
extern int currenthotspot;
extern int winhotspot;

/* Players */
extern float realmultiplier;
extern int numplayers;
extern Person player[maxplayers];
extern int numfalls;
extern int numflipfail;
extern int numseen;
extern int numstaffattack;
extern int numswordattack;
extern int numknifeattack;
extern int numunarmedattack;
extern int numescaped;
extern int numflipped;
extern int numwallflipped;
extern int numthrowkill;
extern int numafterkill;
extern int numreversals;
extern int numattacks;
extern int maxalarmed;
extern int numresponded;

/* Skybox */
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern float skyboxlightr;
extern float skyboxlightg;
extern float skyboxlightb;

/* Dialogues */
extern int numdialogues;
extern int numdialogueboxes[max_dialogues];
extern int dialoguetype[max_dialogues];
extern int dialogueboxlocation[max_dialogues][max_dialoguelength];
extern float dialogueboxcolor[max_dialogues][max_dialoguelength][3];
extern int dialogueboxsound[max_dialogues][max_dialoguelength];
extern char dialoguetext[max_dialogues][max_dialoguelength][128];
extern char dialoguename[max_dialogues][max_dialoguelength][64];
extern XYZ dialoguecamera[max_dialogues][max_dialoguelength];
extern XYZ participantlocation[max_dialogues][10];
extern int participantfocus[max_dialogues][max_dialoguelength];
extern int participantaction[max_dialogues][max_dialoguelength];
extern float participantrotation[max_dialogues][10];
extern XYZ participantfacing[max_dialogues][max_dialoguelength][10];
extern float dialoguecamerarotation[max_dialogues][max_dialoguelength];
extern float dialoguecamerarotation2[max_dialogues][max_dialoguelength];
extern int indialogue;
extern int whichdialogue;
extern float dialoguetime;
extern int dialoguegonethrough[20];
extern int windialogue;

/* Game */
extern int directing;
extern bool campaign;
extern float oldgamespeed;
extern float menupulse;
extern bool gamestart;
extern int oldmainmenu;
extern int mainmenu;
extern Ragaru::Game* pgame;
extern float blurness;
extern bool winfreeze;
extern bool gamestarted;
extern float fadestart;
extern float slomospeed;
extern float slomofreq;

/* Account */
extern int numaccounts;
extern int accountactive;
extern int accountdifficulty[10];
extern int accountprogress[10];
extern float accountpoints[10];
extern float accounthighscore[10][50];
extern float accountfasttime[10][50];
extern bool accountunlocked[10][60];
extern char accountname[10][256];
extern float accountcampaignhighscore[10];
extern float accountcampaignfasttime[10];
extern float accountcampaignscore[10];
extern float accountcampaigntime[10];
extern int accountcampaignchoicesmade[10];
extern int accountcampaignchoices[10][5000];
extern TGAImageRec texture;
extern int whichjointstartarray[26];
extern int whichjointendarray[26];
extern int numsounds;
extern float realtexdetail;
extern FRUSTUM frustum;
extern float playerdist;
extern float targetblurness;
extern GLubyte texturearray[512 * 512 * 3];
extern int netstate;
extern float motionbluramount;
extern bool isclient;
extern int test;
extern bool tilt2weird;
extern bool tiltweird;
extern bool midweird;
extern bool proportionweird;
extern bool vertexweird[6];
extern OPENAL_STREAM *strm[20];


#endif /* GLOBALS_H */
