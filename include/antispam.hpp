#pragma once
#include <map>
#include <deque>
#include <mutex>
#include <ctime>
#include <cstdint>

class SpamProtection {
private:
    // Хранит историю времени сообщений для каждого пользователя
    // Key: {ChatID, UserID}, Value: Очередь временных меток
    std::map<std::pair<int64_t, int64_t>, std::deque<std::time_t>> userHistory;
    
    // Мьютекс для защиты карты от одновременного доступа из разных потоков
    // std::mutex mtx; Не используем mutex ОН СЛИШКОМ СИЛЬНО ЗАМЕДЛЯЕТ БОТА 

public:
    // Возвращает true, если пользователь спамит
    bool isSpaming(int64_t chatId, int64_t userId);
    
    // Очистка истории конкретного пользователя (например, после наказания)
    void clearUserHistory(int64_t chatId, int64_t userId);
};