#include <gtest/gtest.h>
#include "../modules/Services/ValidationService.h"

// Тесты валидации продукта
TEST(ValidationTest, ValidateProduct) {
    Product validProduct;
    validProduct.name = "Тестовый товар";
    validProduct.price = 100.50;
    validProduct.unit = "шт";
    
    EXPECT_TRUE(ValidationService::validateProduct(validProduct));
    
    // Пустое имя
    Product emptyName = validProduct;
    emptyName.name = "";
    EXPECT_FALSE(ValidationService::validateProduct(emptyName));
    
    // Отрицательная цена
    Product negativePrice = validProduct;
    negativePrice.price = -10;
    EXPECT_FALSE(ValidationService::validateProduct(negativePrice));
    
    // Слишком длинное имя
    Product longName = validProduct;
    longName.name = std::string(150, 'a');
    EXPECT_FALSE(ValidationService::validateProduct(longName));
    
    // Пустая единица измерения
    Product emptyUnit = validProduct;
    emptyUnit.unit = "";
    EXPECT_FALSE(ValidationService::validateProduct(emptyUnit));
}

// Тесты валидации склада
TEST(ValidationTest, ValidateStorage) {
    Storage validStorage;
    validStorage.name = "Основной склад";
    validStorage.address = "ул. Ленина, 1";
    
    EXPECT_TRUE(ValidationService::validateStorage(validStorage));
    
    // Пустое имя
    Storage emptyName = validStorage;
    emptyName.name = "";
    EXPECT_FALSE(ValidationService::validateStorage(emptyName));
    
    // Слишком длинное имя
    Storage longName = validStorage;
    longName.name = std::string(100, 'a');
    EXPECT_FALSE(ValidationService::validateStorage(longName));
}

// Тесты валидации количества
TEST(ValidationTest, ValidateQuantity) {
    EXPECT_TRUE(ValidationService::validateQuantity(1));
    EXPECT_TRUE(ValidationService::validateQuantity(100.5));
    EXPECT_TRUE(ValidationService::validateQuantity(0.001));
    
    // Ноль
    EXPECT_FALSE(ValidationService::validateQuantity(0));
    
    // Отрицательное
    EXPECT_FALSE(ValidationService::validateQuantity(-10));
    
    // Слишком большое
    EXPECT_FALSE(ValidationService::validateQuantity(1e12));
    
    // Слишком много знаков после запятой
    EXPECT_FALSE(ValidationService::validateQuantity(1.1234));
}

// Тесты валидации ID
TEST(ValidationTest, ValidateId) {
    EXPECT_TRUE(ValidationService::validateId(1));
    EXPECT_TRUE(ValidationService::validateId(1000));
    
    EXPECT_FALSE(ValidationService::validateId(0));
    EXPECT_FALSE(ValidationService::validateId(-5));
}
