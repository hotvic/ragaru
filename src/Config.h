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

#ifndef CONFIG_H
#define CONFIG_H

#include <glibmm.h>
#include <SDL.h>

namespace Ragaru
{
class Config : public Glib::KeyFile
{
public:
    int video_resw;
    int video_resh;
    int video_detail;
    int video_blood;
    int video_foliage;
    int video_damage_effects;
    int video_blur_velocity;
    bool video_blur;
    bool video_always_blur;
    bool video_vbl_sync;
    bool video_show_fps;
    bool video_trilinear;
    bool video_decals;
    bool video_auto_slomo;
    bool video_text;
    int sound_volume;
    bool sound_ambient;
    bool sound_music;
    int game_difficulty;
    int game_speed;
    bool game_debug;
    bool game_immediate_mode;
    SDL_Scancode input_key_back;
    SDL_Scancode input_key_forward;
    SDL_Scancode input_key_left;
    SDL_Scancode input_key_right;
    SDL_Scancode input_key_jump;
    SDL_Scancode input_key_crouch;
    SDL_Scancode input_key_draw;
    SDL_Scancode input_key_throw;
    SDL_Scancode input_key_chat;
    int input_mouse_speed;
    int input_button_attack;
    bool input_mouse_invert;
    bool input_floatjump;
    bool input_mouse_jump;

    Config();
    bool load();
    bool save();
};
}

#endif /* CONFIG_H */