/*
 * Copruright © 2015 - Victor A. Santos <victoraur.santos@gmail.com>
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

#include "Options.h"

MainGroup::MainGroup()
    : Glib::OptionGroup("ragaru", "ragaru game command line"),
    windowed(false),
    fullscreen(false),
    listresolutions(false),
    nomousegrab(false),
    audio_module("")
{
    Glib::OptionEntry entry1;
    entry1.set_long_name("windowed");
    entry1.set_short_name('w');
    entry1.set_description("Launch game in windowed mode (ignoring configuration)");
    add_entry(entry1, windowed);

    Glib::OptionEntry entry2;
    entry2.set_long_name("fullscreen");
    entry2.set_short_name('f');
    entry2.set_description("Launch game in fullscreen mode (ignoring configuration)");
    add_entry(entry2, fullscreen);

    Glib::OptionEntry entry3;
    entry3.set_long_name("listresolutions");
    entry3.set_short_name('l');
    entry3.set_description("List supported resolutions modes");
    add_entry(entry3, listresolutions);

    Glib::OptionEntry entry4;
    entry4.set_long_name("nomousegrab");
    entry4.set_description("Disable mouse grabbing");
    add_entry(entry4, nomousegrab);

    Glib::OptionEntry entry5;
    entry5.set_long_name("audio-module");
    entry5.set_short_name('A');
    entry5.set_description("OpenAL audio output module to use");
    add_entry(entry5, audio_module);
}

bool
MainGroup::on_option_arg_string(const Glib::ustring& option_name, const Glib::ustring& value, bool has_value)
{
    if(option_name != "-D" && option_name != "--audio-moduule") {
        audio_module = "Unknow '" + option_name + "' option";
        throw Glib::OptionError(Glib::OptionError::UNKNOWN_OPTION, audio_module);
    }

    if(!has_value) return false;

    if(value != "alsa" && value != "oss" && value != "null") return false;

    audio_module = value;

    return true;
}
