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

#ifndef GAME_H
#define GAME_H


#include "TGALoader.h"
#include "Input.h"
#include "Terrain.h"
#include "Skybox.h"
#include "Skeleton.h"
#include "Models.h"
#include "Lights.h"
#include "Person.h"
#include "Constants.h"
#include "Sprites.h"
#include "Text.h"
#include "Objects.h"
#include "Weapons.h"
#include "BinIO.h"
#include "gamegl.h"
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>

extern GLuint rabbittexture;

namespace Ragaru
{
    typedef struct {
        SDL_Scancode crouch;
        SDL_Scancode jump;
        SDL_Scancode forward;
        SDL_Scancode chat;
        SDL_Scancode back;
        SDL_Scancode left;
        SDL_Scancode right;
        SDL_Scancode draw;
        SDL_Scancode throww;

        Uint8 attack;
        Uint8 oldattack;
    } GameKeys;

    class Game
    {
    public:
        typedef std::map<std::string, GLuint> TextureList;
        typedef std::map<GLuint, std::string> GLTextureList;
        typedef TextureList::iterator TexIter;
        static TextureList textures;
        SDL_GLContext glcontext;

        // Something to say
        int screenWidth;
        int screenHeight;
        int textureSize;

        GameKeys keys;

        GLuint terraintexture;
        GLuint terraintexture2;
        GLuint terraintexture3;
        GLuint screentexture;
        GLuint screentexture2;
        GLuint logotexture;
        GLuint loadscreentexture;
        GLuint Maparrowtexture;
        GLuint Mapboxtexture;
        GLuint Mapcircletexture;
        GLuint cursortexture;
        GLuint Mainmenuitems[10];

        int nummenuitems;
        int startx[100];
        int starty[100];
        int endx[100];
        int endy[100];
        float selectedlong[100];
        float offsetx[100];
        float offsety[100];
        float movex[100];
        float movey[100];
        float transition;
        int anim;
        int selected;
        int loaddistrib;
        int keyselect;
        int indemo;

        bool won;

        bool entername;

        char menustring[100][256];
        char registrationname[256];
        float registrationnumber;

        int newdetail;
        int newscreenwidth;
        int newscreenheight;

        bool gameon;
        float deltah, deltav;
        int mousecoordh, mousecoordv;
        int oldmousecoordh, oldmousecoordv;
        float rotation, rotation2;
        SkyBox skybox;
        bool cameramode;
        bool cameratogglekeydown;
        bool chattogglekeydown;
        int olddrawmode;
        int drawmode;
        bool drawmodetogglekeydown;
        bool explodetogglekeydown;
        bool detailtogglekeydown;
        bool firstload;
        bool oldbutton;

        float leveltime;
        float loadtime;

        Model hawk;
        XYZ hawkcoords;
        XYZ realhawkcoords;
        GLuint hawktexture;
        float hawkrotation;
        float hawkcalldelay;

        Model eye;
        Model iris;
        Model cornea;

        bool stealthloading;

        int campaignnumlevels;
        char campaignmapname[50][256];
        char campaigndescription[50][256];
        int campaignchoosenext[50];
        int campaignnumnext[50];
        int campaignnextlevel[50][10];
        int campaignchoicesmade;
        int campaignchoices[5000];
        int campaignlocationx[50];
        int campaignlocationy[50];
        int campaignchoicenum;
        int campaignchoicewhich[10];
        int whichchoice;

        int numlevelspassed;
        int levelorder[5000];
        int levelvisible[50];
        int levelhighlight[50];

        bool minimap;

        int musictype, oldmusictype, oldoldmusictype;
        bool realthreat;

        Model rabbit;
        XYZ rabbitcoords;

        XYZ mapcenter;
        float mapradius;

        Text text;
        float fps;

        XYZ cameraloc;
        float cameradist;

        bool envtogglekeydown;
        bool slomotogglekeydown;
        bool texturesizetogglekeydown;
        bool freezetogglekeydown;
        int drawtoggle;

        bool editorenabled;
        int editortype;
        float editorsize;
        float editorrotation;
        float editorrotation2;

        float brightness;

        int quit;
        int tryquit;

        XYZ pathpoint[30];
        int numpathpoints;
        int numpathpointconnect[30];
        int pathpointconnect[30][30];
        int pathpointselected;

        int endgame;
        bool scoreadded;
        int numchallengelevels;

        bool console;
        int archiveselected;
        char consoletext[15][256];
        int consolechars[15];
        bool chatting;
        char displaytext[15][256];
        int displaychars[15];
        float displaytime[15];
        float displayblinkdelay;
        bool displayblink;
        int displayselected;
        bool consolekeydown;
        bool consoletogglekeydown;
        float consoleblinkdelay;
        bool consoleblink;
        int consoleselected;
        int togglekey[140];
        float togglekeydelay[140];
        bool registernow;
        bool autocam;

        void CleanUp();
        bool Setup();
        void DoMouse();
        long long MD5_string(char* string);
        static SDL_Surface* LoadImage(std::string filename);
        static void LoadTexture(const char* fileName, GLuint* textureid, int mipmap, bool hasalpha);
        static void LoadTextureSave(const char* fileName, GLuint* textureid, int mipmap, GLubyte* array, int* skinsize);
        void LoadSave(const char* fileName, GLuint* textureid, bool mipmap, GLubyte* array, int* skinsize);
        bool AddClothes(const char* fileName, GLuint* textureid, bool mipmap, GLubyte* array, int* skinsize);
        void InitGame();
        void LoadStuff();
        void LoadingScreen();
        void FadeLoadingScreen(float howmuch);
        void Dispose();
        int DrawGLScene(void);
        void Tick();
        void TickOnce();
        void TickOnceAfter();
        void SetUpLighting();
        void Loadlevel(int which);
        void Loadlevel(const char* name);
        void LoadSounds();
        void Setenvironment(int which);
        GLvoid ReSizeGLScene(float fov, float near);
        int findPathDist(int start, int end);
        int checkcollide(XYZ startpoint, XYZ endpoint);
        int checkcollide(XYZ startpoint, XYZ endpoint, int what);
        int loading;
        float talkdelay;

        int numboundaries;
        XYZ boundary[360];

        int whichlevel;
        int oldenvironment;
        int targetlevel;
        float changedelay;

        float musicvolume[4];
        float oldmusicvolume[4];
        int musicselected;
        int change;
        Game();
        ~Game()
        {
            for (int i = 0; i < 10; i++)
            {
                if (Mainmenuitems[i]) glDeleteTextures(1, &Mainmenuitems[i]);
            }
            glDeleteTextures(1, &cursortexture);
            glDeleteTextures(1, &Maparrowtexture);
            glDeleteTextures(1, &Mapboxtexture);
            glDeleteTextures(1, &Mapcircletexture);
            glDeleteTextures(1, &terraintexture);
            glDeleteTextures(1, &terraintexture2);
            if(screentexture > 0)
                glDeleteTextures(1, &screentexture);
            if(screentexture2 > 0)
                glDeleteTextures(1, &screentexture2);
            glDeleteTextures(1, &hawktexture);
            glDeleteTextures(1, &logotexture);
            glDeleteTextures(1, &loadscreentexture);

            Dispose();
        }

    protected:
        void LoadSound(const int sample, const char* filename, float min, float max);
        bool isMouseHoverRect(int sx, int ex, int sy, int ey);
    };
}

#ifdef __GNUC__
#define LONGLONGCONST(x) (x##ll)
#else
#define LONGLONGCONST(x) (x)
#endif

#endif /* GAME_H */
