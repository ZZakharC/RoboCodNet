#pragma once

#include "bot.hpp"
#include "logger.hpp"
#include <cstdint>

// Имя пользователя с ссылкой Markdown
std::string getUserMention(TgBot::User::Ptr user);

// Удаления сообщения (с обёрткой try-catch)
void deleteMessage(Logger &logger, int64_t chatId, int32_t messageId);

// Закрепить сообщения (на время)
void pinMessage(Logger &logger, int64_t chatId, int32_t messageId, uint32_t seconds);

// Проверка является ли пользователь админом
bool isUserAdmin(int64_t chatId, int64_t userId, bool useCache = true);

// Мут пользователя на время (в секундах) 
void muteUser(int64_t chatId, int64_t userId, int timeSeconds);

// Снятия мута
void unmuteUser(int64_t chatId, int64_t userId); 