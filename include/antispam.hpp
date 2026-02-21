#pragma once
#include <map>
#include <deque>
#include <mutex>
#include <ctime>
#include <cstdint>

class SpamProtection {
private:
    // Хранит историю времени сообщений для каждого пользователя
    // Key: UserID, Value: Очередь временных меток
    std::map<int64_t, std::deque<std::time_t>> userHistory;
    
    // Мьютекс для защиты карты от одновременного доступа из разных потоков
    std::mutex mtx;

public:
    // Возвращает true, если пользователь спамит
    bool isSpaming(int64_t userId);
    
    // Очистка истории конкретного пользователя (например, после наказания)
    void clearUserHistory(int64_t userId);
};