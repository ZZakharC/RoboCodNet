#include "tg_tools.hpp"
#include "bot.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <cstdint>
#include <map>
#include <ctime>

// Структура для кэша
struct AdminCacheEntry {
    bool isAdmin;
    time_t expiresAt;
};

// Имя пользователя с ссылкой Markdown
std::string getUserMention(TgBot::User::Ptr user) {
    if (!user) return std::string("Пользователь");
    return std::format("[{0}](tg://user?id={1})", escapeText(user->firstName), user->id);
};

// Удаление сообщения
void deleteMessage(Logger &logger, int64_t chatId, int32_t messageId) {
    try {
        bot->getApi().deleteMessage(chatId, messageId);
    } catch (const TgBot::TgException& e) {
        logger.log(LogLevel::ERROR, e.what());
        return;
    }
}

// Закрепить сообщения (на время)
void pinMessage(Logger &logger, int64_t chatId, int32_t messageId, uint32_t seconds) {
    // Закрепляем
    bot->getApi().pinChatMessage(chatId, messageId, true);
    logger.log(LogLevel::WARNING, std::format("{} {}   Pin message", chatId, messageId));

    // Таймер
    std::thread([&, chatId, messageId, seconds]() {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        try {
            bot->getApi().unpinChatMessage(chatId, messageId);
            logger.log(LogLevel::WARNING, std::format("{} {}   Unpin message", chatId, messageId));
        } catch (...) {} // Ошибки игнорируем
    }).detach();
}

// Проверка пользователя на Админа
bool isUserAdmin(int64_t chatId, int64_t userId, bool useCache) {
    // Статическая переменная сохраняется между вызовами функции
    // Ключ: пара {chatId, userId}, Значение: статус и время
    static std::map<std::pair<int64_t, int64_t>, AdminCacheEntry> cache;
    
    time_t now = std::time(nullptr);
    auto key = std::make_pair(chatId, userId);

    // Защита от бесконечной утечки памяти (Memory Leak)
    if (cache.size() > 5000)
        cache.clear();

    // Проверяем кэш
   if (useCache) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            if (it->second.expiresAt > now)
                return it->second.isAdmin; // Возвращаем сохраненное значение
            else
                cache.erase(it); // Данные устарели, удаляем
        }
    }

    // Если в кэше нет делаем запрос
    bool isAdmin = false;

    try {
        auto member = bot->getApi().getChatMember(chatId, userId);
        if (member->status == "administrator" || member->status == "creator")
            isAdmin = true;
    } catch (...) {
        isAdmin = false; // При ошибке считаем, что не админ
    }

    // Сохраняем в кэш на TIME_CACHE_IS_ADMIN секунд
    cache[key] = {isAdmin, now + TIME_CACHE_IS_ADMIN};

    return isAdmin;
}

// Замутить пользователя на время
void muteUser(int64_t chatId, int64_t userId, int timeSeconds) {
    auto permissions = std::make_shared<TgBot::ChatPermissions>();
    permissions->canSendMessages = false;

    uint32_t untilDate = std::time(nullptr) + timeSeconds;

    bot->getApi().restrictChatMember(
        chatId,
        userId,
        permissions,
        untilDate
    );
}

// Снять мут
void unmuteUser(int64_t chatId, int64_t userId) {
    auto permissions = std::make_shared<TgBot::ChatPermissions>();
    permissions->canSendMessages = true;
    permissions->canSendPhotos = true;
    permissions->canSendOtherMessages = true;
    permissions->canSendAudios = true;
    permissions->canSendDocuments = true;
    permissions->canSendPolls = true;
    permissions->canSendVideoNotes = true;
    permissions->canSendVoiceNotes = true;

    bot->getApi().restrictChatMember(
        chatId,
        userId,
        permissions,
        0
    );
}
