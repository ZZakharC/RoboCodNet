#include "logger.hpp"
#include <ctime>
#include <iostream>
#include <ios>

Logger::Logger(const std::string &filename) : logFile(filename, std::ios::app) {}

Logger::~Logger() {
    log(LogLevel::WARNING, "LOGGER   CLOSE");
    logFile.close();
}

// Вывод лога
void Logger::log(LogLevel level, std::string_view text)
{
    // Время в строке 
    std::time_t t = std::time(nullptr);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    // Сообщение (лог)
    std::string message;
    
    // Выбор уровня
    switch (level) {
        // Запуск бота
        case LogLevel::START: 
            message = std::string("\n==========================================\n") + 
                      timeStr + " [START] " + text.data() +
                      std::string("\n=========================================="); break;
        
        // Информация
        case LogLevel::INFO: message = timeStr + std::string(" [INFO] ") + text.data(); break;
        // Предупреждение
        case LogLevel::WARNING: message = timeStr + std::string(" [WARN] ") + text.data(); break;
        // Ошибка
        case LogLevel::ERROR: message = timeStr + std::string(" [ERROR] ") + text.data(); break;
    }
    // Вывод лога
    std::cout << message << std::endl; // В консоль
    
    if (logFile.is_open()) // Если файл открыт 
        logFile << message << std::endl; // В файл
}
