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
    Logger(const std::string &filename);

    ~Logger();

    void log(LogLevel level, std::string_view text);
};