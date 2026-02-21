#include "db.hpp"
#include <stdexcept>
#include <format>

// Конструктор UserDB
UserDB::UserDB(const std::string &db_file, Logger &log) : logger(log) {
    if (sqlite3_open(db_file.c_str(), &db)) {
        logger.log(LogLevel::ERROR, std::format("DB   OPEN: {}", sqlite3_errmsg(db)));
        sqlite3_close(db);
        throw std::runtime_error("OPEN DB");
    }

    const char* createTableSQL =
    "CREATE TABLE IF NOT EXISTS violations ("
        "chatId INTEGER NOT NULL,"
        "userId INTEGER NOT NULL,"
        "violation INTEGER NOT NULL DEFAULT 0,"
        "PRIMARY KEY(chatId, userId)"
        ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
        logger.log(LogLevel::ERROR, std::format("DB   CREATE TABLE: {}", errMsg));
        sqlite3_free(errMsg);
        throw std::runtime_error(errMsg);
    }

    const char* upsertSQL =
        "INSERT INTO violations (chatId, userId, violation) "
        "VALUES (?, ?, 1) "
        "ON CONFLICT(chatId, userId) "
        "DO UPDATE SET violation = violation + 1;";
    
    if (sqlite3_prepare_v2(db, upsertSQL, -1, &stmtIncrement, nullptr) != SQLITE_OK) {
        logger.log(LogLevel::ERROR, std::format("DB   PREPARE INC: {}", sqlite3_errmsg(db)));
        if (stmtGetViolation) sqlite3_finalize(stmtGetViolation);
        if (db) sqlite3_close(db);
        throw std::runtime_error("PREPARE INC");
    }

    const char* selectSQL = "SELECT violation FROM violations WHERE chatId = ? AND userId = ?;";

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmtGetViolation, nullptr) != SQLITE_OK) {
        logger.log(LogLevel::ERROR, std::format("DB   PREPARE GET: {}", sqlite3_errmsg(db)));
        if (stmtIncrement) sqlite3_finalize(stmtIncrement);
        if (db) sqlite3_close(db);
        throw std::runtime_error("PREPARE GET");
    }

    logger.log(LogLevel::WARNING, "DB   OPEN");
}

// Деструктор UserDB
UserDB::~UserDB() {
    // Сначала удаляем подготовленные запросы
    if (stmtIncrement) sqlite3_finalize(stmtIncrement);
    if (stmtGetViolation) sqlite3_finalize(stmtGetViolation);
    // Потом закрываем базу
    if (db) sqlite3_close(db);
    logger.log(LogLevel::WARNING, "DB   CLOSE");
}

// Увеличить количество нарушений на 1
void UserDB::incrementViolation(const int64_t chatId, const int64_t userId) {
    sqlite3_bind_int64(stmtIncrement, 1, chatId);
    sqlite3_bind_int64(stmtIncrement, 2, userId);
    
    sqlite3_step(stmtIncrement);

    // Сброс состояния для следующего вызова
    sqlite3_reset(stmtIncrement);
    sqlite3_clear_bindings(stmtIncrement);

    logger.log(LogLevel::WARNING, "DB   INC VIOLATION");
}

// Получить количество нарушений
int32_t UserDB::getViolation(const int64_t chatId, const int64_t userId) {
    sqlite3_bind_int64(stmtGetViolation, 1, chatId);
    sqlite3_bind_int64(stmtGetViolation, 2, userId);

    int ret = 1; // НЕ 0 потому что это множитель!

    if (sqlite3_step(stmtGetViolation) == SQLITE_ROW)
        ret = sqlite3_column_int(stmtGetViolation, 0) + 1; // +1 чтобы 1 нарушения не считалось также как 0 нарушений

    // Сброс состояния для следующего вызова
    sqlite3_reset(stmtGetViolation);
    sqlite3_clear_bindings(stmtGetViolation);

    logger.log(LogLevel::WARNING, "DB   GET VIOLATION");

    return ret;
}