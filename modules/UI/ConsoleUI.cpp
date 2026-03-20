#include "ConsoleUI.h"
#include "../Services/ValidationService.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

ConsoleUI::ConsoleUI(std::shared_ptr<ProductManager> pm, 
                     std::shared_ptr<StorageManager> sm,
                     std::shared_ptr<TransactionManager> tm,
                     std::shared_ptr<Logger> log)
    : productManager(pm), storageManager(sm), 
      transactionManager(tm), logger(log), isRunning(true) {
    initializeMenus();
}

void ConsoleUI::initializeMenus() {
    mainMenu = {
        {1, {"Просмотр списка номенклатур", [this]() { handleViewProducts(); }}},
        {2, {"Добавить новую номенклатуру", [this]() { handleAddProduct(); }}},
        {3, {"Редактировать номенклатуру", [this]() { handleEditProduct(); }}},
        {4, {"Удалить номенклатуру", [this]() { handleDeleteProduct(); }}},
        {5, {"Приемка товара (поступление)", [this]() { handleReceiveGoods(); }}},
        {6, {"Отгрузка товара", [this]() { handleShipGoods(); }}},
        {7, {"Просмотр истории операций", [this]() { handleViewHistory(); }}},
        {8, {"Управление складами", [this]() { handleManageStorages(); }}},
        {9, {"Просмотр остатков по складам", [this]() { handleViewBalances(); }}},
        {0, {"Выход", [this]() { isRunning = false; }}}
    };
    
    storageMenu = {
        {1, {"Просмотреть список складов", [this]() { handleListStorages(); }}},
        {2, {"Добавить новый склад", [this]() { handleAddStorage(); }}},
        {3, {"Редактировать склад", [this]() { handleEditStorage(); }}},
        {4, {"Удалить склад", [this]() { handleDeleteStorage(); }}},
        {0, {"Вернуться в главное меню", nullptr}}
    };
}

void ConsoleUI::clearScreen() {
    int exitCode = 0;
    #ifdef _WIN32
        exitCode = system("cls");
    #else
        exitCode = system("clear");
    #endif

    std::cout << exitCode << std::endl;
}

void ConsoleUI::waitForUser() {
    std::cout << "\n  Нажмите Enter для продолжения...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void ConsoleUI::printHeader(const std::string& title) {
    std::cout << DisplayFormatter::getHeader(title);
}

void ConsoleUI::showWelcomeMessage() {
    clearScreen();
    printHeader("СИСТЕМА УПРАВЛЕНИЯ СКЛАДОМ v1.0");
    std::cout << "\n  Добро пожаловать в систему управления складом!\n";
    std::cout << "  Здесь вы можете управлять номенклатурами, складами\n";
    std::cout << "  и отслеживать движение товаров.\n\n";
    waitForUser();
}

void ConsoleUI::runMainLoop() {
    while (isRunning) {
        clearScreen();
        printHeader("ГЛАВНОЕ МЕНЮ");
        
        for (const auto& [key, item] : mainMenu) {
            std::cout << DisplayFormatter::getMenuItem(key, item.name);
        }
        
        std::cout << DisplayFormatter::getPrompt();
        
        int choice = readInt("");
        auto it = mainMenu.find(choice);
        
        if (it != mainMenu.end() && it->second.handler) {
            it->second.handler();
        } else {
            std::cout << DisplayFormatter::getError("Неверный выбор. Попробуйте снова.");
            waitForUser();
        }
    }
}

void ConsoleUI::handleViewProducts() {
    clearScreen();
    printHeader("СПИСОК НОМЕНКЛАТУР");
    
    auto products = productManager->getAllProducts();
    std::cout << DisplayFormatter::formatProductTable(products);
    
    waitForUser();
}

void ConsoleUI::handleAddProduct() {
    clearScreen();
    printHeader("ДОБАВЛЕНИЕ НОМЕНКЛАТУРЫ");
    
    Product product;
    
    auto name = ValidationService::readString("  Наименование: ", false, 100);
    if (!name) {
        std::cout << DisplayFormatter::getError("Некорректное наименование");
        waitForUser();
        return;
    }
    product.name = *name;
    
    auto price = ValidationService::readValue<double>("  Цена: ");
    if (!price) {
        std::cout << DisplayFormatter::getError("Некорректная цена");
        waitForUser();
        return;
    }
    product.price = *price;
    
    auto unit = ValidationService::readString("  Единица измерения (шт/кг/л): ", false, 10);
    if (!unit) {
        std::cout << DisplayFormatter::getError("Некорректная единица измерения");
        waitForUser();
        return;
    }
    product.unit = *unit;
    
    if (productManager->addProduct(product)) {
        std::cout << DisplayFormatter::getSuccess("Номенклатура успешно добавлена!");
        if (logger) logger->info("Added product: " + product.name);
    } else {
        std::cout << DisplayFormatter::getError("Ошибка при добавлении номенклатуры.");
    }
    
    waitForUser();
}

void ConsoleUI::handleEditProduct() {
    clearScreen();
    printHeader("РЕДАКТИРОВАНИЕ НОМЕНКЛАТУРЫ");
    
    int id = readInt("  Введите ID номенклатуры: ");
    if (id <= 0) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    auto product = productManager->getProduct(id);
    if (product.id == 0) {
        std::cout << DisplayFormatter::getError("Номенклатура не найдена.");
        waitForUser();
        return;
    }
    
    std::cout << "\n  Текущие данные:\n";
    std::cout << "    Наименование: " << product.name << "\n";
    std::cout << "    Цена: " << product.price << "\n";
    std::cout << "    Ед. изм.: " << product.unit << "\n\n";
    std::cout << "  Оставьте поле пустым, чтобы оставить без изменений.\n\n";
    
    auto name = ValidationService::readString("  Новое наименование: ", true, 100);
    if (name && !name->empty()) product.name = *name;
    
    auto price = ValidationService::readValue<double>("  Новая цена (0 для пропуска): ");
    if (price && *price > 0) product.price = *price;
    
    auto unit = ValidationService::readString("  Новая единица измерения: ", true, 10);
    if (unit && !unit->empty()) product.unit = *unit;
    
    if (productManager->updateProduct(product)) {
        std::cout << DisplayFormatter::getSuccess("Номенклатура успешно обновлена!");
        if (logger) logger->info("Updated product ID=" + std::to_string(id));
    } else {
        std::cout << DisplayFormatter::getError("Ошибка при обновлении.");
    }
    
    waitForUser();
}

void ConsoleUI::handleDeleteProduct() {
    clearScreen();
    printHeader("УДАЛЕНИЕ НОМЕНКЛАТУРЫ");
    
    int id = readInt("  Введите ID номенклатуры: ");
    if (id <= 0) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    auto product = productManager->getProduct(id);
    if (product.id == 0) {
        std::cout << DisplayFormatter::getError("Номенклатура не найдена.");
        waitForUser();
        return;
    }
    
    std::cout << "\n  Номенклатура: " << product.name << "\n";
    std::cout << "  Остаток: " << product.getTotalQuantity() << " " << product.unit << "\n\n";
    
    if (product.getTotalQuantity() > 0) {
        std::cout << DisplayFormatter::getError("Невозможно удалить - есть остатки на складах!");
        waitForUser();
        return;
    }
    
    if (confirmAction("  Вы уверены, что хотите удалить эту номенклатуру?")) {
        if (productManager->deleteProduct(id)) {
            std::cout << DisplayFormatter::getSuccess("Номенклатура успешно удалена!");
            if (logger) logger->info("Deleted product ID=" + std::to_string(id));
        } else {
            std::cout << DisplayFormatter::getError("Ошибка при удалении.");
        }
    }
    
    waitForUser();
}

void ConsoleUI::handleReceiveGoods() {
    clearScreen();
    printHeader("ПРИЕМКА ТОВАРА");
    
    int productId = readInt("  ID номенклатуры: ");
    if (!ValidationService::validateId(productId)) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    if (!productManager->productExists(productId)) {
        std::cout << DisplayFormatter::getError("Номенклатура не найдена");
        waitForUser();
        return;
    }
    
    int storageId = readInt("  ID склада: ");
    if (!ValidationService::validateId(storageId)) {
        std::cout << DisplayFormatter::getError("Некорректный ID склада");
        waitForUser();
        return;
    }
    
    if (!storageManager->storageExists(storageId)) {
        std::cout << DisplayFormatter::getError("Склад не найден");
        waitForUser();
        return;
    }
    
    double quantity = readDouble("  Количество: ");
    auto qtyValidation = ValidationService::validateQuantity(quantity);
    if (!qtyValidation) {
        std::cout << DisplayFormatter::getError(qtyValidation.message);
        waitForUser();
        return;
    }
    
    std::string comment = readString("  Комментарий (необязательно): ");
    
    if (transactionManager->receiveProduct(productId, storageId, quantity, comment)) {
        std::cout << DisplayFormatter::getSuccess("Товар успешно принят!");
    } else {
        std::cout << DisplayFormatter::getError("Ошибка при приемке товара.");
    }
    
    waitForUser();
}

void ConsoleUI::handleShipGoods() {
    clearScreen();
    printHeader("ОТГРУЗКА ТОВАРА");
    
    int productId = readInt("  ID номенклатуры: ");
    if (!ValidationService::validateId(productId)) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    if (!productManager->productExists(productId)) {
        std::cout << DisplayFormatter::getError("Номенклатура не найдена");
        waitForUser();
        return;
    }
    
    int storageId = readInt("  ID склада: ");
    if (!ValidationService::validateId(storageId)) {
        std::cout << DisplayFormatter::getError("Некорректный ID склада");
        waitForUser();
        return;
    }
    
    if (!storageManager->storageExists(storageId)) {
        std::cout << DisplayFormatter::getError("Склад не найден");
        waitForUser();
        return;
    }
    
    double quantity = readDouble("  Количество: ");
    auto qtyValidation = ValidationService::validateQuantity(quantity);
    if (!qtyValidation) {
        std::cout << DisplayFormatter::getError(qtyValidation.message);
        waitForUser();
        return;
    }
    
    // Проверка наличия
    double currentBalance = storageManager->getProductBalance(productId, storageId);
    if (currentBalance < quantity) {
        std::cout << DisplayFormatter::getError(
            "Недостаточно товара на складе. Доступно: " + 
            std::to_string(currentBalance));
        waitForUser();
        return;
    }
    
    std::string comment = readString("  Комментарий (необязательно): ");
    
    if (confirmAction("  Подтверждаете отгрузку?")) {
        if (transactionManager->shipProduct(productId, storageId, quantity, comment)) {
            std::cout << DisplayFormatter::getSuccess("Товар успешно отгружен!");
        } else {
            std::cout << DisplayFormatter::getError("Ошибка при отгрузке товара.");
        }
    }
    
    waitForUser();
}

void ConsoleUI::handleViewHistory() {
    clearScreen();
    printHeader("ИСТОРИЯ ОПЕРАЦИЙ");
    
    auto history = transactionManager->getTransactionHistory(50);
    std::cout << DisplayFormatter::formatTransactionTable(history);
    
    waitForUser();
}

void ConsoleUI::handleViewBalances() {
    clearScreen();
    printHeader("ОСТАТКИ ПО СКЛАДАМ");
    
    auto balances = storageManager->getAllBalances();
    std::cout << DisplayFormatter::formatBalanceTable(balances);
    
    waitForUser();
}

void ConsoleUI::handleManageStorages() {
    bool inStorageMenu = true;
    
    while (inStorageMenu) {
        clearScreen();
        printHeader("УПРАВЛЕНИЕ СКЛАДАМИ");
        
        for (const auto& [key, item] : storageMenu) {
            std::cout << DisplayFormatter::getMenuItem(key, item.name);
        }
        
        std::cout << DisplayFormatter::getPrompt();
        
        int choice = readInt("");
        
        switch (choice) {
            case 1: handleListStorages(); break;
            case 2: handleAddStorage(); break;
            case 3: handleEditStorage(); break;
            case 4: handleDeleteStorage(); break;
            case 0: inStorageMenu = false; break;
            default:
                std::cout << DisplayFormatter::getError("Неверный выбор.");
                waitForUser();
        }
    }
}

void ConsoleUI::handleListStorages() {
    clearScreen();
    printHeader("СПИСОК СКЛАДОВ");
    
    auto storages = storageManager->getAllStorages();
    std::cout << DisplayFormatter::formatStorageTable(storages);
    
    waitForUser();
}

void ConsoleUI::handleAddStorage() {
    clearScreen();
    printHeader("ДОБАВЛЕНИЕ СКЛАДА");
    
    Storage storage;
    
    auto name = ValidationService::readString("  Название склада: ", false, 50);
    if (!name) {
        std::cout << DisplayFormatter::getError("Некорректное название");
        waitForUser();
        return;
    }
    storage.name = *name;
    
    auto address = ValidationService::readString("  Адрес: ", true, 200);
    if (address) storage.address = *address;
    
    if (storageManager->addStorage(storage)) {
        std::cout << DisplayFormatter::getSuccess("Склад успешно добавлен!");
        if (logger) logger->info("Added storage: " + storage.name);
    } else {
        std::cout << DisplayFormatter::getError("Ошибка при добавлении склада.");
    }
    
    waitForUser();
}

void ConsoleUI::handleEditStorage() {
    clearScreen();
    printHeader("РЕДАКТИРОВАНИЕ СКЛАДА");
    
    int id = readInt("  Введите ID склада: ");
    if (id <= 0) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    auto storage = storageManager->getStorage(id);
    if (storage.isEmpty()) {
        std::cout << DisplayFormatter::getError("Склад не найден.");
        waitForUser();
        return;
    }
    
    std::cout << "\n  Текущие данные:\n";
    std::cout << "    Название: " << storage.name << "\n";
    std::cout << "    Адрес: " << storage.address << "\n\n";
    std::cout << "  Оставьте поле пустым, чтобы оставить без изменений.\n\n";
    
    auto name = ValidationService::readString("  Новое название: ", true, 50);
    if (name && !name->empty()) storage.name = *name;
    
    auto address = ValidationService::readString("  Новый адрес: ", true, 200);
    if (address) storage.address = *address;
    
    if (storageManager->updateStorage(storage)) {
        std::cout << DisplayFormatter::getSuccess("Склад успешно обновлен!");
        if (logger) logger->info("Updated storage ID=" + std::to_string(id));
    } else {
        std::cout << DisplayFormatter::getError("Ошибка при обновлении.");
    }
    
    waitForUser();
}

void ConsoleUI::handleDeleteStorage() {
    clearScreen();
    printHeader("УДАЛЕНИЕ СКЛАДА");
    
    int id = readInt("  Введите ID склада для удаления: ");
    if (id <= 0) {
        std::cout << DisplayFormatter::getError("Некорректный ID");
        waitForUser();
        return;
    }
    
    auto storage = storageManager->getStorage(id);
    if (storage.isEmpty()) {
        std::cout << DisplayFormatter::getError("Склад не найден.");
        waitForUser();
        return;
    }
    
    std::cout << "\n  Склад: " << storage.name << "\n";
    
    if (storageManager->hasBalances(id)) {
        std::cout << DisplayFormatter::getError(
            "  Невозможно удалить склад - на нем есть товары!\n");
        waitForUser();
        return;
    }
    
    if (confirmAction("  Вы уверены, что хотите удалить этот склад?")) {
        if (storageManager->deleteStorage(id)) {
            std::cout << DisplayFormatter::getSuccess("Склад успешно удален!");
            if (logger) logger->info("Deleted storage ID=" + std::to_string(id));
        } else {
            std::cout << DisplayFormatter::getError("Ошибка при удалении склада.");
        }
    }
    
    waitForUser();
}

int ConsoleUI::readInt(const std::string& prompt) {
    std::cout << prompt;
    int value;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

double ConsoleUI::readDouble(const std::string& prompt) {
    std::cout << prompt;
    double value;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

std::string ConsoleUI::readString(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

bool ConsoleUI::confirmAction(const std::string& message) {
    std::cout << message << " (y/n): ";
    char response;
    std::cin >> response;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return tolower(response) == 'y';
}
