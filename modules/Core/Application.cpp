#include "Application.h"
#include <iostream>

Application::Application() {
    // Создаем конфиг первым
    config = std::make_shared<ConfigService>("config.json");
    
    // Создаем логгер
    logger = std::make_shared<Logger>(config->getLogFile());
    setupLogLevel();
}

Application::~Application() {
    shutdown();
}

void Application::setupLogLevel() {
    std::string level = config->getLogLevel();
    if (level == "debug") {
        logger->setMinLevel(LogLevel::DEBUG);
    } else if (level == "warning") {
        logger->setMinLevel(LogLevel::WARNING);
    } else if (level == "error") {
        logger->setMinLevel(LogLevel::ERROR);
    } else {
        logger->setMinLevel(LogLevel::INFO);
    }
}

bool Application::initialize() {
    logger->info("Инициализация приложения...");
    
    try {
        // Инициализация базы данных
        database = std::make_shared<DatabaseManager>(config->getDatabasePath(), logger);
        if (!database->initialize()) {
            logger->error("Ошибка инициализации базы данных");
            return false;
        }
        
        // Создание менеджеров
        productManager = std::make_shared<ProductManager>(database, logger);
        storageManager = std::make_shared<StorageManager>(database, logger);
        transactionManager = std::make_shared<TransactionManager>(database, logger);
        
        // Создание UI
        ui = std::make_shared<ConsoleUI>(productManager, storageManager, 
                                         transactionManager, logger);
        
        logger->info("Приложение успешно инициализировано");
        return true;
        
    } catch (const std::exception& e) {
        logger->error(std::string("Ошибка инициализации: ") + e.what());
        return false;
    }
}

void Application::shutdown() {
    logger->info("Завершение работы приложения");
}

int Application::run() {
    if (!initialize()) {
        std::cerr << "Не удалось инициализировать приложение. Проверьте логи." << std::endl;
        return 1;
    }
    
    ui->showWelcomeMessage();
    ui->runMainLoop();
    
    return 0;
}
