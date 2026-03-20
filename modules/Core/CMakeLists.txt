add_library(Core STATIC
    Application.cpp
)

target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Core
    UI
    Models
    Services
    Database
    Utils
    SQLite::SQLite3  # Явно добавляем SQLite
)
