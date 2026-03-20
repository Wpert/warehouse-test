#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include "../modules/Models/TransactionManager.h"
#include "../modules/Models/ProductManager.h"
#include "../modules/Models/StorageManager.h"
#include "../modules/Database/DatabaseManager.h"
#include "../modules/Utils/Logger.h"

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        dbPath = "test_warehouse.db";
        logger = std::make_shared<Logger>("test.log", LogLevel::ERROR, false);
        db = std::make_shared<DatabaseManager>(dbPath, logger);
        db->initialize();
        
        productManager = std::make_shared<ProductManager>(db, logger);
        storageManager = std::make_shared<StorageManager>(db, logger);
        transactionManager = std::make_shared<TransactionManager>(db, logger);
        
        // Создаем тестовый продукт
        Product product;
        product.name = "Тестовый товар";
        product.price = 100;
        product.unit = "шт";
        productManager->addProduct(product);
        
        auto products = productManager->getAllProducts();
        productId = products[0].id;
    }
    
    void TearDown() override {
        transactionManager.reset();
        productManager.reset();
        storageManager.reset();
        db.reset();
        std::remove(dbPath.c_str());
        std::remove("test.log");
    }
    
    std::string dbPath;
    int productId;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<ProductManager> productManager;
    std::shared_ptr<StorageManager> storageManager;
    std::shared_ptr<TransactionManager> transactionManager;
};

// Тест приемки товара
TEST_F(TransactionTest, ReceiveProduct) {
    EXPECT_TRUE(transactionManager->receiveProduct(productId, 1, 100, "Тестовая поставка"));
    
    double balance = storageManager->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 100);
    
    auto history = transactionManager->getTransactionHistory();
    EXPECT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].type, Transaction::Type::RECEIPT);
    EXPECT_DOUBLE_EQ(history[0].quantity, 100);
}

// Тест отгрузки товара
TEST_F(TransactionTest, ShipProduct) {
    // Сначала принимаем товар
    transactionManager->receiveProduct(productId, 1, 100);
    
    // Отгружаем часть
    EXPECT_TRUE(transactionManager->shipProduct(productId, 1, 30, "Тестовая отгрузка"));
    
    double balance = storageManager->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 70);
    
    auto history = transactionManager->getTransactionHistory();
    EXPECT_EQ(history.size(), 2);
    EXPECT_EQ(history[0].type, Transaction::Type::SHIPMENT); // Последняя операция - отгрузка
}

// Тест отгрузки больше чем есть
TEST_F(TransactionTest, ShipMoreThanAvailable) {
    transactionManager->receiveProduct(productId, 1, 50);
    
    // Попытка отгрузить больше
    EXPECT_FALSE(transactionManager->shipProduct(productId, 1, 100));
    
    // Баланс не должен измениться
    double balance = storageManager->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 50);
}

// Тест отгрузки с несуществующим продуктом
TEST_F(TransactionTest, ShipInvalidProduct) {
    EXPECT_FALSE(transactionManager->shipProduct(9999, 1, 10));
}

// Тест приемки с отрицательным количеством
TEST_F(TransactionTest, ReceiveNegativeQuantity) {
    EXPECT_FALSE(transactionManager->receiveProduct(productId, 1, -10));
}

// Тест множественных транзакций
TEST_F(TransactionTest, MultipleTransactions) {
    // Приемка
    transactionManager->receiveProduct(productId, 1, 100);
    transactionManager->receiveProduct(productId, 1, 50);
    
    // Отгрузка
    transactionManager->shipProduct(productId, 1, 30);
    transactionManager->shipProduct(productId, 1, 20);
    
    double balance = storageManager->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(balance, 100); // 100+50-30-20 = 100
    
    auto history = transactionManager->getTransactionHistory();
    EXPECT_EQ(history.size(), 4);
}

// Тест истории операций с лимитом
TEST_F(TransactionTest, HistoryLimit) {
    for (int i = 0; i < 10; i++) {
        transactionManager->receiveProduct(productId, 1, 10);
    }
    
    auto history = transactionManager->getTransactionHistory(5);
    EXPECT_EQ(history.size(), 5);
}

// Тест отгрузки с другого склада
TEST_F(TransactionTest, DifferentStorages) {
    // Добавляем второй склад
    Storage storage;
    storage.name = "Второй склад";
    storage.address = "Адрес 2";
    storageManager->addStorage(storage);
    
    auto storages = storageManager->getAllStorages();
    int storage2Id = 0;
    for (const auto& s : storages) {
        if (s.name == "Второй склад") {
            storage2Id = s.id;
            break;
        }
    }
    
    // Принимаем на первый склад
    transactionManager->receiveProduct(productId, 1, 100);
    
    // Пытаемся отгрузить со второго - должно провалиться
    EXPECT_FALSE(transactionManager->shipProduct(productId, storage2Id, 10));
    
    // Принимаем на второй склад
    transactionManager->receiveProduct(productId, storage2Id, 50);
    
    // Теперь отгрузка со второго работает
    EXPECT_TRUE(transactionManager->shipProduct(productId, storage2Id, 20));
    
    double balance1 = storageManager->getProductBalance(productId, 1);
    double balance2 = storageManager->getProductBalance(productId, storage2Id);
    
    EXPECT_DOUBLE_EQ(balance1, 100);
    EXPECT_DOUBLE_EQ(balance2, 30);
}
