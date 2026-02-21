#include "antispam.hpp"
#include "config.hpp"

bool SpamProtection::isSpaming(int64_t chatId, int64_t userId) {
    // std::lock_guard<std::mutex> lock(mtx); // Блокируем доступ для других потоков

    std::time_t now = std::time(nullptr);
    auto key = std::make_pair(chatId, userId);
    auto &history = userHistory[key]; // Получаем (или создаем) очередь пользователя

    // Удаляем устаревшие записи (которые вышли за пределы 60 секунд)
    while (!history.empty() && (now - history.front() > 60)) {
        history.pop_front();
    }

    // Добавляем текущее время
    history.push_back(now);

    // === ЗАЩИТА ОТ УТЕЧКИ ПАМЯТИ ===
    // Если в словаре скопилось больше 1000 пользователей, проводим генеральную уборку
    if (userHistory.size() > 1000) {
        for (auto it = userHistory.begin(); it != userHistory.end(); ) {
            // Очищаем старые метки
            while (!it->second.empty() && (now - it->second.front() > 60))
                it->second.pop_front();
            // Если после очистки очередь пуста — удаляем пользователя из памяти
            if (it->second.empty())
                it = userHistory.erase(it);
            else ++it;
        }
    }

    // Проверяем превышение лимита
    if (history.size() > LIMIT_MESSAGES_IN_MINUTE)
        return true;

    return false;
}

void SpamProtection::clearUserHistory(int64_t chatId, int64_t userId) {
    // std::lock_guard<std::mutex> lock(mtx);
    userHistory.erase(std::make_pair(chatId, userId));
}