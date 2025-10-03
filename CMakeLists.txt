cmake_minimum_required(VERSION 3.14)
project(databento-fast VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Options
# ============================================================================

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_PYTHON "Build Python bindings" OFF)

# ============================================================================
# Main library
# ============================================================================

add_library(databento-cpp SHARED
    src/parser.cpp
)

target_include_directories(databento-cpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Aggressive optimizations for maximum performance
target_compile_options(databento-cpp PRIVATE
    -O3
    -march=native
    -Wall
    -Wextra
    -Wpedantic
)

# ============================================================================
# Examples
# ============================================================================

if(BUILD_EXAMPLES)
    add_executable(simple_mbo_parsing examples/simple_mbo_parsing.cpp)
    target_link_libraries(simple_mbo_parsing PRIVATE databento-cpp)
    target_compile_options(simple_mbo_parsing PRIVATE -O3 -march=native)

    add_executable(ultra_fast_parsing examples/ultra_fast_parsing.cpp)
    target_link_libraries(ultra_fast_parsing PRIVATE databento-cpp)
    target_compile_options(ultra_fast_parsing PRIVATE -O3 -march=native)

    add_executable(batch_processing examples/batch_processing.cpp)
    target_link_libraries(batch_processing PRIVATE databento-cpp)
    target_compile_options(batch_processing PRIVATE -O3 -march=native)
    
    add_executable(standalone_test examples/standalone_test.cpp)
    target_link_libraries(standalone_test PRIVATE databento-cpp)
    target_compile_options(standalone_test PRIVATE -O3 -march=native)
    
    message(STATUS "Examples will be built:")
    message(STATUS "  - simple_mbo_parsing")
    message(STATUS "  - ultra_fast_parsing")
    message(STATUS "  - batch_processing")
    message(STATUS "  - standalone_test (self-contained)")
endif()

# ============================================================================
# Benchmarks
# ============================================================================

if(BUILD_BENCHMARKS)
    add_executable(benchmark_all benchmarks/benchmark_all.cpp)
    target_link_libraries(benchmark_all PRIVATE databento-cpp)
    target_compile_options(benchmark_all PRIVATE -O3 -march=native)
    
    message(STATUS "Benchmarks will be built:")
    message(STATUS "  - benchmark_all")
endif()

# ============================================================================
# Tests (GoogleTest)
# ============================================================================

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_parser tests/test_parser.cpp)
    target_link_libraries(test_parser PRIVATE databento-cpp gtest_main)
    target_compile_options(test_parser PRIVATE -O3 -march=native)

    include(GoogleTest)
    gtest_discover_tests(test_parser)
    
    message(STATUS "Tests will be built with GoogleTest")
endif()

# ============================================================================
# Python bindings (pybind11)
# ============================================================================

if(BUILD_PYTHON)
    find_package(pybind11 CONFIG)
    if(NOT pybind11_FOUND)
        message(STATUS "pybind11 not found, fetching from GitHub...")
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.11.1
        )
        FetchContent_MakeAvailable(pybind11)
    endif()

    pybind11_add_module(databento_cpp python/databento_py.cpp)
    target_link_libraries(databento_cpp PRIVATE databento-cpp)
    target_compile_options(databento_cpp PRIVATE -O3 -march=native)
    
    message(STATUS "Python bindings will be built")
endif()

# ============================================================================
# Install
# ============================================================================

include(GNUInstallDirs)

install(TARGETS databento-cpp
    EXPORT databento-cpp-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/databento
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT databento-cpp-targets
    FILE databento-cpp-config.cmake
    NAMESPACE databento::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/databento-cpp
)

# Package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/databento-cpp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/databento-cpp-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/databento-cpp
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "databento-fast Configuration")
message(STATUS "========================================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests:      ${BUILD_TESTS}")
message(STATUS "Build examples:   ${BUILD_EXAMPLES}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Build Python:     ${BUILD_PYTHON}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Optimization:     -O3 -march=native")
message(STATUS "========================================")
message(STATUS "")
