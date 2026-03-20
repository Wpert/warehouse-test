add_library(Services STATIC
    ConfigService.cpp
)

target_include_directories(Services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Services Utils)
