#include "DatabaseManager.h"
#include "Queries.h"
#include <iostream>
#include <sstream>

using namespace std;

DatabaseManager::DatabaseManager(const string& path, shared_ptr<Logger> log) 
    : dbPath(path), db(nullptr), logger(log) {}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::executeSQL(const string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        if (logger) logger->error("SQL Error: " + string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int DatabaseManager::callback(void* data, int argc, char** argv, char** azColName) {
    vector<vector<string>>* result = static_cast<vector<vector<string>>*>(data);
    vector<string> row;
    for (int i = 0; i < argc; i++) {
        row.push_back(argv[i] ? argv[i] : "NULL");
    }
    result->push_back(row);
    return 0;
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        if (logger) logger->error("Can't open database: " + string(sqlite3_errmsg(db)));
        return false;
    }
    
    bool success = executeSQL(Queries::CREATE_STORAGES_TABLE) &&
                   executeSQL(Queries::CREATE_PRODUCTS_TABLE) &&
                   executeSQL(Queries::CREATE_BALANCES_TABLE) &&
                   executeSQL(Queries::CREATE_TRANSACTIONS_TABLE) &&
                   executeSQL(Queries::INSERT_DEFAULT_STORAGE);
    
    if (success && logger) logger->info("Database initialized successfully");
    return success;
}

vector<Product> DatabaseManager::getAllProducts() {
    vector<Product> products;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_ALL_PRODUCTS.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        Product currentProduct;
        int lastProductId = -1;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int productId = sqlite3_column_int(stmt, 0);
            
            if (productId != lastProductId) {
                if (lastProductId != -1) {
                    products.push_back(currentProduct);
                }
                currentProduct.id = productId;
                currentProduct.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                currentProduct.price = sqlite3_column_double(stmt, 2);
                currentProduct.unit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                currentProduct.balances.clear();
                lastProductId = productId;
            }
            
            Product::Balance balance;
            balance.storageId = sqlite3_column_int(stmt, 4);
            balance.storageName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            balance.quantity = sqlite3_column_double(stmt, 6);
            currentProduct.balances.push_back(balance);
        }
        
        if (lastProductId != -1) {
            products.push_back(currentProduct);
        }
    }
    
    sqlite3_finalize(stmt);
    return products;
}

Product DatabaseManager::getProduct(int id) {
    Product product;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_PRODUCT_BY_ID.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            product.id = sqlite3_column_int(stmt, 0);
            product.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            product.price = sqlite3_column_double(stmt, 2);
            product.unit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        }
    }
    
    sqlite3_finalize(stmt);
    return product;
}

bool DatabaseManager::addProduct(const Product& product) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::INSERT_PRODUCT.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, product.price);
        sqlite3_bind_text(stmt, 3, product.unit.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc == SQLITE_DONE) {
            int productId = sqlite3_last_insert_rowid(db);
            initProductBalances(productId);
            return true;
        }
    }
    
    return false;
}

bool DatabaseManager::updateProduct(const Product& product) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::UPDATE_PRODUCT.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, product.price);
        sqlite3_bind_text(stmt, 3, product.unit.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, product.id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

bool DatabaseManager::deleteProduct(int id) {
    string sql = "DELETE FROM products WHERE id = " + to_string(id);
    return executeSQL(sql);
}

bool DatabaseManager::productExists(int id) {
    string sql = "SELECT COUNT(*) FROM products WHERE id = " + to_string(id);
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return count > 0;
}

vector<Storage> DatabaseManager::getAllStorages() {
    vector<Storage> storages;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_ALL_STORAGES.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Storage storage;
            storage.id = sqlite3_column_int(stmt, 0);
            storage.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            storage.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            storages.push_back(storage);
        }
    }
    
    sqlite3_finalize(stmt);
    return storages;
}

Storage DatabaseManager::getStorage(int id) {
    Storage storage;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_STORAGE_BY_ID.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            storage.id = sqlite3_column_int(stmt, 0);
            storage.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            storage.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }
    }
    
    sqlite3_finalize(stmt);
    return storage;
}

bool DatabaseManager::addStorage(const Storage& storage) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::INSERT_STORAGE.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, storage.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, storage.address.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc == SQLITE_DONE) {
            int storageId = sqlite3_last_insert_rowid(db);
            initStorageBalances(storageId);
            return true;
        }
    }
    
    return false;
}

bool DatabaseManager::updateStorage(const Storage& storage) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::UPDATE_STORAGE.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, storage.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, storage.address.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, storage.id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

bool DatabaseManager::deleteStorage(int id) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::DELETE_STORAGE.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

bool DatabaseManager::storageExists(int id) {
    string sql = "SELECT COUNT(*) FROM storages WHERE id = " + to_string(id);
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return count > 0;
}

bool DatabaseManager::storageHasBalance(int id) {
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, Queries::CHECK_STORAGE_HAS_BALANCE.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return count > 0;
}

double DatabaseManager::getProductBalance(int productId, int storageId) {
    sqlite3_stmt* stmt;
    double quantity = 0;
    
    if (sqlite3_prepare_v2(db, Queries::SELECT_BALANCE.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, productId);
        sqlite3_bind_int(stmt, 2, storageId);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            quantity = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return quantity;
}

bool DatabaseManager::updateBalance(int productId, int storageId, double quantity) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::UPDATE_BALANCE.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, quantity);
        sqlite3_bind_int(stmt, 2, productId);
        sqlite3_bind_int(stmt, 3, storageId);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

bool DatabaseManager::initProductBalances(int productId) {
    auto storages = getAllStorages();
    bool success = true;
    
    for (const auto& storage : storages) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, Queries::INIT_PRODUCT_BALANCES.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, productId);
            sqlite3_bind_int(stmt, 2, storage.id);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                success = false;
            }
            sqlite3_finalize(stmt);
        }
    }
    
    return success;
}

bool DatabaseManager::initStorageBalances(int storageId) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::INIT_STORAGE_BALANCES.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, storageId);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

bool DatabaseManager::addTransaction(const Transaction& transaction) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::INSERT_TRANSACTION.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        string type = (transaction.type == Transaction::Type::RECEIPT) ? "RECEIPT" : "SHIPMENT";
        sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, transaction.productId);
        sqlite3_bind_int(stmt, 3, transaction.storageId);
        sqlite3_bind_double(stmt, 4, transaction.quantity);
        sqlite3_bind_text(stmt, 5, transaction.comment.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    return false;
}

vector<Transaction> DatabaseManager::getTransactionHistory(int limit) {
    vector<Transaction> transactions;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_TRANSACTIONS.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Transaction trans;
            trans.id = sqlite3_column_int(stmt, 0);
            trans.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            trans.type = (type == "RECEIPT") ? Transaction::Type::RECEIPT : Transaction::Type::SHIPMENT;
            trans.productId = sqlite3_column_int(stmt, 3);
            trans.storageId = sqlite3_column_int(stmt, 4);
            trans.quantity = sqlite3_column_double(stmt, 5);
            trans.comment = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
            
            transactions.push_back(trans);
        }
    }
    
    sqlite3_finalize(stmt);
    return transactions;
}

vector<BalanceView> DatabaseManager::getAllBalances() {
    vector<BalanceView> balances;
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, Queries::SELECT_ALL_BALANCES.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            BalanceView bv;
            bv.storageName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            bv.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            bv.quantity = sqlite3_column_double(stmt, 2);
            bv.unit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            balances.push_back(bv);
        }
    }
    
    sqlite3_finalize(stmt);
    return balances;
}
