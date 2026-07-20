#include "iniReader.h"
#include <iostream>
#include <fstream>

static std::string trim(std::string str) {
    str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
    str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
    return str;
}

void IniReader::read() {
    std::ifstream file("..\\..\\MapsQt.ini");
    if (!file.is_open())
        std::cerr << "Could not open your dumbass ini file!" << std::endl;;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        // Check for key-value pair
        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos)
            continue;

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        key = trim(key);
        value = trim(value);

        if (key == "pathToMap")
            _mapFilePath = value;
        if (key == "pathToTrafficSignalClustering")
            _trafficSignalAssignmentsFilePath = value;
    }

    file.close();
}
