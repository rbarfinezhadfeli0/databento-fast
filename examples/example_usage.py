#!/usr/bin/env python3
"""
Example usage of databento-fast Python bindings
"""

import databento_cpp
import time

def example_direct_api(filename):
    """Direct API - Fastest (330M records/sec)"""
    print("=== Direct API Example (330M records/sec) ===")
    
    start = time.time()
    records = databento_cpp.parse_file_mbo_fast(filename)
    end = time.time()
    
    print(f"Parsed {len(records)} records in {(end - start) * 1000:.2f} ms")
    
    if records:
        first = records[0]
        print(f"First record - Order ID: {first.order_id}, "
              f"Price: {first.price}, Size: {first.size}")
    
    return records

def example_batch_api(filename):
    """Batch API - Memory efficient (150M records/sec)"""
    print("\n=== Batch API Example (150M records/sec) ===")
    
    start = time.time()
    records = databento_cpp.parse_file_mbo_batch(filename, batch_size=10000)
    end = time.time()
    
    print(f"Parsed {len(records)} records in {(end - start) * 1000:.2f} ms")
    
    return records

def example_callback_api(filename):
    """Callback API - Event-driven (30M records/sec)"""
    print("\n=== Callback API Example (30M records/sec) ===")
    
    count = 0
    total_size = 0
    
    def process_record(record):
        nonlocal count, total_size
        count += 1
        total_size += record.size
        
        if count == 1:
            print(f"First record - Order ID: {record.order_id}")
    
    start = time.time()
    databento_cpp.parse_file_mbo_callback(filename, process_record)
    end = time.time()
    
    print(f"Processed {count} records in {(end - start) * 1000:.2f} ms")
    print(f"Average size: {total_size // count if count > 0 else 0}")

def main():
    import sys
    
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <dbn_file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    try:
        example_direct_api(filename)
        example_batch_api(filename)
        example_callback_api(filename)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
