add_library(Models STATIC
    ProductManager.cpp
    StorageManager.cpp
    TransactionManager.cpp
)

target_include_directories(Models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Models Database Services Utils)
