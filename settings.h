#pragma once

#include "core.h"
#include <string>

// Singleton for general application settings
class Settings {
public:
    static const Settings& instance()
    {
        static Settings instance;
        return instance;
    }

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    inline const std::string& mapFilePath() const { return _mapFilePath; }
    inline const std::string& trafficSignalAssignmentsFilePath() const { return _trafficSignalAssignmentsFilePath; }
    inline double imageSize() const { return _imageSize; }
    inline size_t simulationPoolSize() const { return _simulationPoolSize; }
    inline double sampleTime() const { return _sampleTime; }
    inline double trafficLightSinglePhaseDuration() const { return _trafficLightSinglePhaseDuration; }
    inline double minDesiredGap() const { return _minDesiredGap; }
    inline double safeReactionTime() const { return _safeReactionTime; }
    inline double maxAcceleration() const { return _maxAcceleration; }
    inline double maxDeceleration() const { return _maxDeceleration; }

private:
    Settings();

    std::string _mapFilePath;
    std::string _trafficSignalAssignmentsFilePath;
    // defines the vertical size of the map
    double _imageSize;

    // simulation parameters
    size_t _simulationPoolSize;
    double _sampleTime;
    double _trafficLightSinglePhaseDuration;
    double _minDesiredGap;
    double _safeReactionTime;
    double _maxAcceleration;
    double _maxDeceleration;
};
