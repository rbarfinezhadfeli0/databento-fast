#!/usr/bin/env python3
"""
Example: Using databento-cpp from Python

Install with:
    pip install ./databento-cpp

Usage:
    python example_python.py <dbn_file>
"""

import databento_cpp
import sys
import time

def main():
    if len(sys.argv) < 2:
        print("Usage: python example_python.py <dbn_file>")
        print("Example: python example_python.py ES_FUT_20250101.dbn")
        sys.exit(1)

    filepath = sys.argv[1]
    print(f"🚀 Python Example: databento-cpp Fast Parser")
    print(f"File: {filepath}\n")

    # ========================================================================
    # Method 1: Fast bulk load (recommended for most use cases)
    # ========================================================================
    print("=" * 70)
    print("Method 1: Fast bulk load (RECOMMENDED)")
    print("=" * 70)
    
    parser = databento_cpp.DbnParser(filepath)
    parser.load_into_memory()
    
    print(f"Loaded: {parser.num_records()} records")
    print(f"Size:   {parser.size() / (1024**2):.2f} MB")
    print(f"Record: {parser.record_size()} bytes each\n")

    # Access individual records
    print("First 5 records:")
    for i in range(min(5, parser.num_records())):
        msg = parser.get_record_mbo(i)
        print(f"  [{i}] instrument={msg.instrument_id:4d} "
              f"action={msg.action} side={msg.side} "
              f"price=${msg.price_float:8.2f} size={msg.size:5d}")
    print()

    # ========================================================================
    # Method 2: Load all into Python list (convenient for analysis)
    # ========================================================================
    print("=" * 70)
    print("Method 2: Load all into list")
    print("=" * 70)
    
    start = time.time()
    records = databento_cpp.parse_file_mbo_fast(filepath)
    elapsed = time.time() - start
    
    print(f"Loaded {len(records)} records in {elapsed:.3f} seconds")
    print(f"Rate: {len(records) / elapsed / 1e6:.1f}M records/sec\n")

    # Calculate statistics
    bid_count = sum(1 for r in records if r.side == 'B')
    ask_count = sum(1 for r in records if r.side == 'A')
    add_count = sum(1 for r in records if r.action == 'A')
    cancel_count = sum(1 for r in records if r.action == 'C')
    modify_count = sum(1 for r in records if r.action == 'M')

    print(f"Statistics:")
    print(f"  Total records: {len(records)}")
    print(f"  Bids:          {bid_count} ({100*bid_count/len(records):.1f}%)")
    print(f"  Asks:          {ask_count} ({100*ask_count/len(records):.1f}%)")
    print(f"  Adds:          {add_count}")
    print(f"  Cancels:       {cancel_count}")
    print(f"  Modifies:      {modify_count}")
    print()

    # ========================================================================
    # Method 3: Callback (for streaming/large files)
    # ========================================================================
    print("=" * 70)
    print("Method 3: Callback processing")
    print("=" * 70)
    
    count = 0
    checksum = 0
    
    def callback(msg):
        nonlocal count, checksum
        count += 1
        checksum ^= msg.ts_event
        # Process message here
        # if count <= 3:
        #     print(f"  {msg}")

    stats = databento_cpp.parse_file_mbo(filepath, callback)
    print(f"Processed: {count} records")
    print(f"Rate:      {stats.records_per_second / 1e6:.1f}M records/sec")
    print(f"Throughput:{stats.throughput_gbps:.2f} GB/s")
    print(f"Checksum:  0x{checksum:016x}")
    print()

    # ========================================================================
    # Method 4: Convert to pandas DataFrame (if pandas available)
    # ========================================================================
    try:
        import pandas as pd
        print("=" * 70)
        print("Method 4: Convert to pandas DataFrame")
        print("=" * 70)
        
        # Take first 10K records for demo
        sample_size = min(10000, len(records))
        df = pd.DataFrame([{
            'ts_event': r.ts_event,
            'instrument_id': r.instrument_id,
            'action': r.action,
            'side': r.side,
            'price': r.price_float,
            'size': r.size,
            'order_id': r.order_id,
        } for r in records[:sample_size]])
        
        print(f"Created DataFrame with {len(df)} rows\n")
        print(df.head(10))
        print()
        print(df.describe())
        print()
        
    except ImportError:
        print("=" * 70)
        print("pandas not available, skipping DataFrame conversion")
        print("Install with: pip install pandas")
        print("=" * 70)
        print()

    print("✅ All examples completed successfully!")
    print()
    print("💡 Tips:")
    print("  - Use parse_file_mbo_fast() for best performance")
    print("  - Access individual records with get_record_mbo(index)")
    print("  - Use callbacks for streaming large files")
    print()

if __name__ == "__main__":
    main()
