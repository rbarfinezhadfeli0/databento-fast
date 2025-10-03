// Comprehensive benchmark comparing all parsing methods

#include <databento/parser.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>

struct BenchmarkResult {
  std::string method;
  double elapsed;
  uint64_t records;
  double rate;
  double throughput_gbps;
};

void print_results(const std::vector<BenchmarkResult>& results) {
  std::cout << "\n" << std::string(90, '=') << "\n";
  std::cout << "BENCHMARK RESULTS\n";
  std::cout << std::string(90, '=') << "\n";
  std::cout << std::left << std::setw(35) << "Method" 
            << std::right << std::setw(15) << "Time (s)"
            << std::setw(20) << "Records/sec"
            << std::setw(15) << "GB/s" << "\n";
  std::cout << std::string(90, '-') << "\n";

  for (const auto& r : results) {
    std::cout << std::left << std::setw(35) << r.method
              << std::right << std::setw(15) << std::fixed << std::setprecision(6) << r.elapsed
              << std::setw(20) << std::fixed << std::setprecision(0) << r.rate
              << std::setw(15) << std::fixed << std::setprecision(2) << r.throughput_gbps << "\n";
  }
  std::cout << std::string(90, '=') << "\n";
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    std::cerr << "Example: " << argv[0] << " ES_FUT_20250101.dbn\n";
    return 1;
  }

  std::cout << "🚀 Comprehensive Performance Benchmark\n";
  std::cout << "File: " << argv[1] << "\n\n";

  std::vector<BenchmarkResult> results;

  try {
    // ========================================================================
    // Method 1: Direct memory access (fastest)
    // ========================================================================
    std::cout << "[1/4] Benchmarking: Direct Memory Access...\n";
    {
      databento::DbnParser parser(argv[1]);
      parser.load_into_memory();

      const size_t total = parser.num_records();
      std::cout << "      Loaded " << total << " records\n";

      auto start = std::chrono::high_resolution_clock::now();

      const uint8_t* data = parser.data();
      const size_t offset = parser.metadata_offset();
      const size_t rec_size = parser.record_size();
      uint64_t checksum = 0;

      for (size_t i = 0; i < total; ++i) {
        const uint8_t* record = data + offset + (i * rec_size);
        uint64_t ts = databento::read_u64_le(record);
        uint32_t id = databento::read_u32_le(record + 8);
        checksum ^= ts ^ id;
      }

      auto end = std::chrono::high_resolution_clock::now();
      double elapsed = std::chrono::duration<double>(end - start).count();

      results.push_back({
        "Direct Memory Access (Zero-Copy)",
        elapsed,
        total,
        total / elapsed,
        (total * 48.0) / (elapsed * 1024 * 1024 * 1024)
      });

      std::cout << "      Checksum: " << std::hex << checksum << std::dec << "\n";
      std::cout << "      ✅ Complete\n\n";
    }

    // ========================================================================
    // Method 2: Per-record callback
    // ========================================================================
    std::cout << "[2/4] Benchmarking: Per-Record Callback...\n";
    {
      uint64_t count = 0;
      uint64_t checksum = 0;
      
      auto callback = [&](const databento::MboMsg& msg) {
        checksum ^= msg.ts_event ^ msg.instrument_id;
        ++count;
      };

      auto start = std::chrono::high_resolution_clock::now();
      databento::DbnParser parser(argv[1]);
      parser.load_into_memory();
      parser.parse_mbo(callback);
      auto end = std::chrono::high_resolution_clock::now();

      double elapsed = std::chrono::duration<double>(end - start).count();

      results.push_back({
        "Per-Record Callback (Structured)",
        elapsed,
        count,
        count / elapsed,
        (count * 48.0) / (elapsed * 1024 * 1024 * 1024)
      });

      std::cout << "      Processed " << count << " records\n";
      std::cout << "      Checksum: " << std::hex << checksum << std::dec << "\n";
      std::cout << "      ✅ Complete\n\n";
    }

    // ========================================================================
    // Method 3: Batch processing (512K)
    // ========================================================================
    std::cout << "[3/4] Benchmarking: Batch Processing (512K)...\n";
    {
      databento::DbnParser parser(argv[1]);
      parser.load_into_memory();

      databento::BatchProcessor batch_proc(512 * 1024);
      uint64_t count = 0;
      uint64_t checksum = 0;

      auto start = std::chrono::high_resolution_clock::now();

      auto batch_callback = [&](const std::vector<databento::MboMsg>& batch) {
        for (const auto& msg : batch) {
          checksum ^= msg.ts_event ^ msg.instrument_id;
        }
        count += batch.size();
      };

      batch_proc.process_batches<databento::MboMsg>(parser, batch_callback);

      auto end = std::chrono::high_resolution_clock::now();
      double elapsed = std::chrono::duration<double>(end - start).count();

      results.push_back({
        "Batch Processing (512K per batch)",
        elapsed,
        count,
        count / elapsed,
        (count * 48.0) / (elapsed * 1024 * 1024 * 1024)
      });

      std::cout << "      Processed " << count << " records\n";
      std::cout << "      Checksum: " << std::hex << checksum << std::dec << "\n";
      std::cout << "      ✅ Complete\n\n";
    }

    // ========================================================================
    // Method 4: Inline parsing with manual unroll (4x)
    // ========================================================================
    std::cout << "[4/4] Benchmarking: Inline Unrolled (4x)...\n";
    {
      databento::DbnParser parser(argv[1]);
      parser.load_into_memory();

      auto start = std::chrono::high_resolution_clock::now();

      const uint8_t* data = parser.data();
      const size_t offset = parser.metadata_offset();
      const size_t rec_size = parser.record_size();
      const size_t total = parser.num_records();
      uint64_t checksum = 0;

      // Unroll by 4 for better instruction pipelining
      size_t i = 0;
      for (; i + 4 <= total; i += 4) {
        const uint8_t* r0 = data + offset + (i * rec_size);
        const uint8_t* r1 = r0 + rec_size;
        const uint8_t* r2 = r1 + rec_size;
        const uint8_t* r3 = r2 + rec_size;

        checksum ^= databento::read_u64_le(r0);
        checksum ^= databento::read_u64_le(r1);
        checksum ^= databento::read_u64_le(r2);
        checksum ^= databento::read_u64_le(r3);
      }

      // Handle remaining
      for (; i < total; ++i) {
        const uint8_t* record = data + offset + (i * rec_size);
        checksum ^= databento::read_u64_le(record);
      }

      auto end = std::chrono::high_resolution_clock::now();
      double elapsed = std::chrono::duration<double>(end - start).count();

      results.push_back({
        "Inline Unrolled (4x loop unroll)",
        elapsed,
        total,
        total / elapsed,
        (total * 48.0) / (elapsed * 1024 * 1024 * 1024)
      });

      std::cout << "      Processed " << total << " records\n";
      std::cout << "      Checksum: " << std::hex << checksum << std::dec << "\n";
      std::cout << "      ✅ Complete\n\n";
    }

    // ========================================================================
    // Print results and analysis
    // ========================================================================
    print_results(results);

    // Find fastest
    auto fastest = std::max_element(results.begin(), results.end(),
      [](const BenchmarkResult& a, const BenchmarkResult& b) {
        return a.rate < b.rate;
      });

    std::cout << "\n🏆 Fastest Method: " << fastest->method << "\n";
    std::cout << "   Performance: " << static_cast<uint64_t>(fastest->rate / 1'000'000) 
              << " million records/sec\n";
    std::cout << "   Throughput:  " << fastest->throughput_gbps << " GB/s\n\n";

    // Performance rating
    if (fastest->rate >= 200'000'000) {
      std::cout << "🎉 EXCELLENT! Performance is competitive with Rust implementations!\n";
    } else if (fastest->rate >= 150'000'000) {
      std::cout << "🚀 GREAT! Very high performance achieved!\n";
    } else if (fastest->rate >= 100'000'000) {
      std::cout << "✅ GOOD! Solid performance!\n";
    } else {
      std::cout << "⚠️  Performance lower than expected.\n";
      std::cout << "   Tip: Compile with -O3 -march=native for maximum speed.\n";
    }

  } catch (const std::exception& e) {
    std::cerr << "❌ Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
