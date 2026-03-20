#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <memory>

// Добавляем проверку наличия sqlite3.h
#ifndef SQLITE3_INCLUDE_DIR
    #include <sqlite3.h>
#else
    #include <sqlite3.h>
#endif

#include "Entities.h"
#include "../Utils/Logger.h"

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    std::shared_ptr<Logger> logger;

public:
    DatabaseManager(const std::string& path, std::shared_ptr<Logger> log);
    ~DatabaseManager();

    bool initialize();
    bool executeSQL(const std::string& sql);
    static int callback(void* data, int argc, char** argv, char** azColName);
    
    // Продукты
    std::vector<Product> getAllProducts();
    Product getProduct(int id);
    bool addProduct(const Product& product);
    bool updateProduct(const Product& product);
    bool deleteProduct(int id);
    bool productExists(int id);
    
    // Склады
    std::vector<Storage> getAllStorages();
    Storage getStorage(int id);
    bool addStorage(const Storage& storage);
    bool updateStorage(const Storage& storage);
    bool deleteStorage(int id);
    bool storageExists(int id);
    bool storageHasBalance(int id);
    
    // Транзакции
    bool addTransaction(const Transaction& transaction);
    std::vector<Transaction> getTransactionHistory(int limit = 100);
    
    // Остатки
    double getProductBalance(int productId, int storageId);
    bool updateBalance(int productId, int storageId, double quantity);
    bool initProductBalances(int productId);
    bool initStorageBalances(int storageId);
    std::vector<BalanceView> getAllBalances();
};

#endif
