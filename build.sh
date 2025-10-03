#!/bin/bash
# Build script for databento-cpp-fast
# Compiles library, examples, tests, and benchmarks
#
# Usage: ./build.sh
# Run from any directory - automatically uses script location

set -e  # Exit on error

echo "========================================="
echo "databento-cpp-fast Build Script"
echo "========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Get script directory (works even if called from elsewhere)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Project directory: $SCRIPT_DIR"
echo ""

# Clean old build
if [ -d "build" ]; then
    echo -e "${YELLOW}Cleaning old build...${NC}"
    rm -rf build
fi

# Create build directory
echo -e "${GREEN}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake
echo ""
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_BENCHMARKS=ON \
    -DBUILD_PYTHON=OFF

echo ""
echo -e "${GREEN}Building with make...${NC}"
# Build (use all CPU cores)
make -j$(nproc)

echo ""
echo "========================================="
echo -e "${GREEN}✅ Build completed successfully!${NC}"
echo "========================================="
echo ""

# List executables
echo "Built executables:"
echo "  Libraries:"
echo "    - build/libdatabento-cpp.so"
echo ""
echo "  Examples:"
if [ -f "simple_mbo_parsing" ]; then
    echo "    - build/simple_mbo_parsing"
fi
if [ -f "ultra_fast_parsing" ]; then
    echo "    - build/ultra_fast_parsing"
fi
if [ -f "batch_processing" ]; then
    echo "    - build/batch_processing"
fi
echo ""
echo "  Benchmarks:"
if [ -f "benchmark_all" ]; then
    echo "    - build/benchmark_all"
fi
echo ""
echo "  Tests:"
if [ -f "test_parser" ]; then
    echo "    - build/test_parser"
fi
echo ""
echo "========================================="
echo "Next Steps:"
echo "========================================="
echo ""
echo "1. Run tests:"
echo "   cd build && ctest --output-on-failure"
echo ""
echo "2. Run examples (create test file first):"
echo "   ./build/simple_mbo_parsing /path/to/your/data.dbn"
echo "   ./build/ultra_fast_parsing /path/to/your/data.dbn"
echo "   ./build/batch_processing /path/to/your/data.dbn"
echo ""
echo "3. Run benchmarks:"
echo "   ./build/benchmark_all /path/to/your/data.dbn"
echo ""
echo "4. Install library (optional):"
echo "   cd build && sudo make install"
echo ""

