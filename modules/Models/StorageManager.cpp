#include "StorageManager.h"

StorageManager::StorageManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log)
    : db(database), logger(log) {}

std::vector<Storage> StorageManager::getAllStorages() {
    return db->getAllStorages();
}

Storage StorageManager::getStorage(int id) {
    return db->getStorage(id);
}

bool StorageManager::addStorage(const Storage& storage) {
    auto validation = ValidationService::validateStorage(storage);
    if (!validation) {
        if (logger) logger->error("Storage validation failed: " + validation.message);
        return false;
    }
    
    bool result = db->addStorage(storage);
    if (result && logger) {
        logger->info("Storage added: " + storage.name);
    }
    return result;
}

bool StorageManager::updateStorage(const Storage& storage) {
    auto validation = ValidationService::validateStorage(storage);
    if (!validation) {
        if (logger) logger->error("Storage validation failed: " + validation.message);
        return false;
    }
    
    bool result = db->updateStorage(storage);
    if (result && logger) {
        logger->info("Storage updated: ID=" + std::to_string(storage.id));
    }
    return result;
}

bool StorageManager::deleteStorage(int id) {
    if (db->storageHasBalance(id)) {
        if (logger) logger->error("Cannot delete storage with balance: ID=" + std::to_string(id));
        return false;
    }
    
    bool result = db->deleteStorage(id);
    if (result && logger) {
        logger->info("Storage deleted: ID=" + std::to_string(id));
    }
    return result;
}

bool StorageManager::storageExists(int id) {
    return db->storageExists(id);
}

bool StorageManager::hasBalances(int storageId) {
    return db->storageHasBalance(storageId);
}

double StorageManager::getProductBalance(int productId, int storageId) {
    return db->getProductBalance(productId, storageId);
}

std::vector<BalanceView> StorageManager::getAllBalances() {
    return db->getAllBalances();
}
