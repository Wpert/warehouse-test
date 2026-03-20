#include <iostream>
#include <memory>
#include <exception>
#include "modules/Core/Application.h"

int main() {
    try {
        auto app = std::make_unique<Application>();
        return app->run();
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
}
