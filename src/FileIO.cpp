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

#include "FileIO.h"
#include "Globals.h"
#include <glibmm.h>
#include <giomm.h>
#include <iostream>
#include <unistd.h>


std::string
locateConfigFile(const char *configFile) {
    std::string file = Glib::get_user_config_dir();
    file.append("/ragaru/");
    file.append(configFile);

    // initialize giomm
    Gio::init();

    Glib::RefPtr<Gio::File> gfile = Gio::File::create_for_path(file);

    if(!gfile->query_exists()) {
        try {
            gfile->get_parent()->make_directory_with_parents();
        } catch (const Gio::Error& e) {
            if(!(e.code() == Gio::Error::EXISTS)) {
                LOG->ERR("Can't create config directory, exiting...");
                SDL_Quit();
            }
        }

        try {
            gfile->create_file();
        } catch (const Gio::Error& e) {
            if(!(e.code() == Gio::Error::EXISTS)) {
                LOG->ERR("Can't create config file, exiting...");
                SDL_Quit();
            }
        }
    }

    LOG->LOG("ConfigFile %s: %s", configFile, file.c_str());
    return file;
}

std::string
locateDataFile(const char* subdir, const char* filename) {
    std::string file;

    // first check for relative data
    if (Glib::file_test(std::string("data"), Glib::FILE_TEST_EXISTS) && Glib::file_test(std::string("data"),  Glib::FILE_TEST_IS_DIR)) {
        file = Glib::build_filename("data", subdir, filename);
    } else { // so check at system data folders for data
        std::vector<std::string> datadirs = Glib::get_system_data_dirs();

        for (std::vector<std::string>::iterator it = datadirs.begin(); it != datadirs.end(); ++it) {
            file = Glib::build_filename(*it, "ragaru");

            if (Glib::file_test(file, Glib::FILE_TEST_EXISTS) && Glib::file_test(file,  Glib::FILE_TEST_IS_DIR)) {
                file = Glib::build_filename(file, subdir, filename);
            }
        }
    }

    bool found = false;
    if (Glib::file_test(file, Glib::FILE_TEST_EXISTS) && Glib::file_test(file, Glib::FILE_TEST_IS_REGULAR)) found = true;

    LOG->LOG("DataFile %s: %s (found: %s)", filename, file.c_str(), found ? "yes" : "no");
    return file;
}
