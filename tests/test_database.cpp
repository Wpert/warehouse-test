#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include "../modules/Database/DatabaseManager.h"
#include "../modules/Utils/Logger.h"

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Используем тестовую базу данных
        dbPath = "test_warehouse.db";
        logger = std::make_shared<Logger>("test.log", LogLevel::ERROR, false);
        db = std::make_shared<DatabaseManager>(dbPath, logger);
        db->initialize();
    }
    
    void TearDown() override {
        db.reset();
        // Удаляем тестовую базу
        std::remove(dbPath.c_str());
        std::remove("test.log");
    }
    
    std::string dbPath;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DatabaseManager> db;
};

// Тест инициализации базы данных
TEST_F(DatabaseTest, Initialization) {
    EXPECT_TRUE(db->initialize());
}

// Тест добавления продукта
TEST_F(DatabaseTest, AddProduct) {
    Product product;
    product.name = "Тестовый товар";
    product.price = 100.50;
    product.unit = "шт";
    
    EXPECT_TRUE(db->addProduct(product));
    
    auto products = db->getAllProducts();
    EXPECT_EQ(products.size(), 1);
    EXPECT_EQ(products[0].name, "Тестовый товар");
    EXPECT_DOUBLE_EQ(products[0].price, 100.50);
}

// Тест добавления дубликата продукта
TEST_F(DatabaseTest, AddDuplicateProduct) {
    Product product1;
    product1.name = "Тестовый товар";
    product1.price = 100;
    product1.unit = "шт";
    
    Product product2;
    product2.name = "Тестовый товар";
    product2.price = 200;
    product2.unit = "шт";
    
    EXPECT_TRUE(db->addProduct(product1));
    EXPECT_FALSE(db->addProduct(product2)); // Должно вернуть false из-за UNIQUE constraint
}

// Тест получения продукта по ID
TEST_F(DatabaseTest, GetProductById) {
    Product product;
    product.name = "Тестовый товар";
    product.price = 100;
    product.unit = "шт";
    
    db->addProduct(product);
    auto products = db->getAllProducts();
    ASSERT_FALSE(products.empty());
    
    auto retrieved = db->getProduct(products[0].id);
    EXPECT_EQ(retrieved.id, products[0].id);
    EXPECT_EQ(retrieved.name, "Тестовый товар");
}

// Тест обновления продукта
TEST_F(DatabaseTest, UpdateProduct) {
    Product product;
    product.name = "Старое название";
    product.price = 100;
    product.unit = "шт";
    
    db->addProduct(product);
    auto products = db->getAllProducts();
    ASSERT_FALSE(products.empty());
    
    Product updated = products[0];
    updated.name = "Новое название";
    updated.price = 200;
    
    EXPECT_TRUE(db->updateProduct(updated));
    
    auto retrieved = db->getProduct(updated.id);
    EXPECT_EQ(retrieved.name, "Новое название");
    EXPECT_DOUBLE_EQ(retrieved.price, 200);
}

// Тест добавления склада
TEST_F(DatabaseTest, AddStorage) {
    Storage storage;
    storage.name = "Тестовый склад";
    storage.address = "Тестовый адрес";
    
    EXPECT_TRUE(db->addStorage(storage));
    
    auto storages = db->getAllStorages();
    EXPECT_EQ(storages.size(), 2); // 1 дефолтный + 1 тестовый
    EXPECT_EQ(storages[1].name, "Тестовый склад");
}

// Тест обновления баланса
TEST_F(DatabaseTest, UpdateBalance) {
    // Добавляем продукт
    Product product;
    product.name = "Тестовый товар";
    product.price = 100;
    product.unit = "шт";
    db->addProduct(product);
    
    auto products = db->getAllProducts();
    ASSERT_FALSE(products.empty());
    int productId = products[0].id;
    
    // Проверяем начальный баланс
    double balance = db->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 0.0);
    
    // Обновляем баланс
    EXPECT_TRUE(db->updateBalance(productId, 1, 100.5));
    
    // Проверяем новый баланс
    balance = db->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 100.5);
}

// Тест получения всех балансов
TEST_F(DatabaseTest, GetAllBalances) {
    // Добавляем продукт
    Product product;
    product.name = "Тестовый товар";
    product.price = 100;
    product.unit = "шт";
    db->addProduct(product);
    
    auto products = db->getAllProducts();
    ASSERT_FALSE(products.empty());
    
    // Устанавливаем баланс
    db->updateBalance(products[0].id, 1, 50);
    
    auto balances = db->getAllBalances();
    EXPECT_EQ(balances.size(), 1);
    EXPECT_DOUBLE_EQ(balances[0].quantity, 50);
}
