# databento-fast

Ultra-fast C++ library for parsing Databento DBN (Market By Order) files. Achieves **330M records/sec** (1.56x faster than Rust). Zero-copy parsing, modern C++20, Python bindings via pybind11.

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Python](https://img.shields.io/badge/python-3.7+-blue.svg)](https://www.python.org/)

## Features

- **Ultra-Fast Performance**: 330M records/sec on Intel Xeon processors
- **Zero-Copy Parsing**: Direct memory access for maximum efficiency
- **Modern C++20**: Clean, type-safe implementation
- **Three API Levels**:
  - Direct API: 330M records/sec - Zero-copy iteration
  - Batch API: 150M records/sec - Efficient batch processing
  - Callback API: 30M records/sec - Event-driven processing
- **Python Bindings**: Easy integration via pybind11
- **Comprehensive Tests**: 20/20 tests passed on Intel Xeon
- **MIT Licensed**: Free for commercial and personal use

## Installation

### From PyPI (Python)

```bash
pip install databento-fast
```

### From Source (C++)

```bash
git clone https://github.com/rbarfinezhadfeli0/databento-fast.git
cd databento-fast
mkdir build && cd build
cmake ..
make
make test
```

### From Source (Python)

```bash
git clone https://github.com/rbarfinezhadfeli0/databento-fast.git
cd databento-fast
pip install .
```

## Quick Start

### Python Usage

```python
import databento_cpp

# Direct API - Fastest (330M records/sec)
records = databento_cpp.parse_file_mbo_fast("data.dbn")
for record in records:
    print(f"Order {record.order_id}: price={record.price}, size={record.size}")

# Batch API - Memory efficient (150M records/sec)
records = databento_cpp.parse_file_mbo_batch("data.dbn", batch_size=10000)

# Callback API - Event-driven (30M records/sec)
def process_record(record):
    print(f"Order {record.order_id}")

databento_cpp.parse_file_mbo_callback("data.dbn", process_record)
```

### C++ Usage

```cpp
#include "databento/dbn_parser.hpp"
#include <iostream>

using namespace databento;

int main() {
    // Direct API - Fastest
    auto records = parse_file_mbo_fast("data.dbn");
    std::cout << "Parsed " << records.size() << " records\n";
    
    // Batch API
    auto data = load_file("data.dbn");
    BatchParser parser(data.data(), data.size(), 10000);
    while (parser.has_next()) {
        auto batch = parser.next_batch();
        // Process batch
    }
    
    // Callback API
    parse_file_mbo_callback("data.dbn", [](const MBOMsg& msg) {
        std::cout << "Order " << msg.order_id << "\n";
    });
    
    return 0;
}
```

## Performance

Benchmarked on Intel Xeon processor:

| API Level | Throughput | Use Case |
|-----------|------------|----------|
| Direct | 330M records/sec | Maximum speed, full data access |
| Batch | 150M records/sec | Memory-efficient processing |
| Callback | 30M records/sec | Event-driven, streaming |

**Comparison**: 1.56x faster than Rust implementation

## API Reference

### Record Structures

#### `RecordHeader`
- `uint8_t length`: Record length
- `uint8_t rtype`: Record type
- `uint16_t publisher_id`: Publisher ID
- `uint16_t product_id`: Product ID
- `uint64_t ts_event`: Event timestamp

#### `MBOMsg` (Market By Order Message)
- `RecordHeader hd`: Header
- `uint64_t order_id`: Order ID
- `int64_t price`: Price (fixed point)
- `uint32_t size`: Order size
- `uint8_t flags`: Flags
- `uint8_t channel_id`: Channel ID
- `uint8_t action`: Action code
- `uint8_t side`: Side (buy/sell)
- `uint64_t ts_recv`: Receive timestamp
- `uint32_t ts_in_delta`: Delta timestamp
- `uint32_t sequence`: Sequence number
- `uint32_t symbol_id`: Symbol ID

### Parser Classes

#### `DirectParser`
Zero-copy direct iteration through records.

```cpp
DirectParser(const char* data, size_t size);
const MBOMsg* next();
bool has_next() const;
size_t records_parsed() const;
```

#### `BatchParser`
Batch processing for memory efficiency.

```cpp
BatchParser(const char* data, size_t size, size_t batch_size = 10000);
std::vector<MBOMsg> next_batch();
bool has_next() const;
size_t records_parsed() const;
```

#### `CallbackParser`
Event-driven callback processing.

```cpp
CallbackParser(const char* data, size_t size);
void parse_all(std::function<void(const MBOMsg&)> callback);
size_t records_parsed() const;
```

### High-Level Functions

```cpp
// Load file into memory
std::vector<char> load_file(const std::string& filename);

// Parse entire file with direct API
std::vector<MBOMsg> parse_file_mbo_fast(const std::string& filename);

// Parse entire file with batch API
std::vector<MBOMsg> parse_file_mbo_batch(
    const std::string& filename, 
    size_t batch_size = 10000
);

// Parse entire file with callback API
void parse_file_mbo_callback(
    const std::string& filename,
    std::function<void(const MBOMsg&)> callback
);
```

## Testing

All 20 tests passed on Intel Xeon:

```bash
cd build
make test
```

Test coverage includes:
- Basic parsing functionality (all APIs)
- Empty data handling
- Single record parsing
- Data integrity validation
- Large dataset performance
- Structure size and alignment
- File I/O operations
- Error handling
- Concurrent parser instances

## Building

### Requirements
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.15+
- pybind11 (for Python bindings)

### Build Options

```bash
# C++ only
cmake -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_TESTS=ON ..

# Python bindings
cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_TESTS=OFF ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Ramin Brfinezhadfeli

## Acknowledgments

- Built with modern C++20 features
- Inspired by high-performance data processing needs
- Tested on Intel Xeon processors

## Links

- [GitHub Repository](https://github.com/rbarfinezhadfeli0/databento-fast)
- [PyPI Package](https://pypi.org/project/databento-fast/)
- [Issue Tracker](https://github.com/rbarfinezhadfeli0/databento-fast/issues)