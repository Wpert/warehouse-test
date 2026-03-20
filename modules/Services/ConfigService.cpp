#include "ConfigService.h"
#include <algorithm>
#include <cctype>

ConfigService::ConfigService(const std::string& filename) : configFile(filename) {
    load();
}

void ConfigService::parseLine(const std::string& line) {
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    
    if (trimmed.empty() || trimmed[0] == '#') return;
    
    size_t pos = trimmed.find('=');
    if (pos != std::string::npos) {
        std::string key = trimmed.substr(0, pos);
        std::string value = trimmed.substr(pos + 1);
        
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        if (!key.empty()) {
            config[key] = value;
        }
    }
}

void ConfigService::load() {
    std::ifstream file(configFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            parseLine(line);
        }
        file.close();
    } else {
        // Создаем конфиг по умолчанию
        config["database"] = "warehouse.db";
        config["log_file"] = "warehouse.log";
        config["log_level"] = "info";
        save();
    }
}

void ConfigService::save() {
    std::ofstream file(configFile);
    if (file.is_open()) {
        for (const auto& [key, value] : config) {
            file << key << " = " << value << std::endl;
        }
        file.close();
    }
}

std::string ConfigService::getString(const std::string& key, const std::string& defaultValue) {
    auto it = config.find(key);
    return (it != config.end()) ? it->second : defaultValue;
}

int ConfigService::getInt(const std::string& key, int defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

double ConfigService::getDouble(const std::string& key, double defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) {
        try {
            return std::stod(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool ConfigService::getBool(const std::string& key, bool defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) {
        std::string val = it->second;
        std::transform(val.begin(), val.end(), val.begin(), ::tolower);
        return val == "true" || val == "yes" || val == "1";
    }
    return defaultValue;
}

void ConfigService::setString(const std::string& key, const std::string& value) {
    config[key] = value;
    save();
}

std::string ConfigService::getDatabasePath() {
    return getString("database", "warehouse.db");
}

std::string ConfigService::getLogFile() {
    return getString("log_file", "warehouse.log");
}

std::string ConfigService::getLogLevel() {
    std::string level = getString("log_level", "info");
    std::transform(level.begin(), level.end(), level.begin(), ::tolower);
    return level;
}
