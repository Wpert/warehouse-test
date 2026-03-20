cmake_minimum_required(VERSION 3.16)
project(Warehouse VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Опции сборки
option(BUILD_TESTS "Сборка тестов" OFF)

# Подключаем Conan (если есть)
if(EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    message(STATUS "Conan toolchain найден")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# Подключаем зависимости Conan
if(EXISTS ${CMAKE_BINARY_DIR}/conan_deps.cmake)
    include(${CMAKE_BINARY_DIR}/conan_deps.cmake)
    message(STATUS "Conan зависимости подключены")
endif()

# Поиск SQLite3
find_package(SQLite3 CONFIG REQUIRED)
if(SQLite3_FOUND)
    message(STATUS "SQLite3 найден: ${sqlite3_LIBRARIES}")
endif()

# Поиск nlohmann_json (опционально)
find_package(nlohmann_json CONFIG QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "nlohmann_json найден")
endif()

# Поиск GTest (для тестов)
if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    message(STATUS "GTest найден")
endif()

# Добавляем модули
add_subdirectory(modules/Utils)
add_subdirectory(modules/Database)
add_subdirectory(modules/Services)
add_subdirectory(modules/Models)
add_subdirectory(modules/Core)
add_subdirectory(modules/UI)

# Исполняемый файл
add_executable(warehouse main.cpp)

target_link_libraries(warehouse
    Core
    UI
    Models
    Services
    Database
    Utils
)

if(TARGET SQLite::SQLite3)
    target_link_libraries(warehouse SQLite::SQLite3)
endif()

if(CONAN_LIBS)
    target_link_libraries(warehouse ${CONAN_LIBS})
endif()

# Копирование конфига
add_custom_command(TARGET warehouse POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:warehouse>/config.json
)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
