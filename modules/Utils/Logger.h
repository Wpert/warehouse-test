#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel minLevel;
    bool consoleOutput;
    
    std::string getCurrentTime();
    std::string levelToString(LogLevel level);
    
public:
    Logger(const std::string& filename, LogLevel level = LogLevel::INFO, bool console = true);
    ~Logger();
    
    void log(LogLevel level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    
    void setMinLevel(LogLevel level) { minLevel = level; }
};

#endif
