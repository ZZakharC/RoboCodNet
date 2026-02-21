#include "config.hpp"
#include "bot.hpp"
#include "tg_tools.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "antispam.hpp"
#include <cstdint>
#include <string>
#include <format>
#include <tgbot/tools/StringTools.h>

// Создаем глобальный экземпляр (или static внутри функции, как вам удобнее)
static SpamProtection spamFilter;

// Настройка событий
void setupEvents(Logger &logger, UserDB &db) {
    bot->getEvents().onAnyMessage([&logger, &db](TgBot::Message::Ptr message) {
        if (!message || !message->chat) return;
        int64_t chatId = message->chat->id;
        int64_t userId = (message->from && !message->from->isBot) ? message->from->id : 0;
        bool isAdmin = userId == 0 || isUserAdmin(chatId, userId);

        // Прикрепления сообщения
        if (message->pinnedMessage) {
            // Удаляем
            deleteMessage(logger, message->chat->id, message->messageId);
            return;
        }

        // Проверка на Спам
        if (
            !isAdmin &&                   // Если не Админ И
            spamFilter.isSpaming(chatId, userId) // Превышен ли лимит сообщений 
        ) {
            int muteTime = db.getViolation(chatId, userId) * MUTE_TIME_SPAM;
            // Мутим пользователя
            muteUser(chatId, userId, muteTime);

            // Уведомляем чат (без уведомления)
            bot->getApi().sendMessage(chatId, 
                std::format("🐢 {} превысил(а) лимит сообщений в минуту. Пользователь *заглушен* на {} сек", getUserMention(message->from), muteTime),
                nullptr, nullptr, nullptr, MESSAGE_FORMAT, true
            );

            spamFilter.clearUserHistory(chatId, userId); // Очищаем историю
            logger.log(LogLevel::WARNING, std::format("{} {}   Mute user, reason: spam", chatId, userId)); // Лог о муте
            db.incrementViolation(chatId, userId); // Добавляем 1 к нарушения в DB

            return;
        }
        
        // Обработка команд команды
        if (StringTools::startsWith(message->text, "/start") || StringTools::startsWith(message->text, "/rules") || StringTools::startsWith(message->text, "get_task")) return;

        // Присоединение Пользователя
        if (!message->newChatMembers.empty()) {
            for (auto &user : message->newChatMembers) { // Пользователь (перебираем всех)
                deleteMessage(logger, chatId, message->messageId); // Удаление сообщения от Telegram

                if (user->isBot) continue; // Не приветствуем ботов

                logger.log(LogLevel::INFO, std::format("{} {}   User join", chatId, user->id)); // Лог
                bot->getApi().sendMessage(chatId,std::format("Добро пожаловать в группу {} 🎉", getUserMention(message->from)), nullptr, nullptr, nullptr, MESSAGE_FORMAT, true); // Сообщение без уведомления
            }
            return;
        }

        // Выход Пользователя
        if (message->leftChatMember) {
            // Удаляем сообщение от Telegram
            deleteMessage(logger, chatId, message->messageId); // Удаление сообщения от Telegram
            // Лог
            logger.log(LogLevel::INFO, std::format("{} {}   User left", chatId, message->leftChatMember->id)); // Лог
            // Отправка сообщения об уходе (отключено)
            //bot->getApi().sendMessage(message->chat->id, std::format("Пользователь {} выбрал быть счастливым.", getUserMention(message)), nullptr, nullptr, nullptr, MESSAGE_FORMAT); // Сообщения
            return;
        }
        
        // Логирование сообщения 
        logger.log(LogLevel::INFO, std::format("{} {} {}   {}    {}", chatId, userId, message->messageId, message->text, message->caption)); 

        // === Модерация сообщений ===

        // Проверка если пользователь админ то его сообщения не модерируется
        if (isAdmin) return; 

        /// Удаление ссылок
        if ((hasLink(message->text)            || // Проверка текста 
             hasLink(message->caption)         || // Проверка описания
             (message->poll && hasLink(message->poll->question))) // Проверка опросов (только заголовка)
        ) { 
            deleteMessage(logger, chatId, message->messageId); // Удаление сообщения с ссылкой
            
            // Отправка сообщения в чат об удалении (без уведомления)
            bot->getApi().sendMessage(
                chatId,
                std::format("⛔ {} отправил(а) ссылку в чат. Сообщение удалено", getUserMention(message->from)), nullptr, nullptr, nullptr, MESSAGE_FORMAT, true
            );

            logger.log(LogLevel::WARNING, std::format("{} {}   Deleted message, reason: sending link", chatId, message->messageId)); // Лог о удалении

            return;
        }

        /// Удаления запрещённых слов 
        if (hasBanWords(message->text)        || // Проверка текста 
            hasBanWords(message->caption)     || // Проверка описания
            (message->poll && hasBanWords(message->poll->question)) // Проверка опросов (только заголовка)
        ){
            logger.log(LogLevel::WARNING, std::format("{} {}   Deleted message, reason: ban words", chatId, message->messageId)); // Лог о удалении
            deleteMessage(logger, chatId, message->messageId); // Удаление сообщения 
            
            // Мут пользователя
            int32_t muteTime = MUTE_TIME_BAN_WORDS * db.getViolation(chatId, userId);
            muteUser(chatId, message->from->id, muteTime); // Выдаём мут не время * количество нарушений

            // Отправка сообщения в чат об удалении (без уведомления)
            bot->getApi().sendMessage(
                chatId,
                std::format("❌ {} отправил(а) сообщения с запрещённым словом в чат. *Сообщение удалено*, *пользователь заглушен* на {} сек", getUserMention(message->from), muteTime), nullptr, nullptr, nullptr, MESSAGE_FORMAT, true
            );

            // Откладываем для быстрого ответа от бота
            logger.log(LogLevel::WARNING, std::format("{} {}   Mute user, reason: ban words", chatId, userId)); // Лог о муте
            db.incrementViolation(chatId, userId); // Добавляем 1 к нарушения в DB

            return;
        }
    });
}