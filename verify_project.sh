#!/bin/bash
# Verification script - checks all files are present and correct

echo "==========================================="
echo "databento-cpp-fast - Project Verification"
echo "==========================================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

ERRORS=0
WARNINGS=0

# Check function
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
        return 0
    else
        echo -e "${RED}✗${NC} $1 - MISSING"
        ((ERRORS++))
        return 1
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
        return 0
    else
        echo -e "${RED}✗${NC} $1/ - MISSING"
        ((ERRORS++))
        return 1
    fi
}

echo "Checking core files..."
check_file "CMakeLists.txt"
check_file "LICENSE"
check_file "requirements.txt"
check_file "setup.py"

echo ""
echo "Checking headers..."
check_file "include/databento/dbn.hpp"
check_file "include/databento/parser.hpp"

echo ""
echo "Checking source..."
check_file "src/parser.cpp"

echo ""
echo "Checking examples..."
check_file "examples/simple_mbo_parsing.cpp"
check_file "examples/ultra_fast_parsing.cpp"
check_file "examples/batch_processing.cpp"
check_file "examples/standalone_test.cpp"

echo ""
echo "Checking benchmarks..."
check_file "benchmarks/benchmark_all.cpp"

echo ""
echo "Checking tests..."
check_file "tests/test_parser.cpp"

echo ""
echo "Checking Python files..."
check_file "python/databento_py.cpp"
check_file "python/example_python.py"
check_file "python/example_download_and_parse.py"
check_file "python/simple_download_parse.py"

echo ""
echo "Checking scripts..."
check_file "build.sh"
check_file "test.sh"
check_file "install_python.sh"

echo ""
echo "Checking documentation..."
check_file "README.md"
check_file "README_GITHUB.md"
check_file "QUICK_START.md"
check_file "USAGE_GUIDE.md"
check_file "BUILD_INSTRUCTIONS.md"
check_file "START_HERE.md"
check_file "RUN_NOW.md"
check_file "INDEX.md"
check_file "PROJECT_STATUS.md"
check_file "COMPLETION_SUMMARY.md"
check_file "FINAL_REPORT.md"
check_file "CONTRIBUTING.md"
check_file "CHANGELOG.md"
check_file "GITHUB_RELEASE.md"
check_file "GITHUB_READY.md"

echo ""
echo "Checking CI/CD..."
check_file ".github/workflows/build-and-test.yml"

echo ""
echo "Checking Git configuration..."
check_file ".gitignore"

echo ""
echo "==========================================="
echo "Verification Results"
echo "==========================================="

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ All files present!${NC}"
    echo ""
    echo "Project is complete and ready for:"
    echo "  1. Building: ./build.sh"
    echo "  2. Testing: ./test.sh"
    echo "  3. GitHub release: See GITHUB_RELEASE.md"
    echo ""
    exit 0
else
    echo -e "${RED}✗ Found $ERRORS missing files${NC}"
    echo ""
    echo "Please check the missing files above."
    exit 1
fi

