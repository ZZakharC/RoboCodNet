#include "antispam.hpp"
#include "config.hpp"

bool SpamProtection::isSpaming(int64_t userId) {
    std::lock_guard<std::mutex> lock(mtx); // Блокируем доступ для других потоков

    std::time_t now = std::time(nullptr);
    auto &history = userHistory[userId]; // Получаем (или создаем) очередь пользователя

    // Удаляем устаревшие записи (которые вышли за пределы 60 секунд)
    while (!history.empty() && (now - history.front() > 60)) {
        history.pop_front();
    }

    // Добавляем текущее время
    history.push_back(now);

    // Проверяем превышение лимита
    if (history.size() > LIMIT_MESSAGES_IN_MINUTE)
        return true;

    return false;
}

void SpamProtection::clearUserHistory(int64_t userId) {
    std::lock_guard<std::mutex> lock(mtx);
    userHistory.erase(userId);
}