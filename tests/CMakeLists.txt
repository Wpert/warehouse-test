# Поиск Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Тесты для Database модуля
add_executable(test_database test_database.cpp)
target_link_libraries(test_database 
    Database 
    Utils
    ${GTEST_LIBRARIES} 
    pthread
)
add_test(NAME DatabaseTest COMMAND test_database)

# Тесты для Models модуля
add_executable(test_models test_models.cpp)
target_link_libraries(test_models 
    Models 
    Database 
    Services 
    Utils
    ${GTEST_LIBRARIES} 
    pthread
)
add_test(NAME ModelsTest COMMAND test_models)

# Тесты для Validation сервиса
add_executable(test_validation test_validation.cpp)
target_link_libraries(test_validation 
    Services 
    Utils
    ${GTEST_LIBRARIES} 
    pthread
)
add_test(NAME ValidationTest COMMAND test_validation)

# Тесты для Transaction Manager
add_executable(test_transactions test_transactions.cpp)
target_link_libraries(test_transactions 
    Models 
    Database 
    Services 
    Utils
    ${GTEST_LIBRARIES} 
    pthread
)
add_test(NAME TransactionTest COMMAND test_transactions)

# Интеграционные тесты
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration 
    Core
    Models
    Database
    Services
    UI
    Utils
    ${GTEST_LIBRARIES} 
    pthread
)
add_test(NAME IntegrationTest COMMAND test_integration)

# Копирование тестовой конфигурации
add_custom_command(TARGET test_database POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../config.json
        $<TARGET_FILE_DIR:test_database>/config.json
)
