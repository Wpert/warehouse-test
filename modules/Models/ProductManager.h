#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include "../Database/DatabaseManager.h"
#include "../Services/ValidationService.h"
#include <memory>

class ProductManager {
private:
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<Logger> logger;
    
public:
    ProductManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log);
    
    std::vector<Product> getAllProducts();
    Product getProduct(int id);
    bool addProduct(const Product& product);
    bool updateProduct(const Product& product);
    bool deleteProduct(int id);
    bool productExists(int id);
};

#endif
