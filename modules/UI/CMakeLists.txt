add_library(UI STATIC
    ConsoleUI.cpp
    # DisplayFormatter.cpp
)

target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(UI Models Services Utils)
