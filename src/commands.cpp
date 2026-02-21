#include "bot.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "tg_tools.hpp"
#include "utils.hpp"
#include <string>
#include <format>
#include <sstream>
#include <utility>

void status(const TgBot::Message::Ptr message) {
    // Отправляем сообщения (без уведомления)
    bot->getApi().sendMessage(message->chat->id, "🟢 Бот стабильно работает", nullptr, nullptr, nullptr, MESSAGE_FORMAT, true);
}

void parse_quiz(const std::string &str,
           std::string &question,
           int &answer,
           std::vector<std::string> &options)
{
    std::stringstream ss(str);
    std::string token;

    // Очищаем options
    options.clear();

    // Первое значение Заголовок
    if (std::getline(ss, question, '|')) {
        question = trim(question);
        // Второе значение номер правильного ответа
        if (std::getline(ss, token, '|')) {
            try {
                answer = std::stoi(trim(token)) - 1; // -1 потому что tgbot читает с 0, а мы считаем с 1
            } catch (...) { answer = 0; }
            
            // Остальные значения
            while (std::getline(ss, token, '|')) {
                token = trim(token);
                if (!token.empty())
                    options.push_back(token);
            }
        }
    }

    if (answer+1 > static_cast<int>(options.size())) answer = 0;
}

// Настройка команд
void setupCommands(Logger &logger)
{
    bot->getEvents().onCommand("start",  status);
    bot->getEvents().onCommand("status", status);

    // Правила
    bot->getEvents().onCommand("rules", [&logger](TgBot::Message::Ptr message)
    {
        sendMessage(logger, message->chat->id, std::format("📖 [Ссылка на Правила]({})", RULES_GROUP_LINK), nullptr, true);
    });
    
    // Удаления
    bot->getEvents().onCommand("delete", [&logger](TgBot::Message::Ptr message)
    {
        // Получаем ID нарушителя и чата
        int64_t chatId = message->chat->id;
        
        // Проверка условий
        bool isAdmin =
            (!message->from || message->from->isBot
            || isUserAdmin(chatId, message->from->id));

        if (!isAdmin) return; // Если не админ

        // Удаляем сообщения с командой /mute
        deleteMessage(logger, chatId, message->messageId);

        // Проверка условий
        bool invalid =
            !message->replyToMessage; // Сообщения не является ответом

        if (invalid) return;

        deleteMessage(logger, chatId, message->replyToMessage->messageId);
    });

    // Мут
    bot->getEvents().onCommand("mute", [&logger](TgBot::Message::Ptr message)
    {
        // Получаем ID нарушителя и чата
        int64_t chatId = message->chat->id;
        
        // Проверка условий
        bool isAdmin =
            (!message->from || message->from->isBot
            || isUserAdmin(chatId, message->from->id));

        if (!isAdmin) return; // Если не админ

        // Удаляем сообщения с командой /mute
        deleteMessage(logger, chatId, message->messageId);

        // Проверка условий
        bool invalid =
            !message->replyToMessage || // Сообщения не является ответом
            !message->replyToMessage->from || // У нарушителя нет from
            message->replyToMessage->from->isBot; // Нарушитель бот

        if (invalid) return;


        std::stringstream text(getTextInCommand(message->text));
        std::string timeStr;
        text >> timeStr; // считываем время

        int time = MUTE_TIME_DEFAULT; // дефолтное время
        
        if (!timeStr.empty()) {
            try {
                time = std::stoi(timeStr);
            } catch (...) {}
        }

        // Мутим
        muteUser(chatId, message->replyToMessage->from->id, time);

        // Отправляем сообщения (без уведомления)
        sendMessage(logger, chatId, std::format("{} заглушил(а) пользователя {} на {} сек", getUserMention(message->from), getUserMention(message->replyToMessage->from), time), nullptr, true);
    });

    // УнМут
    bot->getEvents().onCommand("unmute", [&logger](TgBot::Message::Ptr message)
    {
        // Получаем ID нарушителя и чата
        int64_t chatId = message->chat->id;

        // Проверка условий
        bool isAdmin =
            (!message->from || message->from->isBot
            || isUserAdmin(chatId, message->from->id));

        if (!isAdmin) return; // Если не админ
        
        // Удаляем сообщения с командой /mute
        deleteMessage(logger, chatId, message->messageId);

        // Проверка условий
        bool invalid =
            !message->replyToMessage || // Сообщения не является ответом
            !message->replyToMessage->from || // У нарушителя нет from
            message->replyToMessage->from->isBot; // Нарушитель бот

        if (invalid) return;

        // Мутим
        unmuteUser(chatId, message->replyToMessage->from->id);

        // Отправляем сообщения (без уведомления)
        sendMessage(logger, chatId, std::format("{} снял(а) заглушение с пользователя {}", getUserMention(message->from), getUserMention(message->replyToMessage->from)), nullptr, true);
    });

    // Квиз
    bot->getEvents().onCommand("quiz", [&logger](TgBot::Message::Ptr message)
    {
        // Проверка условий
        bool isAdmin =
            (!message->from || message->from->isBot
            || isUserAdmin(message->chat->id, message->from->id));

        if (!isAdmin) return; // Если не админ

        // Удаляем сообщение
        deleteMessage(logger, message->chat->id, message->messageId);

        // Текст сообщения
        std::string text = getTextInCommand(message->text);

        // Пропускаем пустые сообщения
        if (text.empty()) return;

        std::string question;             // Заголовок опроса
        int answer = 0;                   // Заголовок опроса
        std::vector<std::string> options; // Варианты ответа
        
        // Парсим question, answer и options
        parse_quiz(text, question, answer, options);

        // Если заголовок или варианты пустые - скип
        if (question.empty() || options.size() < 2) return;
                    
        // Отправляем опрос (не анонимный, с уведомления, в режиме quiz)
        auto new_message = bot->getApi().sendPoll(message->chat->id, question, options, false, nullptr, nullptr, false, "quiz", false, answer);

        pinMessage(logger, new_message->chat->id, new_message->messageId, TIME_PIN_QUIZ);
    });

    // Задача
    bot->getEvents().onCommand("task", [&logger](TgBot::Message::Ptr message)
    {
        // Проверка условий
        bool isAdmin =
            (!message->from || message->from->isBot
            || isUserAdmin(message->chat->id, message->from->id));

        if (!isAdmin) return; // Если не админ

        // Удаляем сообщение
        deleteMessage(logger, message->chat->id, message->messageId);

        // Текст сообщения
        std::string text = getTextInCommand(message->text);

        // Пропускаем пустые сообщения
        if (text.empty()) return;

        auto new_message = sendMessage(logger, message->chat->id, text, nullptr);
        
        pinMessage(logger, new_message->chat->id, new_message->messageId, TIME_PIN_TASK);
    });

}