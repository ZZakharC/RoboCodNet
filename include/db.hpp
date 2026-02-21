#pragma once

#include "logger.hpp"
#include <sqlite3.h>
#include <cstdint>
#include <string>

class UserDB {
private:
    sqlite3 *db = nullptr;
    Logger &logger;
    sqlite3_stmt* stmtIncrement = nullptr;
    sqlite3_stmt* stmtGetViolation = nullptr;

public:
    // Конструктор
    UserDB(const std::string &db_file, Logger &log);

    // Деструктор
    ~UserDB();


    // Запрещаем копирование и перемещение
    UserDB(const UserDB&) = delete;
    UserDB& operator=(const UserDB&) = delete;
    UserDB(UserDB&&) = delete;
    UserDB& operator=(UserDB&&) = delete;

    // Увеличить количество нарушений на 1
    void incrementViolation(const int64_t chatId, const int64_t userId);
    
    // Получить количество нарушений
    int32_t getViolation(const int64_t chatId, const int64_t userId);
};