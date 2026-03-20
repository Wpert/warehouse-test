#include "TransactionManager.h"

TransactionManager::TransactionManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log)
    : db(database), logger(log) {}

bool TransactionManager::executeTransaction(Transaction::Type type, int productId, int storageId,
                                           double quantity, const std::string& comment) {
    // Валидация
    auto idValidation = ValidationService::validateId(productId);
    if (!idValidation) {
        if (logger) logger->error("Invalid product ID: " + std::to_string(productId));
        return false;
    }
    
    idValidation = ValidationService::validateId(storageId);
    if (!idValidation) {
        if (logger) logger->error("Invalid storage ID: " + std::to_string(storageId));
        return false;
    }
    
    auto qtyValidation = ValidationService::validateQuantity(quantity);
    if (!qtyValidation) {
        if (logger) logger->error("Invalid quantity: " + std::to_string(quantity));
        return false;
    }
    
    // Проверка существования
    if (!db->productExists(productId)) {
        if (logger) logger->error("Product not found: ID=" + std::to_string(productId));
        return false;
    }
    
    if (!db->storageExists(storageId)) {
        if (logger) logger->error("Storage not found: ID=" + std::to_string(storageId));
        return false;
    }
    
    // Для отгрузки проверяем наличие
    if (type == Transaction::Type::SHIPMENT) {
        double currentBalance = db->getProductBalance(productId, storageId);
        if (currentBalance < quantity) {
            if (logger) logger->error("Insufficient balance: available=" + 
                                      std::to_string(currentBalance) + 
                                      ", requested=" + std::to_string(quantity));
            return false;
        }
    }
    
    // Транзакция
    db->executeSQL("BEGIN TRANSACTION");
    
    // Обновляем баланс
    double currentBalance = db->getProductBalance(productId, storageId);
    double newBalance = (type == Transaction::Type::RECEIPT) 
                       ? currentBalance + quantity 
                       : currentBalance - quantity;
    
    if (!db->updateBalance(productId, storageId, newBalance)) {
        db->executeSQL("ROLLBACK");
        if (logger) logger->error("Failed to update balance");
        return false;
    }
    
    // Добавляем запись в историю
    Transaction trans;
    trans.type = type;
    trans.productId = productId;
    trans.storageId = storageId;
    trans.quantity = quantity;
    trans.comment = comment;
    
    if (!db->addTransaction(trans)) {
        db->executeSQL("ROLLBACK");
        if (logger) logger->error("Failed to add transaction record");
        return false;
    }
    
    db->executeSQL("COMMIT");
    
    if (logger) {
        std::string typeStr = (type == Transaction::Type::RECEIPT) ? "Receipt" : "Shipment";
        logger->info(typeStr + " completed: product=" + std::to_string(productId) +
                    ", storage=" + std::to_string(storageId) +
                    ", quantity=" + std::to_string(quantity));
    }
    
    return true;
}

bool TransactionManager::receiveProduct(int productId, int storageId, double quantity, const std::string& comment) {
    return executeTransaction(Transaction::Type::RECEIPT, productId, storageId, quantity, comment);
}

bool TransactionManager::shipProduct(int productId, int storageId, double quantity, const std::string& comment) {
    return executeTransaction(Transaction::Type::SHIPMENT, productId, storageId, quantity, comment);
}

std::vector<Transaction> TransactionManager::getTransactionHistory(int limit) {
    return db->getTransactionHistory(limit);
}
