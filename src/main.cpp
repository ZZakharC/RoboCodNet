#include "bot.hpp"
#include "db.hpp"
#include "config.hpp"
#include "logger.hpp"
#include <clocale>

TgBot::Bot *bot = nullptr; 

int main()
{
    std::setlocale(LC_ALL, ""); 

    // Инициализируем бота
    bot = new TgBot::Bot(BOT_TOKEN);

    Logger logger(LOGS_FILE_NAME); // Создаём логгер
    
    try {
        logger.log(LogLevel::START, bot->getApi().getMe()->username);

        // База данных
        UserDB db(DB_FILE_NAME, logger);
        
        // Настройка бота
        setupCommands(logger); 
        setupEvents(logger, db);   

        bot->getApi().deleteWebhook(true);
        TgBot::TgLongPoll longPoll(*bot);
  
        while (true) longPoll.start();

    } catch (TgBot::TgException &e) { 
        logger.log(LogLevel::ERROR, e.what());
        delete bot; 
        return 1;
    } catch (...) { 
        logger.log(LogLevel::ERROR, "Unknown error occurred");
        delete bot;
        return 1;
    }

    delete bot; // Не забываем удалить бота, так как он создан через new
    return 0;
}