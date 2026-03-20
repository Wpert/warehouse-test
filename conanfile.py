from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os

class WarehouseConan(ConanFile):
    name = "warehouse"
    version = "1.0.0"
    
    # Настройки
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    
    # Зависимости
    requires = [
        "sqlite3/3.42.0",
        "gtest/1.14.0",
        "nlohmann_json/3.11.3"
    ]
    
    # Генераторы для CMake
    generators = "CMakeDeps"
    
    def layout(self):
        cmake_layout(self)
        # Все зависимости будут в папке build/
        self.folders.generators = "."
    
    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = "ON"
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        self.cpp_info.libs = ["warehouse"]
