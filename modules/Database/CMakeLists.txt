add_library(Database STATIC
    DatabaseManager.cpp
)

target_include_directories(Database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Добавляем пути к заголовкам
target_include_directories(Database PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(Database
    Utils
    SQLite::SQLite3
)

# Для отладки - показываем, что линкуем
get_target_property(SQLITE_LOC SQLite::SQLite3 IMPORTED_LOCATION)
message(STATUS "Database module линкуется с SQLite: ${SQLITE_LOC}")
