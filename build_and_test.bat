@echo off
REM Windows batch file to build and test databento-cpp
REM Run this from Windows command prompt or PowerShell

echo =========================================
echo databento-cpp Build and Test
echo =========================================
echo.

REM Get the directory where the script is located
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

echo Project directory: %SCRIPT_DIR%
echo.

REM Use WSL to run the build
echo Cleaning old build...
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp && rm -rf build"

echo.
echo Creating build directory...
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp && mkdir -p build"

echo.
echo Running CMake configuration...
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DBUILD_BENCHMARKS=ON"

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building with make...
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp/build && make -j$(nproc)"

if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo =========================================
echo Build completed successfully!
echo =========================================
echo.

echo Running tests...
echo.
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp/build && ctest --output-on-failure"

if errorlevel 1 (
    echo.
    echo ERROR: Tests failed!
    pause
    exit /b 1
)

echo.
echo =========================================
echo All tests passed!
echo =========================================
echo.

echo Built executables:
wsl bash -c "cd /home/nvidia/my_github_libraries/databento-cpp/build && ls -lh simple_mbo_parsing ultra_fast_parsing batch_processing benchmark_all test_parser 2>/dev/null || echo 'No executables found'"

echo.
echo =========================================
echo Next steps:
echo =========================================
echo.
echo Try the examples (need a .dbn file):
echo   wsl bash -c "./build/ultra_fast_parsing /path/to/data.dbn"
echo.
echo Or create a test file and run:
echo   wsl bash -c "./build/test_parser"
echo.

pause

