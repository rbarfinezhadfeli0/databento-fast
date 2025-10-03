cmake_minimum_required(VERSION 3.15)
project(databento-fast VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O3 -march=native -Wall -Wextra)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core library
add_library(databento_core STATIC
    src/dbn_parser.cpp
)

target_include_directories(databento_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable position independent code for static library (needed for Python bindings)
set_target_properties(databento_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Python bindings (optional)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)
    
    pybind11_add_module(databento_cpp
        python/bindings.cpp
    )
    
    target_link_libraries(databento_cpp PRIVATE databento_core)
endif()

# Tests
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install rules
install(TARGETS databento_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/databento
    DESTINATION include
)
