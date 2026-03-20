#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <vector>

struct Product {
    int id = 0;
    std::string name;
    double price = 0.0;
    std::string unit;
    
    struct Balance {
        int storageId;
        std::string storageName;
        double quantity;
    };
    std::vector<Balance> balances;
    
    double getTotalQuantity() const {
        double total = 0;
        for (const auto& b : balances) {
            total += b.quantity;
        }
        return total;
    }
};

struct Storage {
    int id = 0;
    std::string name;
    std::string address;
    
    bool isEmpty() const { return id == 0; }
};

struct Transaction {
    int id = 0;
    std::string timestamp;
    enum class Type { RECEIPT, SHIPMENT } type;
    int productId = 0;
    int storageId = 0;
    double quantity = 0.0;
    std::string comment;
    
    std::string getTypeString() const {
        return type == Type::RECEIPT ? "ПРИХОД" : "РАСХОД";
    }
};

struct BalanceView {
    std::string storageName;
    std::string productName;
    double quantity;
    std::string unit;
};

#endif
