#pragma once
#include <string>

// Очистка строки от мусора
std::string trim(std::string_view s);

// Перевод строки в нижний регистр
std::string to_lower(const std::string &str);

// Экранирования текста (автоматически подбирает формат)
std::string escapeText(const std::string &str);

// Проверка строки на ссылки 
bool hasLink(const std::string &str);

// Проверка строки на запрещённые слова
bool hasBanWords(const std::string &str);

// Получить текст из команды
std::string getTextInCommand(const std::string &str);