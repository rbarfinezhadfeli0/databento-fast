#!/usr/bin/env python3
"""
Minimal example: Download and parse Databento MBO data

Usage:
    python minimal_example.py
"""

import databento as db
import databento_cpp
import os

# Download data from Databento
client = db.Historical("YOUR_API_KEY")

# Get data with your exact parameters
data = client.timeseries.get_range(
    dataset="GLBX.MDP3",
    symbols=["ES.FUT", "NQ.FUT", "CL.FUT"],
    schema="mbo",
    start="2024-09-04T00:00:00.000000Z",
    end="2024-10-02T00:00:00.000000Z",
    stype_in="parent",
    stype_out="instrument_id",
    path="data.dbn"
)

print("Data downloaded to data.dbn")

# Parse with fast C++ library
print("Parsing with databento-fast (ultra-fast C++ parser)...")
records = databento_cpp.parse_file_mbo_fast("data.dbn")

# Print record count
print(f"Processed {len(records):,} records")
