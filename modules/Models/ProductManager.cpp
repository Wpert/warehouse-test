#include "ProductManager.h"

ProductManager::ProductManager(std::shared_ptr<DatabaseManager> database, std::shared_ptr<Logger> log)
    : db(database), logger(log) {}

std::vector<Product> ProductManager::getAllProducts() {
    return db->getAllProducts();
}

Product ProductManager::getProduct(int id) {
    return db->getProduct(id);
}

bool ProductManager::addProduct(const Product& product) {
    auto validation = ValidationService::validateProduct(product);
    if (!validation) {
        if (logger) logger->error("Product validation failed: " + validation.message);
        return false;
    }
    
    bool result = db->addProduct(product);
    if (result && logger) {
        logger->info("Product added: " + product.name);
    }
    return result;
}

bool ProductManager::updateProduct(const Product& product) {
    auto validation = ValidationService::validateProduct(product);
    if (!validation) {
        if (logger) logger->error("Product validation failed: " + validation.message);
        return false;
    }
    
    bool result = db->updateProduct(product);
    if (result && logger) {
        logger->info("Product updated: ID=" + std::to_string(product.id));
    }
    return result;
}

bool ProductManager::deleteProduct(int id) {
    auto product = db->getProduct(id);
    if (product.id == 0) {
        if (logger) logger->error("Product not found: ID=" + std::to_string(id));
        return false;
    }
    
    if (product.getTotalQuantity() > 0) {
        if (logger) logger->error("Cannot delete product with balance: ID=" + std::to_string(id));
        return false;
    }
    
    bool result = db->deleteProduct(id);
    if (result && logger) {
        logger->info("Product deleted: ID=" + std::to_string(id));
    }
    return result;
}

bool ProductManager::productExists(int id) {
    return db->productExists(id);
}
