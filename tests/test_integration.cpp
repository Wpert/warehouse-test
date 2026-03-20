#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include <thread>
#include <chrono>
#include "../modules/Core/Application.h"
#include "../modules/Models/ProductManager.h"
#include "../modules/Models/StorageManager.h"
#include "../modules/Models/TransactionManager.h"
#include "../modules/Services/ConfigService.h"
#include "../modules/Database/DatabaseManager.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовую конфигурацию
        std::ofstream config("test_config.json");
        config << R"({
            "database": "integration_test.db",
            "log_level": "error",
            "log_file": "integration_test.log"
        })";
        config.close();
        
        configService = std::make_shared<ConfigService>("test_config.json");
        logger = std::make_shared<Logger>("integration_test.log", LogLevel::ERROR, false);
        db = std::make_shared<DatabaseManager>(configService->getDatabasePath(), logger);
        db->initialize();
        
        productManager = std::make_shared<ProductManager>(db, logger);
        storageManager = std::make_shared<StorageManager>(db, logger);
        transactionManager = std::make_shared<TransactionManager>(db, logger);
    }
    
    void TearDown() override {
        productManager.reset();
        storageManager.reset();
        transactionManager.reset();
        db.reset();
        configService.reset();
        
        std::remove("test_config.json");
        std::remove("integration_test.db");
        std::remove("integration_test.log");
    }
    
    std::shared_ptr<ConfigService> configService;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<ProductManager> productManager;
    std::shared_ptr<StorageManager> storageManager;
    std::shared_ptr<TransactionManager> transactionManager;
};

// Полный сценарий работы со складом
TEST_F(IntegrationTest, FullWarehouseScenario) {
    // 1. Добавляем товары
    Product product1;
    product1.name = "Ноутбук";
    product1.price = 50000;
    product1.unit = "шт";
    
    Product product2;
    product2.name = "Мышь";
    product2.price = 1500;
    product2.unit = "шт";
    
    Product product3;
    product3.name = "Клавиатура";
    product3.price = 3000;
    product3.unit = "шт";
    
    EXPECT_TRUE(productManager->addProduct(product1));
    EXPECT_TRUE(productManager->addProduct(product2));
    EXPECT_TRUE(productManager->addProduct(product3));
    
    auto products = productManager->getAllProducts();
    EXPECT_EQ(products.size(), 3);
    
    // 2. Добавляем склад
    Storage storage;
    storage.name = "IT-склад";
    storage.address = "Москва, ул. Программистов, 1";
    EXPECT_TRUE(storageManager->addStorage(storage));
    
    auto storages = storageManager->getAllStorages();
    int storageId = 0;
    for (const auto& s : storages) {
        if (s.name == "IT-склад") {
            storageId = s.id;
            break;
        }
    }
    ASSERT_NE(storageId, 0);
    
    // 3. Приемка товаров
    EXPECT_TRUE(transactionManager->receiveProduct(products[0].id, storageId, 10, "Поставка ноутбуков"));
    EXPECT_TRUE(transactionManager->receiveProduct(products[1].id, storageId, 50, "Поставка мышей"));
    EXPECT_TRUE(transactionManager->receiveProduct(products[2].id, storageId, 30, "Поставка клавиатур"));
    
    // 4. Проверка остатков
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[0].id, storageId), 10);
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[1].id, storageId), 50);
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[2].id, storageId), 30);
    
    // 5. Отгрузка товаров
    EXPECT_TRUE(transactionManager->shipProduct(products[0].id, storageId, 2, "Продажа 2 ноутбуков"));
    EXPECT_TRUE(transactionManager->shipProduct(products[1].id, storageId, 10, "Продажа 10 мышей"));
    EXPECT_TRUE(transactionManager->shipProduct(products[2].id, storageId, 5, "Продажа 5 клавиатур"));
    
    // 6. Проверка остатков после отгрузки
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[0].id, storageId), 8);
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[1].id, storageId), 40);
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[2].id, storageId), 25);
    
    // 7. Проверка истории операций
    auto history = transactionManager->getTransactionHistory();
    EXPECT_EQ(history.size(), 6); // 3 приемки + 3 отгрузки
    
    // 8. Добавляем второй склад
    Storage storage2;
    storage2.name = "Резервный склад";
    storage2.address = "Москва, ул. Запасная, 1";
    EXPECT_TRUE(storageManager->addStorage(storage2));
    
    auto storages2 = storageManager->getAllStorages();
    int storage2Id = 0;
    for (const auto& s : storages2) {
        if (s.name == "Резервный склад") {
            storage2Id = s.id;
            break;
        }
    }
    ASSERT_NE(storage2Id, 0);
    
    // 9. Перемещение товаров (через отгрузку и приемку)
    EXPECT_TRUE(transactionManager->shipProduct(products[0].id, storageId, 3, "Перемещение в резерв"));
    EXPECT_TRUE(transactionManager->receiveProduct(products[0].id, storage2Id, 3, "Перемещение из основного"));
    
    // 10. Проверка финальных остатков
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[0].id, storageId), 5);
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[0].id, storage2Id), 3);
    
    // 11. Проверка всех остатков через общий баланс
    auto balances = storageManager->getAllBalances();
    EXPECT_EQ(balances.size(), 4); // 3 товара на основном складе (8,40,25) + 1 товар на резервном (3)
    
    // 12. Проверка валидации - попытка отгрузить больше чем есть
    EXPECT_FALSE(transactionManager->shipProduct(products[0].id, storageId, 100));
    
    // 13. Проверка удаления товара без остатков
    // Сначала обнулим остатки мышей
    EXPECT_TRUE(transactionManager->shipProduct(products[1].id, storageId, 40));
    EXPECT_DOUBLE_EQ(storageManager->getProductBalance(products[1].id, storageId), 0);
    
    // Теперь можно удалить товар
    EXPECT_TRUE(productManager->deleteProduct(products[1].id));
    
    auto productsAfterDelete = productManager->getAllProducts();
    EXPECT_EQ(productsAfterDelete.size(), 2);
}

// Тест конкурентного доступа (если нужно проверить многопоточность)
TEST_F(IntegrationTest, ConcurrentAccess) {
    // Добавляем товар
    Product product;
    product.name = "Конкурентный товар";
    product.price = 1000;
    product.unit = "шт";
    productManager->addProduct(product);
    
    auto products = productManager->getAllProducts();
    int productId = products[0].id;
    
    // Принимаем большое количество
    transactionManager->receiveProduct(productId, 1, 1000);
    
    // Запускаем несколько потоков для отгрузки
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);
    std::atomic<int> failCount(0);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < 10; j++) {
                if (transactionManager->shipProduct(productId, 1, 1)) {
                    successCount++;
                } else {
                    failCount++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Должно быть 100 успешных операций (отгрузили все 1000)
    EXPECT_EQ(successCount, 100);
    EXPECT_GE(failCount, 0);
    
    double finalBalance = storageManager->getProductBalance(productId, 1);
    EXPECT_DOUBLE_EQ(finalBalance, 900); // 1000 - 100 = 900
}
