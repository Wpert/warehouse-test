#ifndef DISPLAYFORMATTER_H
#define DISPLAYFORMATTER_H

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include "../Database/Entities.h"

class DisplayFormatter {
private:
    static const int TABLE_WIDTH = 100;
    
public:
    static std::string getHeader(const std::string& title) {
        std::ostringstream oss;
        oss << "\n" << std::string(60, '=') << "\n";
        oss << "     " << title << "\n";
        oss << std::string(60, '=') << "\n";
        return oss.str();
    }
    
    static std::string getMenuItem(int key, const std::string& name) {
        std::ostringstream oss;
        oss << "  " << std::left << std::setw(2) << key << ". " << name << "\n";
        return oss.str();
    }
    
    static std::string getPrompt() {
        return "\n  Выберите действие: ";
    }
    
    static std::string getError(const std::string& message) {
        return "\n  [ОШИБКА] " + message + "\n";
    }
    
    static std::string getSuccess(const std::string& message) {
        return "\n  [УСПЕХ] " + message + "\n";
    }
    
    static std::string getWarning(const std::string& message) {
        return "\n  [ВНИМАНИЕ] " + message + "\n";
    }
    
    static std::string getInfo(const std::string& message) {
        return "\n  [ИНФО] " + message + "\n";
    }
    
    static std::string formatProductTable(const std::vector<Product>& products) {
        if (products.empty()) {
            return getInfo("Номенклатуры отсутствуют");
        }
        
        std::ostringstream table;
        
        table << "\n" << std::left 
              << std::setw(5) << "ID"
              << std::setw(30) << "Наименование"
              << std::setw(12) << "Цена"
              << std::setw(10) << "Ед.изм."
              << "Остаток\n";
        table << std::string(80, '-') << "\n";
        
        for (const auto& p : products) {
            table << std::left 
                  << std::setw(5) << p.id
                  << std::setw(30) << truncateString(p.name, 28)
                  << std::setw(12) << std::fixed << std::setprecision(2) << p.price
                  << std::setw(10) << p.unit
                  << std::setw(10) << p.getTotalQuantity() << "\n";
            
            // Детализация по складам
            for (const auto& b : p.balances) {
                if (b.quantity > 0) {
                    table << std::string(57, ' ') << "  └ " 
                          << truncateString(b.storageName, 15) << ": " 
                          << b.quantity << " " << p.unit << "\n";
                }
            }
        }
        
        return table.str();
    }
    
    static std::string formatStorageTable(const std::vector<Storage>& storages) {
        if (storages.empty()) {
            return getInfo("Склады отсутствуют");
        }
        
        std::ostringstream table;
        
        table << "\n" << std::left 
              << std::setw(5) << "ID"
              << std::setw(30) << "Название"
              << "Адрес\n";
        table << std::string(80, '-') << "\n";
        
        for (const auto& s : storages) {
            table << std::left 
                  << std::setw(5) << s.id
                  << std::setw(30) << truncateString(s.name, 28)
                  << s.address << "\n";
        }
        
        return table.str();
    }
    
    static std::string formatBalanceTable(const std::vector<BalanceView>& balances) {
        if (balances.empty()) {
            return getInfo("Нет товаров на складах");
        }
        
        std::ostringstream table;
        
        table << "\n" << std::left 
              << std::setw(30) << "Склад"
              << std::setw(30) << "Номенклатура"
              << std::setw(15) << "Количество"
              << "Ед.изм.\n";
        table << std::string(85, '-') << "\n";
        
        for (const auto& b : balances) {
            table << std::left 
                  << std::setw(30) << truncateString(b.storageName, 28)
                  << std::setw(30) << truncateString(b.productName, 28)
                  << std::setw(15) << b.quantity
                  << b.unit << "\n";
        }
        
        return table.str();
    }
    
    static std::string formatTransactionTable(const std::vector<Transaction>& transactions) {
        if (transactions.empty()) {
            return getInfo("История операций пуста");
        }
        
        std::ostringstream table;
        
        table << "\n" << std::left 
              << std::setw(5) << "ID"
              << std::setw(20) << "Дата"
              << std::setw(8) << "Тип"
              << std::setw(8) << "Товар"
              << std::setw(8) << "Склад"
              << std::setw(12) << "Количество"
              << "Комментарий\n";
        table << std::string(100, '-') << "\n";
        
        for (const auto& t : transactions) {
            table << std::left 
                  << std::setw(5) << t.id
                  << std::setw(20) << t.timestamp.substr(0, 19)  // Обрезаем секунды
                  << std::setw(8) << t.getTypeString()
                  << std::setw(8) << t.productId
                  << std::setw(8) << t.storageId
                  << std::setw(12) << t.quantity
                  << truncateString(t.comment, 20) << "\n";
        }
        
        return table.str();
    }
    
private:
    static std::string truncateString(const std::string& str, size_t width) {
        if (str.length() <= width) return str;
        return str.substr(0, width - 3) + "...";
    }
};

#endif
