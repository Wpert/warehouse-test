#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "../Database/DatabaseManager.h"
#include "../Services/ConfigService.h"
#include "../Models/ProductManager.h"
#include "../Models/StorageManager.h"
#include "../Models/TransactionManager.h"
#include "../UI/ConsoleUI.h"
#include "../Utils/Logger.h"

class Application {
private:
    std::shared_ptr<ConfigService> config;
    std::shared_ptr<DatabaseManager> database;
    std::shared_ptr<ProductManager> productManager;
    std::shared_ptr<StorageManager> storageManager;
    std::shared_ptr<TransactionManager> transactionManager;
    std::shared_ptr<ConsoleUI> ui;
    std::shared_ptr<Logger> logger;
    
    bool initialize();
    void setupLogLevel();
    void shutdown();
    
public:
    Application();
    ~Application();
    
    int run();
};

#endif
