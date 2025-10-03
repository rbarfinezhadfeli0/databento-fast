# databento-fast Usage Guide

## Table of Contents
1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [API Reference](#api-reference)
4. [Performance Tips](#performance-tips)
5. [Examples](#examples)

## Installation

### Python Package (PyPI)

```bash
pip install databento-fast
```

### From Source

#### C++ Library Only
```bash
git clone https://github.com/rbarfinezhadfeli0/databento-fast.git
cd databento-fast
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make test
```

#### Python Bindings
```bash
pip install pybind11
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON ..
make
```

## Quick Start

### Python

```python
import databento_cpp

# Direct API - Fastest (330M records/sec)
records = databento_cpp.parse_file_mbo_fast("data.dbn")
print(f"Parsed {len(records)} records")

# Access record data
for record in records[:10]:  # First 10 records
    print(f"Order {record.order_id}: price={record.price}, size={record.size}")
```

### C++

```cpp
#include "databento/dbn_parser.hpp"
#include <iostream>

using namespace databento;

int main() {
    auto records = parse_file_mbo_fast("data.dbn");
    std::cout << "Parsed " << records.size() << " records\n";
    return 0;
}
```

Compile with:
```bash
g++ -std=c++20 -O3 -I/path/to/include -o my_app my_app.cpp -ldatabento_core
```

## API Reference

### Three API Levels

databento-fast provides three API levels optimized for different use cases:

#### 1. Direct API (330M records/sec)
- **Use Case**: Maximum performance, full data access
- **Features**: Zero-copy parsing, direct memory access
- **Best For**: Batch processing, data analysis

**Python:**
```python
records = databento_cpp.parse_file_mbo_fast("data.dbn")
```

**C++:**
```cpp
auto records = parse_file_mbo_fast("data.dbn");
// or
auto data = load_file("data.dbn");
DirectParser parser(data.data(), data.size());
while (const MBOMsg* msg = parser.next()) {
    // Process msg
}
```

#### 2. Batch API (150M records/sec)
- **Use Case**: Memory-efficient processing
- **Features**: Configurable batch size, balanced performance
- **Best For**: Large files, streaming processing

**Python:**
```python
records = databento_cpp.parse_file_mbo_batch("data.dbn", batch_size=10000)
```

**C++:**
```cpp
auto records = parse_file_mbo_batch("data.dbn", 10000);
// or
auto data = load_file("data.dbn");
BatchParser parser(data.data(), data.size(), 10000);
while (parser.has_next()) {
    auto batch = parser.next_batch();
    // Process batch
}
```

#### 3. Callback API (30M records/sec)
- **Use Case**: Event-driven processing
- **Features**: Low memory footprint, streaming
- **Best For**: Real-time processing, filtering, aggregation

**Python:**
```python
def process_record(record):
    if record.price > 1000000:
        print(f"High price: {record.price}")

databento_cpp.parse_file_mbo_callback("data.dbn", process_record)
```

**C++:**
```cpp
parse_file_mbo_callback("data.dbn", [](const MBOMsg& msg) {
    if (msg.price > 1000000) {
        std::cout << "High price: " << msg.price << "\n";
    }
});
```

### Record Structures

#### RecordHeader
```cpp
struct RecordHeader {
    uint8_t length;          // Record length in bytes
    uint8_t rtype;           // Record type (160 for MBO)
    uint16_t publisher_id;   // Publisher identifier
    uint16_t product_id;     // Product identifier
    uint64_t ts_event;       // Event timestamp (nanoseconds)
};
```

#### MBOMsg (Market By Order Message)
```cpp
struct MBOMsg {
    RecordHeader hd;         // Record header
    uint64_t order_id;       // Unique order identifier
    int64_t price;           // Price (fixed-point)
    uint32_t size;           // Order size
    uint8_t flags;           // Bit flags
    uint8_t channel_id;      // Channel identifier
    uint8_t action;          // Action code (add/modify/delete)
    uint8_t side;            // Side (buy=1, sell=2)
    uint64_t ts_recv;        // Receive timestamp
    uint32_t ts_in_delta;    // Delta timestamp
    uint32_t sequence;       // Sequence number
    uint32_t symbol_id;      // Symbol identifier
};
```

**Python Access:**
```python
record = records[0]
print(f"Order ID: {record.order_id}")
print(f"Price: {record.price}")
print(f"Size: {record.size}")
print(f"Header type: {record.hd.rtype}")
print(f"Timestamp: {record.hd.ts_event}")
```

## Performance Tips

### 1. Choose the Right API
- **Direct API**: Use when you need all records in memory
- **Batch API**: Use for large files with memory constraints
- **Callback API**: Use for filtering or real-time processing

### 2. Optimize Batch Size
```python
# Too small = overhead
records = databento_cpp.parse_file_mbo_batch("data.dbn", batch_size=100)

# Optimal = 10,000-100,000
records = databento_cpp.parse_file_mbo_batch("data.dbn", batch_size=10000)

# Too large = memory pressure
records = databento_cpp.parse_file_mbo_batch("data.dbn", batch_size=1000000)
```

### 3. Use Callback for Filtering
```python
# Inefficient - loads all records
records = databento_cpp.parse_file_mbo_fast("data.dbn")
filtered = [r for r in records if r.price > 1000000]

# Efficient - filters during parsing
high_price_records = []
def filter_callback(record):
    if record.price > 1000000:
        high_price_records.append(record)

databento_cpp.parse_file_mbo_callback("data.dbn", filter_callback)
```

### 4. Compiler Optimizations (C++)
```bash
# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Additional optimizations
g++ -std=c++20 -O3 -march=native -flto ...
```

## Examples

### Example 1: Count Orders by Side

**Python:**
```python
import databento_cpp

buy_count = 0
sell_count = 0

def count_by_side(record):
    global buy_count, sell_count
    if record.side == 1:
        buy_count += 1
    elif record.side == 2:
        sell_count += 1

databento_cpp.parse_file_mbo_callback("data.dbn", count_by_side)
print(f"Buy orders: {buy_count}, Sell orders: {sell_count}")
```

### Example 2: Calculate Price Statistics

**Python:**
```python
import databento_cpp

records = databento_cpp.parse_file_mbo_fast("data.dbn")
prices = [r.price for r in records]

avg_price = sum(prices) / len(prices)
min_price = min(prices)
max_price = max(prices)

print(f"Average: {avg_price}, Min: {min_price}, Max: {max_price}")
```

### Example 3: Find Large Orders

**Python:**
```python
import databento_cpp

def find_large_orders(threshold=1000):
    large_orders = []
    
    def callback(record):
        if record.size >= threshold:
            large_orders.append({
                'order_id': record.order_id,
                'price': record.price,
                'size': record.size
            })
    
    databento_cpp.parse_file_mbo_callback("data.dbn", callback)
    return large_orders

large = find_large_orders(threshold=500)
print(f"Found {len(large)} large orders")
```

### Example 4: Time-Series Analysis

**Python:**
```python
import databento_cpp
from collections import defaultdict

# Group orders by time window
time_buckets = defaultdict(int)

def bucket_by_time(record):
    # Bucket by second
    second = record.hd.ts_event // 1_000_000_000
    time_buckets[second] += 1

databento_cpp.parse_file_mbo_callback("data.dbn", bucket_by_time)

for timestamp, count in sorted(time_buckets.items())[:10]:
    print(f"Time {timestamp}: {count} orders")
```

### Example 5: C++ Advanced Usage

```cpp
#include "databento/dbn_parser.hpp"
#include <iostream>
#include <unordered_map>
#include <chrono>

using namespace databento;

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Load file once
    auto data = load_file("data.dbn");
    
    // Use DirectParser for zero-copy access
    DirectParser parser(data.data(), data.size());
    
    // Track order statistics
    std::unordered_map<uint32_t, uint64_t> symbol_counts;
    int64_t total_volume = 0;
    
    while (const MBOMsg* msg = parser.next()) {
        symbol_counts[msg->symbol_id]++;
        total_volume += msg->size;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Processed " << parser.records_parsed() << " records in "
              << duration.count() << " ms\n";
    std::cout << "Total volume: " << total_volume << "\n";
    std::cout << "Unique symbols: " << symbol_counts.size() << "\n";
    
    return 0;
}
```

## Benchmarking

To benchmark your use case:

**Python:**
```python
import databento_cpp
import time

file = "data.dbn"

# Benchmark Direct API
start = time.time()
records = databento_cpp.parse_file_mbo_fast(file)
elapsed = time.time() - start
records_per_sec = len(records) / elapsed

print(f"Direct API: {records_per_sec/1e6:.1f}M records/sec")
```

**C++:**
```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
auto records = parse_file_mbo_fast("data.dbn");
auto end = std::chrono::high_resolution_clock::now();

auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
double records_per_sec = (records.size() * 1000.0) / ms;
std::cout << "Rate: " << (records_per_sec / 1e6) << "M records/sec\n";
```

## Troubleshooting

### Python Module Not Found
```bash
# Ensure module is in Python path
export PYTHONPATH=/path/to/build:$PYTHONPATH
python3 -c "import databento_cpp; print('Success')"
```

### Build Errors
```bash
# Ensure C++20 compiler
g++ --version  # Should be >= 10.0

# Install dependencies
pip install pybind11
sudo apt-get install cmake
```

### Performance Issues
- Use Release build: `cmake -DCMAKE_BUILD_TYPE=Release`
- Enable optimizations: `-O3 -march=native`
- Choose appropriate API for your use case
- For large files, use Batch or Callback API

## License

MIT License - see LICENSE file for details.
