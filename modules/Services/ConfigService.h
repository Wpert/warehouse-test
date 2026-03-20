#ifndef CONFIGSERVICE_H
#define CONFIGSERVICE_H

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

class ConfigService {
private:
    std::string configFile;
    std::map<std::string, std::string> config;
    
    void parseLine(const std::string& line);
    void load();
    void save();
    
public:
    ConfigService(const std::string& filename);
    
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    int getInt(const std::string& key, int defaultValue = 0);
    double getDouble(const std::string& key, double defaultValue = 0.0);
    bool getBool(const std::string& key, bool defaultValue = false);
    
    void setString(const std::string& key, const std::string& value);
    
    std::string getDatabasePath();
    std::string getLogFile();
    std::string getLogLevel();
};

#endif
