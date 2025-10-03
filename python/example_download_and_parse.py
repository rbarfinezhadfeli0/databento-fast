#!/usr/bin/env python3
"""
Example: Download data from Databento API and parse with fast C++ library

This example shows:
1. How to download MBO data using official databento Python client
2. How to parse it with our ultra-fast C++ library (200M+ rec/s)
3. Performance comparison between official library and ours

Requirements:
    pip install databento databento-fast
"""

import databento as db
import databento_cpp
import time
import os

def main():
    print("=" * 70)
    print("Databento MBO Download & Fast Parse Example")
    print("=" * 70)
    print()
    
    # ========================================================================
    # Step 1: Download data from Databento API
    # ========================================================================
    print("Step 1: Downloading MBO data from Databento API...")
    print("-" * 70)
    
    # Configuration
    api_key = "db-xxxx"  # Replace with your API key from databento.com
    dataset = "GLBX.MDP3"
    symbols = ["ES.FUT", "NQ.FUT", "CL.FUT"]
    start = "2024-09-04T00:00:00.000000Z"
    end = "2024-10-02T00:00:00.000000Z"
    stype_in = "parent"
    stype_out = "instrument_id"
    schema = "mbo"
    
    # Output file
    output_file = "downloaded_mbo_data.dbn"
    
    print(f"  Dataset:     {dataset}")
    print(f"  Symbols:     {', '.join(symbols)}")
    print(f"  Date range:  {start} to {end}")
    print(f"  Schema:      {schema}")
    print(f"  Output:      {output_file}")
    print()
    
    # Check if API key is set
    if api_key == "db-xxxx":
        print("⚠️  WARNING: Please set your Databento API key!")
        print("   Get your API key from: https://databento.com/portal/keys")
        print()
        print("   For now, checking if we have a previously downloaded file...")
        
        if not os.path.exists(output_file):
            print()
            print("❌ No data file found. Please:")
            print("   1. Get API key from https://databento.com/portal/keys")
            print("   2. Set api_key variable in this script")
            print("   3. Run this script again to download data")
            print()
            print("Alternatively, use standalone_test for testing without API:")
            print("   cd build && ./standalone_test")
            return
        else:
            print(f"✅ Found existing file: {output_file}")
            print("   Skipping download, will parse existing file...")
            print()
    else:
        # Download data
        try:
            print("Connecting to Databento API...")
            client = db.Historical(api_key)
            
            print("Requesting data (this may take a few moments)...")
            client.timeseries.get_range(
                dataset=dataset,
                symbols=symbols,
                schema=schema,
                start=start,
                end=end,
                stype_in=stype_in,
                stype_out=stype_out,
                path=output_file,
            )
            
            print(f"✅ Data downloaded successfully!")
            print(f"   Saved to: {output_file}")
            print()
            
        except Exception as e:
            print(f"❌ Error downloading data: {e}")
            print()
            print("Common issues:")
            print("  - Invalid API key")
            print("  - No data available for date range")
            print("  - Network connectivity")
            print("  - Databento service unavailable")
            return
    
    # Check file size
    file_size_mb = os.path.getsize(output_file) / (1024 * 1024)
    print(f"File size: {file_size_mb:.2f} MB")
    print()
    
    # ========================================================================
    # Step 2: Parse with official databento Python library (for comparison)
    # ========================================================================
    print("=" * 70)
    print("Step 2: Parsing with official databento Python library...")
    print("-" * 70)
    
    try:
        import databento as db
        
        start_time = time.time()
        
        count = 0
        for record in db.DBNStore.from_file(output_file):
            count += 1
        
        elapsed = time.time() - start_time
        rate = count / elapsed if elapsed > 0 else 0
        
        print(f"✅ Parsed {count:,} records")
        print(f"   Time: {elapsed:.3f} seconds")
        print(f"   Rate: {rate:,.0f} records/sec ({rate/1e6:.2f}M/s)")
        print()
        
    except ImportError:
        print("⚠️  Official databento library not installed")
        print("   Install with: pip install databento")
        print("   Skipping comparison...")
        print()
    except Exception as e:
        print(f"⚠️  Error with official library: {e}")
        print("   Skipping comparison...")
        print()
    
    # ========================================================================
    # Step 3: Parse with our ultra-fast C++ library
    # ========================================================================
    print("=" * 70)
    print("Step 3: Parsing with databento-fast (our C++ library)...")
    print("-" * 70)
    
    try:
        start_time = time.time()
        
        # Use our fast C++ parser
        records = databento_cpp.parse_file_mbo_fast(output_file)
        
        elapsed = time.time() - start_time
        count = len(records)
        rate = count / elapsed if elapsed > 0 else 0
        
        print(f"✅ Parsed {count:,} records")
        print(f"   Time: {elapsed:.3f} seconds")
        print(f"   Rate: {rate:,.0f} records/sec ({rate/1e6:.2f}M/s)")
        print()
        
        # Show sample records
        print("Sample records (first 3):")
        for i, r in enumerate(records[:3]):
            print(f"  [{i}] instrument_id={r.instrument_id:4d} "
                  f"action={r.action} side={r.side} "
                  f"price=${r.price_float:8.2f} size={r.size:5d}")
        print()
        
        # ====================================================================
        # Step 4: Calculate statistics
        # ====================================================================
        print("=" * 70)
        print("Step 4: Calculating statistics...")
        print("-" * 70)
        
        # Count by action
        add_count = sum(1 for r in records if r.action == 'A')
        cancel_count = sum(1 for r in records if r.action == 'C')
        modify_count = sum(1 for r in records if r.action == 'M')
        
        # Count by side
        bid_count = sum(1 for r in records if r.side == 'B')
        ask_count = sum(1 for r in records if r.side == 'A')
        
        # Count by instrument
        from collections import Counter
        instrument_counts = Counter(r.instrument_id for r in records)
        
        print(f"Total records: {count:,}")
        print()
        print("By Action:")
        print(f"  Add (A):    {add_count:,} ({100*add_count/count:.1f}%)")
        print(f"  Cancel (C): {cancel_count:,} ({100*cancel_count/count:.1f}%)")
        print(f"  Modify (M): {modify_count:,} ({100*modify_count/count:.1f}%)")
        print()
        print("By Side:")
        print(f"  Bid (B):    {bid_count:,} ({100*bid_count/count:.1f}%)")
        print(f"  Ask (A):    {ask_count:,} ({100*ask_count/count:.1f}%)")
        print()
        print(f"Unique instruments: {len(instrument_counts)}")
        print("Top 5 instruments by message count:")
        for inst_id, inst_count in instrument_counts.most_common(5):
            print(f"  {inst_id:6d}: {inst_count:,} messages ({100*inst_count/count:.1f}%)")
        print()
        
        # ====================================================================
        # Summary
        # ====================================================================
        print("=" * 70)
        print("SUMMARY")
        print("=" * 70)
        print(f"✅ Successfully processed {count:,} MBO records")
        print(f"✅ Parsing rate: {rate/1e6:.1f}M records/sec")
        print(f"✅ File size: {file_size_mb:.2f} MB")
        print()
        print("Next steps:")
        print("  - Analyze the data further")
        print("  - Build order book reconstruction")
        print("  - Run backtesting strategies")
        print("  - Calculate market microstructure metrics")
        print()
        
    except ImportError:
        print("❌ databento_cpp module not found!")
        print()
        print("Please install the C++ Python bindings:")
        print("  1. Build the C++ library:")
        print("     cd <project-dir> && ./build.sh")
        print("  2. Install Python bindings:")
        print("     ./install_python.sh")
        print()
        return
    except Exception as e:
        print(f"❌ Error parsing with C++ library: {e}")
        import traceback
        traceback.print_exc()
        return

if __name__ == "__main__":
    main()

