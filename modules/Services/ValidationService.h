#ifndef VALIDATIONSERVICE_H
#define VALIDATIONSERVICE_H

#include <string>
#include <optional>
#include <functional>
#include <iostream>
#include <cmath>
#include <limits>
#include "../Database/Entities.h"

struct ValidationResult {
    bool isValid;
    std::string message;
    
    ValidationResult(bool valid = true, const std::string& msg = "") 
        : isValid(valid), message(msg) {}
    
    operator bool() const { return isValid; }
};

class ValidationService {
public:
    static ValidationResult validateProduct(const Product& product) {
        if (product.name.empty()) {
            return {false, "Наименование товара не может быть пустым"};
        }
        
        if (product.name.length() > 100) {
            return {false, "Наименование товара слишком длинное (макс. 100 символов)"};
        }
        
        if (product.price < 0) {
            return {false, "Цена не может быть отрицательной"};
        }
        
        if (product.price > 999999999.99) {
            return {false, "Цена слишком большая"};
        }
        
        if (product.unit.empty()) {
            return {false, "Единица измерения не может быть пустой"};
        }
        
        if (product.unit.length() > 10) {
            return {false, "Единица измерения слишком длинная (макс. 10 символов)"};
        }
        
        return {true, ""};
    }
    
    static ValidationResult validateStorage(const Storage& storage) {
        if (storage.name.empty()) {
            return {false, "Название склада не может быть пустым"};
        }
        
        if (storage.name.length() > 50) {
            return {false, "Название склада слишком длинное (макс. 50 символов)"};
        }
        
        return {true, ""};
    }
    
    static ValidationResult validateQuantity(double quantity) {
        if (quantity <= 0) {
            return {false, "Количество должно быть положительным"};
        }
        
        if (quantity > 999999999.99) {
            return {false, "Слишком большое количество"};
        }
        
        // Проверка на точность (не более 3 знаков после запятой)
        double rounded = std::round(quantity * 1000) / 1000;
        if (std::abs(quantity - rounded) > 0.0001) {
            return {false, "Количество может содержать не более 3 знаков после запятой"};
        }
        
        return {true, ""};
    }
    
    static ValidationResult validateId(int id) {
        if (id <= 0) {
            return {false, "ID должен быть положительным числом"};
        }
        return {true, ""};
    }
    
    // Чтение числа с валидацией
    template<typename T>
    static std::optional<T> readValue(const std::string& prompt) {
        std::cout << prompt;
        T value;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return std::nullopt;
        }
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
    
    // Чтение строки с валидацией
    static std::optional<std::string> readString(const std::string& prompt, 
                                                   bool allowEmpty = false,
                                                   size_t maxLength = 100) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        
        // Удаляем пробелы в начале и конце
        size_t start = value.find_first_not_of(" \t\r\n");
        size_t end = value.find_last_not_of(" \t\r\n");
        
        if (start != std::string::npos && end != std::string::npos) {
            value = value.substr(start, end - start + 1);
        } else {
            value.clear();
        }
        
        if (!allowEmpty && value.empty()) {
            return std::nullopt;
        }
        
        if (value.length() > maxLength) {
            return std::nullopt;
        }
        
        return value;
    }
    
    // Чтение целого числа с проверкой диапазона
    static std::optional<int> readInt(const std::string& prompt, int min = 1, int max = 999999) {
        auto value = readValue<int>(prompt);
        if (!value) return std::nullopt;
        
        if (*value < min || *value > max) {
            return std::nullopt;
        }
        
        return value;
    }
    
    // Чтение числа с плавающей точкой с проверкой диапазона
    static std::optional<double> readDouble(const std::string& prompt, 
                                            double min = 0.001, 
                                            double max = 999999999.99) {
        auto value = readValue<double>(prompt);
        if (!value) return std::nullopt;
        
        if (*value < min || *value > max) {
            return std::nullopt;
        }
        
        return value;
    }
    
    // Подтверждение действия (y/n)
    static bool confirm(const std::string& message) {
        std::cout << message << " (y/n): ";
        char response;
        std::cin >> response;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return std::tolower(response) == 'y';
    }
};

#endif
