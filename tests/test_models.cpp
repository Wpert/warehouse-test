#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include "../modules/Models/ProductManager.h"
#include "../modules/Models/StorageManager.h"
#include "../modules/Database/DatabaseManager.h"
#include "../modules/Utils/Logger.h"

class ModelsTest : public ::testing::Test {
protected:
    void SetUp() override {
        dbPath = "test_warehouse.db";
        logger = std::make_shared<Logger>("test.log", LogLevel::ERROR, false);
        db = std::make_shared<DatabaseManager>(dbPath, logger);
        db->initialize();
        
        productManager = std::make_shared<ProductManager>(db, logger);
        storageManager = std::make_shared<StorageManager>(db, logger);
    }
    
    void TearDown() override {
        productManager.reset();
        storageManager.reset();
        db.reset();
        std::remove(dbPath.c_str());
        std::remove("test.log");
    }
    
    std::string dbPath;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<ProductManager> productManager;
    std::shared_ptr<StorageManager> storageManager;
};

// Тест добавления продукта через ProductManager
TEST_F(ModelsTest, AddProduct) {
    Product product;
    product.name = "Тестовый товар";
    product.price = 150.75;
    product.unit = "кг";
    
    EXPECT_TRUE(productManager->addProduct(product));
    
    auto products = productManager->getAllProducts();
    ASSERT_FALSE(products.empty());
    EXPECT_EQ(products[0].name, "Тестовый товар");
}

// Тест добавления невалидного продукта
TEST_F(ModelsTest, AddInvalidProduct) {
    Product invalidProduct;
    invalidProduct.name = ""; // Пустое имя
    invalidProduct.price = -100; // Отрицательная цена
    invalidProduct.unit = "шт";
    
    EXPECT_FALSE(productManager->addProduct(invalidProduct));
}

// Тест обновления продукта
TEST_F(ModelsTest, UpdateProduct) {
    Product product;
    product.name = "Старое название";
    product.price = 100;
    product.unit = "шт";
    
    EXPECT_TRUE(productManager->addProduct(product));
    
    auto products = productManager->getAllProducts();
    ASSERT_FALSE(products.empty());
    
    Product updated = products[0];
    updated.name = "Обновленное название";
    updated.price = 200;
    
    EXPECT_TRUE(productManager->updateProduct(updated));
    
    auto retrieved = productManager->getProduct(updated.id);
    EXPECT_EQ(retrieved.name, "Обновленное название");
}

// Тест удаления продукта с остатками
TEST_F(ModelsTest, DeleteProductWithBalance) {
    Product product;
    product.name = "Тестовый товар";
    product.price = 100;
    product.unit = "шт";
    
    EXPECT_TRUE(productManager->addProduct(product));
    
    auto products = productManager->getAllProducts();
    ASSERT_FALSE(products.empty());
    
    // Добавляем остаток
    db->updateBalance(products[0].id, 1, 50);
    
    // Попытка удалить продукт с остатками должна провалиться
    EXPECT_FALSE(productManager->deleteProduct(products[0].id));
}

// Тест добавления склада
TEST_F(ModelsTest, AddStorage) {
    Storage storage;
    storage.name = "Новый склад";
    storage.address = "Новый адрес";
    
    EXPECT_TRUE(storageManager->addStorage(storage));
    
    auto storages = storageManager->getAllStorages();
    bool found = false;
    for (const auto& s : storages) {
        if (s.name == "Новый склад") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// Тест удаления склада с остатками
TEST_F(ModelsTest, DeleteStorageWithBalance) {
    // Добавляем склад
    Storage storage;
    storage.name = "Склад для удаления";
    storage.address = "Адрес";
    storageManager->addStorage(storage);
    
    auto storages = storageManager->getAllStorages();
    int storageId = 0;
    for (const auto& s : storages) {
        if (s.name == "Склад для удаления") {
            storageId = s.id;
            break;
        }
    }
    ASSERT_NE(storageId, 0);
    
    // Добавляем продукт и остаток на этот склад
    Product product;
    product.name = "Тестовый товар";
    product.price = 100;
    product.unit = "шт";
    productManager->addProduct(product);
    
    auto products = productManager->getAllProducts();
    db->updateBalance(products[0].id, storageId, 10);
    
    // Проверяем наличие остатков
    EXPECT_TRUE(storageManager->hasBalances(storageId));
    
    // Попытка удалить склад с остатками должна провалиться
    EXPECT_FALSE(storageManager->deleteStorage(storageId));
}
