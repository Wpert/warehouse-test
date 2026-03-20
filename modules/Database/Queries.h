#ifndef QUERIES_H
#define QUERIES_H

#include <string>

namespace Queries {

    const std::string CREATE_STORAGES_TABLE = R"(
        CREATE TABLE IF NOT EXISTS storages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            address TEXT
        );
    )";
    
    const std::string CREATE_PRODUCTS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            price REAL DEFAULT 0,
            unit TEXT DEFAULT 'шт'
        );
    )";
    
    const std::string CREATE_BALANCES_TABLE = R"(
        CREATE TABLE IF NOT EXISTS balances (
            product_id INTEGER,
            storage_id INTEGER,
            quantity REAL DEFAULT 0,
            PRIMARY KEY (product_id, storage_id),
            FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
            FOREIGN KEY (storage_id) REFERENCES storages(id) ON DELETE CASCADE
        );
    )";
    
    const std::string CREATE_TRANSACTIONS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            type TEXT CHECK(type IN ('RECEIPT', 'SHIPMENT')),
            product_id INTEGER,
            storage_id INTEGER,
            quantity REAL,
            comment TEXT,
            FOREIGN KEY (product_id) REFERENCES products(id),
            FOREIGN KEY (storage_id) REFERENCES storages(id)
        );
    )";
    
    const std::string INSERT_DEFAULT_STORAGE = R"(
        INSERT OR IGNORE INTO storages (id, name, address) 
        VALUES (1, 'Основной склад', 'г. Москва, ул. Промышленная, 1');
    )";
    
    const std::string SELECT_ALL_PRODUCTS = R"(
        SELECT p.id, p.name, p.price, p.unit,
               s.id as storage_id, s.name as storage_name,
               COALESCE(b.quantity, 0) as quantity
        FROM products p
        CROSS JOIN storages s
        LEFT JOIN balances b ON p.id = b.product_id AND s.id = b.storage_id
        ORDER BY p.id, s.id
    )";
    
    const std::string SELECT_PRODUCT_BY_ID = 
        "SELECT id, name, price, unit FROM products WHERE id = ?";
    
    const std::string INSERT_PRODUCT = 
        "INSERT INTO products (name, price, unit) VALUES (?, ?, ?)";
    
    const std::string UPDATE_PRODUCT = 
        "UPDATE products SET name = ?, price = ?, unit = ? WHERE id = ?";
    
    const std::string SELECT_ALL_STORAGES = 
        "SELECT id, name, address FROM storages ORDER BY id";
    
    const std::string SELECT_STORAGE_BY_ID = 
        "SELECT id, name, address FROM storages WHERE id = ?";
    
    const std::string INSERT_STORAGE = 
        "INSERT INTO storages (name, address) VALUES (?, ?)";
    
    const std::string UPDATE_STORAGE = 
        "UPDATE storages SET name = ?, address = ? WHERE id = ?";
    
    const std::string DELETE_STORAGE = 
        "DELETE FROM storages WHERE id = ?";
    
    const std::string SELECT_BALANCE = 
        "SELECT quantity FROM balances WHERE product_id = ? AND storage_id = ?";
    
    const std::string UPDATE_BALANCE = 
        "UPDATE balances SET quantity = ? WHERE product_id = ? AND storage_id = ?";
    
    const std::string INSERT_BALANCE = 
        "INSERT INTO balances (product_id, storage_id, quantity) VALUES (?, ?, ?)";
    
    const std::string SELECT_ALL_BALANCES = R"(
        SELECT s.name as storage_name, p.name as product_name, 
               b.quantity, p.unit
        FROM balances b
        JOIN storages s ON b.storage_id = s.id
        JOIN products p ON b.product_id = p.id
        WHERE b.quantity > 0
        ORDER BY s.name, p.name
    )";
    
    const std::string INSERT_TRANSACTION = R"(
        INSERT INTO transactions (type, product_id, storage_id, quantity, comment) 
        VALUES (?, ?, ?, ?, ?)
    )";
    
    const std::string SELECT_TRANSACTIONS = R"(
        SELECT t.id, t.timestamp, t.type, t.product_id, 
               t.storage_id, t.quantity, t.comment,
               p.name as product_name, s.name as storage_name
        FROM transactions t
        JOIN products p ON t.product_id = p.id
        JOIN storages s ON t.storage_id = s.id
        ORDER BY t.timestamp DESC
        LIMIT ?
    )";
    
    const std::string CHECK_STORAGE_HAS_BALANCE = 
        "SELECT COUNT(*) FROM balances WHERE storage_id = ? AND quantity > 0";
    
    const std::string INIT_PRODUCT_BALANCES = 
        "INSERT INTO balances (product_id, storage_id, quantity) VALUES (?, ?, 0)";
    
    const std::string INIT_STORAGE_BALANCES = 
        "INSERT INTO balances (product_id, storage_id, quantity) SELECT id, ?, 0 FROM products";
}

#endif
