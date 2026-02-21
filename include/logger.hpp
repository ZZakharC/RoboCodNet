#pragma once

#include <fstream>
#include <string>
#include <string_view>

enum class LogLevel { START, INFO, WARNING, ERROR };

class Logger
{
private:
    std::ofstream logFile;

public:
    // Запрещаем копирование (защита от случайных багов при передаче по значению)
    Logger() = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    Logger(const std::string &filename);

    ~Logger();

    void log(LogLevel level, std::string_view text);
};