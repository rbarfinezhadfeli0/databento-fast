#!/usr/bin/env python3
"""
Simple example: Download MBO data and print record count

This is the minimal version that:
1. Downloads data from Databento API
2. Parses with fast C++ library
3. Prints record count
"""

import databento as db
import databento_cpp
import os

# Configuration - Set your API key here
api_key = "db-xxxx"  # Get from https://databento.com/portal/keys
dataset = "GLBX.MDP3"
symbols = ["ES.FUT", "NQ.FUT", "CL.FUT"]
start = "2024-09-04T00:00:00.000000Z"  # September 4, 2024
end = "2024-10-02T00:00:00.000000Z"    # October 2, 2024
stype_in = "parent"
stype_out = "instrument_id"
output_file = "mbo_data.dbn"

def main():
    # Download data if API key is set
    if api_key != "db-xxxx" and not os.path.exists(output_file):
        print(f"Downloading MBO data from {dataset}...")
        print(f"Symbols: {', '.join(symbols)}")
        print(f"Date: {start} to {end}")
        
        client = db.Historical(api_key)
        client.timeseries.get_range(
            dataset=dataset,
            symbols=symbols,
            schema="mbo",
            start=start,
            end=end,
            stype_in=stype_in,
            stype_out=stype_out,
            path=output_file,
        )
        print(f"✅ Downloaded to {output_file}")
    
    # Check if file exists
    if not os.path.exists(output_file):
        print(f"❌ File not found: {output_file}")
        print("Please set your API key or provide a DBN file")
        return
    
    # Parse with fast C++ library
    print(f"\nParsing {output_file}...")
    records = databento_cpp.parse_file_mbo_fast(output_file)
    
    # Print record count
    print(f"✅ Processed {len(records):,} records")
    
    # Optional: Show first record
    if len(records) > 0:
        r = records[0]
        print(f"First record: instrument_id={r.instrument_id}, "
              f"action={r.action}, side={r.side}, "
              f"price=${r.price_float:.2f}, size={r.size}")

if __name__ == "__main__":
    main()

