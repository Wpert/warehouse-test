#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <functional>
#include <map>
#include <memory>
#include "../Models/ProductManager.h"
#include "../Models/StorageManager.h"
#include "../Models/TransactionManager.h"
#include "../Utils/Logger.h"
#include "MenuItems.h"
#include "DisplayFormatter.h"

class ConsoleUI {
private:
    std::shared_ptr<ProductManager> productManager;
    std::shared_ptr<StorageManager> storageManager;
    std::shared_ptr<TransactionManager> transactionManager;
    std::shared_ptr<Logger> logger;
    
    std::map<int, MenuItem> mainMenu;
    std::map<int, MenuItem> storageMenu;
    
    bool isRunning;
    
    void initializeMenus();
    void clearScreen();
    void waitForUser();
    void printHeader(const std::string& title);
    
    // Обработчики команд
    void handleViewProducts();
    void handleAddProduct();
    void handleEditProduct();
    void handleDeleteProduct();
    void handleReceiveGoods();
    void handleShipGoods();
    void handleViewHistory();
    void handleManageStorages();
    void handleViewBalances();
    
    // Подменю управления складами
    void handleListStorages();
    void handleAddStorage();
    void handleEditStorage();
    void handleDeleteStorage();
    
    // Вспомогательные методы
    int readInt(const std::string& prompt);
    double readDouble(const std::string& prompt);
    std::string readString(const std::string& prompt);
    bool confirmAction(const std::string& message);
    
public:
    ConsoleUI(std::shared_ptr<ProductManager> pm, 
              std::shared_ptr<StorageManager> sm,
              std::shared_ptr<TransactionManager> tm,
              std::shared_ptr<Logger> log);
    
    void showWelcomeMessage();
    void runMainLoop();
};

#endif
