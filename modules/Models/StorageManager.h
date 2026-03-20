#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include "../Database/DatabaseManager.h"
#include "../Services/ValidationService.h"
#include <memory>

class StorageManager {
private:
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<Logger> logger;
    
public:
    StorageManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log);
    
    std::vector<Storage> getAllStorages();
    Storage getStorage(int id);
    bool addStorage(const Storage& storage);
    bool updateStorage(const Storage& storage);
    bool deleteStorage(int id);
    bool storageExists(int id);
    bool hasBalances(int storageId);
    
    double getProductBalance(int productId, int storageId);
    std::vector<BalanceView> getAllBalances();
};

#endif
