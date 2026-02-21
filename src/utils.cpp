#include <string_view>
#include <vector>
#include <string>
#include <unordered_set>
#include <regex>
#include <string_view>
#include <ranges>
#include <boost/algorithm/string.hpp>
#include "config.hpp"

// Хеш-таблица для поиска за O(1). Создается один раз при запуске.
const std::unordered_set<std::string> banSet(BAN_WORDS.begin(), BAN_WORDS.end());

// === ФУНКЦИИ ===

// Очистка строки от мусора
std::string trim(std::string_view s) {
    const auto first = s.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string_view::npos) return {};
    const auto last = s.find_last_not_of(" \t\n\r\f\v");
    return std::string(s.substr(first, (last - first + 1)));
}

// Функция для перевода UTF-8 строки в нижний регистр (поддержка RU + EN)
std::string to_lower(const std::string &str) {
    std::string res = str;
    for (size_t i = 0; i < res.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(res[i]);
        
        // Английский (ASCII)
        if (c >= 'A' && c <= 'Z') {
            res[i] = c + 32;
        }
        // Русские буквы в UTF-8 обычно занимают 2 байта.
        // Основной диапазон 'А'..'Я' (D0 90 .. D0 AF) -> 'а'..'п' (D0 B0 .. D0 CF)
        // 'Р'..'Я' (D0 B0 .. D0 BF) -> 'р'..'я' (D1 80 .. D1 8F)
        else if (c == 0xD0) {
            if (i + 1 < res.size()) {
                unsigned char next = static_cast<unsigned char>(res[i+1]);
                // Диапазон А-П (D0 90 - D0 9F)
                if (next >= 0x90 && next <= 0x9F)
                    res[i+1] = next + (char)0x20;

                // Диапазон Р-Я (D0 A0 - D0 AF)
                else if (next >= 0xA0 && next <= 0xAF) {
                    res[i] = (char)0xD1;      // меняем префикс байта
                    res[i+1] = next - (char)0x20; 
                }
                // Ё (D0 81) -> ё (D1 91)
                else if (next == 0x81) {
                    res[i] = (char)0xD1;
                    res[i+1] = (char)0x91;
                }
            }
        }
    }
    return res;
}

// Экранирования Markdown
std::string escapeMarkdown(const std::string &str) {
    std::string escaped;
    escaped.reserve(str.size() * 1.1);
    for (char c : str) {
        switch (c) {
            case '_': case '*': case '[': case ']': case '(': case ')': 
            case '~': case '`': case '>': case '#': case '+': case '-': 
            case '=': case '|': case '{': case '}': case '.': case '!':
                escaped += '\\';
                escaped += c;
                break;
            default:
                escaped += c;
        }
    }
    return escaped;
}

// Экранирование HTML
std::string escapeHtml(const std::string &str) {
    std::string escaped;
    escaped.reserve(str.size() * 1.1); // Выделяем память заранее 
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

// Экранирования текста
std::string escapeText(const std::string &str) {
    if (std::strncmp(MESSAGE_FORMAT,"Markdown", 8) == 0)
        return escapeMarkdown(str);
    else if (std::strcmp(MESSAGE_FORMAT, "HTML") == 0)
        return escapeHtml(str);
    else
        return str;
}

// Проверка строки на ссылки
bool hasLink(const std::string &str) {
    if (str.empty()) return false;
    static const std::regex url_regex(
        // Ловит: http, https, ftp, www, t.me, discord.gg
        R"((https?://|ftp://|www\.|t\.me/|discord\.gg|youtube\.com/)\S+)", 
        std::regex::icase | std::regex::optimize
    );
    
    return std::regex_search(str, url_regex);
}

// Проверка строки на запрещённые слова
bool hasBanWords(const std::string &str) {
    if (str.empty()) return false;

    std::vector<std::string_view> words;
    boost::split(
        words,
        str,
        boost::is_any_of(" .,!?;:()[]{}\"'\n\r\t-"),
        boost::token_compress_on
    );

    for (std::string_view rawWord : words) {
        if (rawWord.empty()) continue;

        std::string word = to_lower(std::string(rawWord));
        if (banSet.find(word) != banSet.end())
            return true;
    }
    return false;
}


// Получить текст из команды
std::string getTextInCommand(const std::string &str) {
    auto pos = str.find(' ');
    if (pos == std::string::npos)
        return ""; // текста нет
    return str.substr(pos + 1);
}
