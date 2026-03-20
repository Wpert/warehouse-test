#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include "../Database/DatabaseManager.h"
#include "../Services/ValidationService.h"
#include <memory>

class TransactionManager {
private:
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<Logger> logger;
    
    bool executeTransaction(Transaction::Type type, int productId, int storageId, 
                           double quantity, const std::string& comment);
    
public:
    TransactionManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log);
    
    bool receiveProduct(int productId, int storageId, double quantity, const std::string& comment = "");
    bool shipProduct(int productId, int storageId, double quantity, const std::string& comment = "");
    
    std::vector<Transaction> getTransactionHistory(int limit = 100);
};

#endif
