#pragma once
#include <tgbot/tgbot.h>
#include "logger.hpp"
#include "db.hpp"

extern TgBot::Bot *bot;

void setupCommands(Logger &logger);
void setupEvents(Logger &logger, UserDB &db);