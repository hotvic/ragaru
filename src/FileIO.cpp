#include "FileIO.h"
#include "Log.h"
#include <glibmm.h>
#include <iostream>
#include <unistd.h>


std::string
locateConfigFile(const char *configFile) {
    std::string file = Glib::get_user_config_dir();
    file.append("/lugaru/");
    file.append(configFile);

    LOG->LOG("ConfigFile %s: %s", configFile, file.c_str());
    return file;
}

std::string
locateDataFile(const char *datafile) {
    std::string file;

    // first check for relative data
    if (Glib::file_test(std::string("data"), Glib::FILE_TEST_EXISTS) && Glib::file_test(std::string("data"),  Glib::FILE_TEST_IS_DIR)) {
        file.append("data/");
        file.append(datafile);
    } else { // so check at system data folders for data
        std::vector<std::string> datadirs = Glib::get_system_data_dirs();

        for (std::vector<std::string>::iterator it = datadirs.begin(); it != datadirs.end(); ++it) {
            file.append(*it);
            file.append("/lugaru");

            if (Glib::file_test(file, Glib::FILE_TEST_EXISTS) && Glib::file_test(file,  Glib::FILE_TEST_IS_DIR)) {
                file.append("/");
                file.append(datafile);
            }
        }
    }

    LOG->LOG("DataFile %s: %s", datafile, file.c_str());
    return file;
}
