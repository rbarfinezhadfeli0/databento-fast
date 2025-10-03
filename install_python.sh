#!/bin/bash
# Install Python bindings for databento-cpp-fast

set -e  # Exit on error

echo "========================================="
echo "databento-cpp-fast Python Installation"
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

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}❌ Python 3 not found!${NC}"
    echo "Please install Python 3.7 or later"
    exit 1
fi

PYTHON_VERSION=$(python3 --version)
echo "Found: $PYTHON_VERSION"
echo ""

# Check if pip is available
if ! command -v pip3 &> /dev/null && ! command -v pip &> /dev/null; then
    echo -e "${RED}❌ pip not found!${NC}"
    echo "Please install pip"
    exit 1
fi

# Check if pybind11 is installed
echo -e "${YELLOW}Checking for pybind11...${NC}"
if ! python3 -c "import pybind11" 2>/dev/null; then
    echo "pybind11 not found, installing..."
    pip3 install pybind11 --user
else
    echo "pybind11 already installed"
fi

echo ""
echo -e "${GREEN}Installing databento-cpp Python module...${NC}"
echo ""

# Install in development mode (allows editing C++ code)
pip3 install -e . --user

echo ""
echo "========================================="
echo -e "${GREEN}✅ Python bindings installed!${NC}"
echo "========================================="
echo ""
echo "Test the installation:"
echo "  python3 -c \"import databento_cpp; print(databento_cpp.__version__)\""
echo ""
echo "Run the example:"
echo "  python3 python/example_python.py /path/to/your/data.dbn"
echo ""
echo "Use in your code:"
echo "  import databento_cpp"
echo "  records = databento_cpp.parse_file_mbo_fast('data.dbn')"
echo ""

