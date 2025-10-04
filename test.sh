#!/bin/bash
# Test script for databento-cpp-fast
# Runs all unit tests

set -e  # Exit on error

echo "========================================="
echo "databento-cpp-fast Test Runner"
echo "========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if build directory exists
if [ ! -d "build" ]; then
    echo -e "${RED}❌ Build directory not found!${NC}"
    echo "Please run ./build.sh first"
    exit 1
fi

cd build

# Check if test executable exists
if [ ! -f "test_parser" ]; then
    echo -e "${RED}❌ Test executable not found!${NC}"
    echo "Please run ./build.sh first"
    exit 1
fi

echo -e "${GREEN}Running unit tests with GoogleTest...${NC}"
echo ""

# Run tests with CTest (provides nice output)
if command -v ctest &> /dev/null; then
    ctest --output-on-failure --verbose
else
    # Fallback: run test executable directly
    ./test_parser
fi

echo ""
echo "========================================="
echo -e "${GREEN}✅ All tests passed!${NC}"
echo "========================================="
echo ""
echo "Test coverage:"
echo "  ✅ File loading and parsing"
echo "  ✅ MBO/Trade record parsing"
echo "  ✅ Direct memory access"
echo "  ✅ Batch processing"
echo "  ✅ Binary readers (uint32/uint64)"
echo "  ✅ Price conversion utilities"
echo "  ✅ Error handling (out of range, file not found)"
echo "  ✅ High-level API functions"
echo ""

