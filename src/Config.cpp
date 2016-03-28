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

#include "Config.h"
#include "FileIO.h"
#include "Globals.h"

Ragaru::Config::Config()
{

}

bool
Ragaru::Config::load()
{
    const char *file = locateConfigFile("ragaru.ini").c_str();

    try {
        load_from_file(file);
    } catch (const Glib::KeyFileError& ex) {
        LOG->ERR("Can't load configuration file... why: %s", ex.what().data());
    } catch (const Glib::FileError& ex) {
        LOG->ERR("Can't open configuration file... because: %s", ex.what().data());
    }

    game_difficulty = 1;
    game_speed = 1;
    game_debug = false;
    game_immediate_mode = false;
    video_resw = 0;
    video_resh = 0;
    video_detail = 1;
    video_blood = 0;
    video_foliage = 1;
    video_damage_effects = 0;
    video_blur_velocity = 0;
    video_blur = false;
    video_always_blur = false;
    video_vbl_sync = false;
    video_show_fps = false;
    video_trilinear = true;
    video_decals = true;
    video_auto_slomo = false;
    sound_volume = 80;
    sound_ambient = true;
    sound_music = true;
    input_key_back = SDL_SCANCODE_S;
    input_key_forward = SDL_SCANCODE_W;
    input_key_left = SDL_SCANCODE_A;
    input_key_right = SDL_SCANCODE_D;
    input_key_jump = SDL_SCANCODE_SPACE;
    input_key_crouch = SDL_SCANCODE_LSHIFT;
    input_key_draw = SDL_SCANCODE_E;
    input_key_throw = SDL_SCANCODE_Q;
    input_key_chat = SDL_SCANCODE_T;
    input_button_attack = SDL_BUTTON_LEFT;
    input_mouse_speed = 1;
    input_mouse_invert = false;
    input_floatjump = true;
    input_mouse_jump = false;

    if(!has_group("GAME")) {
        set_integer("GAME", "difficulty", game_difficulty);
        set_integer("GAME", "speed", game_speed);
        set_boolean("GAME", "debug", game_debug);
        set_boolean("GAME", "immediate_mode", game_immediate_mode);
    } else {
        if(!has_key("GAME", "difficulty"))
            set_integer("GAME", "difficulty", game_difficulty);
        else
            game_difficulty = get_integer("GAME", "difficulty");
        if(!has_key("GAME", "speed"))
            set_integer("GAME", "speed", game_speed);
        else
            game_speed = get_integer("GAME", "speed");
        if(!has_key("GAME", "debug"))
            set_boolean("GAME", "debug", game_debug);
        else
            game_debug = get_boolean("GAME", "debug");
        if(!has_key("GAME", "immediate_mode"))
            set_boolean("GAME", "immediate_mode", game_immediate_mode);
        else
            game_immediate_mode = get_boolean("GAME", "immediate_mode");
    }

    if(!has_group("VIDEO")) {
        set_integer("VIDEO", "resw", video_resw);
        set_integer("VIDEO", "resh", video_resh);
        set_integer("VIDEO", "blur", video_blur);
        set_integer("VIDEO", "detail", video_detail);
        set_integer("VIDEO", "blood", video_blood);
        set_integer("VIDEO", "foliage", video_foliage);
        set_integer("VIDEO", "damage_effects", video_damage_effects);
        set_integer("VIDEO", "blur_velocity", video_blur_velocity);
        set_boolean("VIDEO", "blur", video_blur);
        set_boolean("VIDEO", "always_blur", video_always_blur);
        set_boolean("VIDEO", "vbl_sync", video_vbl_sync);
        set_boolean("VIDEO", "show_fps", video_show_fps);
        set_boolean("VIDEO", "trilinear", video_trilinear);
        set_boolean("VIDEO", "decals", video_decals);
        set_boolean("VIDEO", "auto_slomo", video_auto_slomo);
        set_boolean("VIDEO", "text", video_text);
    } else {
        if(!has_key("VIDEO", "resw"))
            set_integer("VIDEO", "resw", video_resw);
        else
            video_resw = get_integer("VIDEO", "resw");
        if(!has_key("VIDEO", "resh"))
            set_integer("VIDEO", "resh", video_resh);
        else
            video_resh = get_integer("VIDEO", "resh");
        if(!has_key("VIDEO", "detail"))
            set_integer("VIDEO", "detail", video_detail);
        else
            video_detail = get_integer("VIDEO", "detail");
        if(!has_key("VIDEO", "blood"))
            set_integer("VIDEO", "blood", video_blood);
        else
            video_blood = get_integer("VIDEO", "blood");
        if(!has_key("VIDEO", "foliage"))
            set_integer("VIDEO", "foliage", video_foliage);
        else
            video_foliage = get_integer("VIDEO", "foliage");
        if(!has_key("VIDEO", "damage_effects"))
            set_integer("VIDEO", "damage_effects", video_damage_effects);
        else
            video_damage_effects = get_integer("VIDEO", "damage_effects");
        if(!has_key("VIDEO", "blur_velocity"))
            set_integer("VIDEO", "blur_velocity", video_blur_velocity);
        else
            video_blur_velocity = get_integer("VIDEO", "blur_velocity");
        if(!has_key("VIDEO", "blur"))
            set_boolean("VIDEO", "blur", video_blur);
        else
            video_blur = get_boolean("VIDEO", "blur");
        if(!has_key("VIDEO", "always_blur"))
            set_boolean("VIDEO", "always_blur", video_always_blur);
        else
            video_always_blur = get_boolean("VIDEO", "always_blur");
        if(!has_key("VIDEO", "vbl_sync"))
            set_boolean("VIDEO", "vbl_sync", video_vbl_sync);
        else
            video_vbl_sync = get_boolean("VIDEO", "vbl_sync");
        if(!has_key("VIDEO", "show_fps"))
            set_boolean("VIDEO", "show_fps", video_show_fps);
        else
            video_show_fps = get_boolean("VIDEO", "show_fps");
        if(!has_key("VIDEO", "trilinear"))
            set_boolean("VIDEO", "trilinear", video_trilinear);
        else
            video_trilinear = get_boolean("VIDEO", "trilinear");
        if(!has_key("VIDEO", "decals"))
            set_boolean("VIDEO", "decals", video_decals);
        else
            video_decals = get_boolean("VIDEO", "decals");
        if(!has_key("VIDEO", "auto_slomo"))
            set_boolean("VIDEO", "auto_slomo", video_auto_slomo);
        else
            video_auto_slomo = get_boolean("VIDEO", "auto_slomo");
        if(!has_key("VIDEO", "text"))
            set_boolean("VIDEO", "text", video_text);
        else
            video_decals = get_boolean("VIDEO", "text");
    }

    if(!has_group("SOUND")) {
        set_integer("SOUND", "volume", sound_volume);
        set_boolean("SOUND", "ambient", sound_ambient);
        set_boolean("SOUND", "music", sound_music);
    } else {
        if(!has_key("SOUND", "volume"))
            set_integer("SOUND", "volume", sound_volume);
        else
            sound_volume = get_integer("SOUND", "volume");
        if(!has_key("SOUND", "ambient"))
            set_boolean("SOUND", "ambient", sound_ambient);
        else
            sound_ambient = get_boolean("SOUND", "ambient");
        if(!has_key("SOUND", "music"))
            set_boolean("SOUND", "music", sound_music);
        else
            sound_music = get_boolean("SOUND", "music");
    }

    if(!has_group("INPUT")) {
        set_integer("INPUT", "key_back", input_key_back);
        set_integer("INPUT", "key_forward", input_key_forward);
        set_integer("INPUT", "key_left", input_key_left);
        set_integer("INPUT", "key_right", input_key_right);
        set_integer("INPUT", "key_jump", input_key_jump);
        set_integer("INPUT", "key_crouchk", input_key_crouch);
        set_integer("INPUT", "key_draw", input_key_back);
        set_integer("INPUT", "key_throw", input_key_throw);
        set_integer("INPUT", "key_chat", input_key_chat);
        set_integer("INPUT", "button_attack", input_button_attack);
        set_integer("INPUT", "mouse_speed", input_mouse_speed);
        set_boolean("INPUT", "mouse_invert", input_mouse_invert);
        set_boolean("INPUT", "floatjump", input_floatjump);
        set_boolean("INPUT", "mouse_jump", input_mouse_jump);
    } else {
        if(!has_key("INPUT", "key_back"))
            set_integer("INPUT", "key_back", input_key_back);
        else
            input_key_back = (SDL_Scancode) get_integer("INPUT", "key_back");
        if(!has_key("INPUT", "key_forward"))
            set_integer("INPUT", "key_forward", input_key_forward);
        else
            input_key_forward = (SDL_Scancode) get_integer("INPUT", "key_forward");
        if(!has_key("INPUT", "key_left"))
            set_integer("INPUT", "key_left", input_key_left);
        else
            input_key_left = (SDL_Scancode) get_integer("INPUT", "key_left");
        if(!has_key("INPUT", "key_right"))
            set_integer("INPUT", "key_right", input_key_right);
        else
            input_key_right = (SDL_Scancode) get_integer("INPUT", "key_right");
        if(!has_key("INPUT", "key_jump"))
            set_integer("INPUT", "key_jump", input_key_jump);
        else
            input_key_jump = (SDL_Scancode) get_integer("INPUT", "key_jump");
        if(!has_key("INPUT", "key_crouch"))
            set_integer("INPUT", "key_crouchk", input_key_crouch);
        else
            input_key_crouch = (SDL_Scancode) get_integer("INPUT", "key_crouchk");
        if(!has_key("INPUT", "key_draw"))
            set_integer("INPUT", "key_draw", input_key_back);
        else
            input_key_draw = (SDL_Scancode) get_integer("INPUT", "key_draw");
        if(!has_key("INPUT", "key_throw"))
            set_integer("INPUT", "key_throw", input_key_throw);
        else
            input_key_throw = (SDL_Scancode) get_integer("INPUT", "key_throw");
        if(!has_key("INPUT", "key_chat"))
            set_integer("INPUT", "key_chat", input_key_chat);
        else
            input_key_chat = (SDL_Scancode) get_integer("INPUT", "key_chat");
        if(!has_key("INPUT", "button_attack"))
            set_integer("INPUT", "button_attack", input_button_attack);
        else
            input_button_attack = get_integer("INPUT", "button_attack");
        if(!has_key("INPUT", "mouse_speed"))
            set_integer("INPUT", "mouse_speed", input_mouse_speed);
        else
            input_mouse_speed = get_integer("INPUT", "mouse_speed");
        if(!has_key("INPUT", "mouse_invert"))
            set_boolean("INPUT", "mouse_invert", input_mouse_invert);
        else
            input_mouse_invert = get_boolean("INPUT", "mouse_invert");
        if(!has_key("INPUT", "floatjump"))
            set_boolean("INPUT", "floatjump", input_floatjump);
        else
            input_floatjump = get_boolean("INPUT", "floatjump");
        if(!has_key("INPUT", "mouse_jump"))
            set_boolean("INPUT", "mouse_jump", input_mouse_jump);
        else
            input_mouse_jump = get_boolean("INPUT", "mouse_jump");
    }

    return true;
}

bool
Ragaru::Config::save()
{
    set_integer("GAME", "difficulty", game_difficulty);
    set_boolean("GAME", "debug", game_debug);
    set_boolean("GAME", "immediate_mode", game_immediate_mode);
    set_integer("VIDEO", "resw", video_resw);
    set_integer("VIDEO", "resh", video_resh);
    set_integer("VIDEO", "blur", video_blur);
    set_integer("VIDEO", "detail", video_detail);
    set_integer("VIDEO", "blood", video_blood);
    set_integer("VIDEO", "foliage", video_foliage);
    set_integer("VIDEO", "damage_effects", video_damage_effects);
    set_integer("VIDEO", "blur_velocity", video_blur_velocity);
    set_boolean("VIDEO", "always_blur", video_always_blur);
    set_boolean("VIDEO", "vbl_sync", video_vbl_sync);
    set_boolean("VIDEO", "show_fps", video_show_fps);
    set_boolean("VIDEO", "trilinear", video_trilinear);
    set_boolean("VIDEO", "decals", video_decals);
    set_boolean("VIDEO", "auto_slomo", video_auto_slomo);
    set_boolean("VIDEO", "text", video_text);
    set_integer("SOUND", "volume", sound_volume);
    set_boolean("SOUND", "ambient", sound_ambient);
    set_boolean("SOUND", "music", sound_music);
    set_integer("INPUT", "key_back", input_key_back);
    set_integer("INPUT", "key_forward", input_key_forward);
    set_integer("INPUT", "key_left", input_key_left);
    set_integer("INPUT", "key_right", input_key_right);
    set_integer("INPUT", "key_jump", input_key_jump);
    set_integer("INPUT", "key_crouchk", input_key_crouch);
    set_integer("INPUT", "key_draw", input_key_back);
    set_integer("INPUT", "key_throw", input_key_throw);
    set_integer("INPUT", "key_chat", input_key_chat);
    set_integer("INPUT", "button_attack", input_button_attack);
    set_integer("INPUT", "mouse_speed", input_mouse_speed);
    set_boolean("INPUT", "mouse_invert", input_mouse_invert);
    set_boolean("INPUT", "floatjump", input_floatjump);
    set_boolean("INPUT", "mouse_jump", input_mouse_jump);

    bool saved = save_to_file(locateConfigFile("ragaru.ini").c_str());

    if(!saved) {
        LOG->ERR("Can't save config file... (permissions ?)");

        return false;
    }

    return true;
}
