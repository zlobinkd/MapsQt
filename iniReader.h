#include <string>

class IniReader{
public:
    IniReader() = default;
    void read();
    const std::string& mapFilePath() const { return _mapFilePath; }
    const std::string& trafficSignalAssignmentsFilePath() const { return _trafficSignalAssignmentsFilePath; }

private:
    std::string _mapFilePath;
    std::string _trafficSignalAssignmentsFilePath;
};
