#include "settings.h"
#include <iostream>
#include <fstream>

static std::string trim(std::string str) {
    str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
    str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
    return str;
}

Settings::Settings() {
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
        if (key == "imageSize")
            _imageSize = std::stod(value);
        if (key == "simulationPoolSize")
            _simulationPoolSize = std::stoi(value);
        if (key == "sampleTime")
            _sampleTime = std::stod(value);
        if (key == "trafficLightSinglePhaseDuration")
            _trafficLightSinglePhaseDuration = std::stod(value);
        if (key == "minDesiredGap")
            _minDesiredGap = std::stod(value);
        if (key == "safeReactionTime")
            _safeReactionTime = std::stod(value);
        if (key == "maxAcceleration")
            _maxAcceleration = std::stod(value);
        if (key == "maxDeceleration")
            _maxDeceleration = std::stod(value);
    }

    file.close();
}
