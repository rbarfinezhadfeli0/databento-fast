// Ultra-fast parsing example (200M+ records/sec)
// Direct memory access without callbacks for maximum performance

#include <databento/parser.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    std::cerr << "Example: " << argv[0] << " ES_FUT_20250101.dbn\n";
    return 1;
  }

  std::cout << "🚀 Ultra-Fast DBN Parser (Direct Memory Access)\n";
  std::cout << "Target: 200M+ records/sec\n";
  std::cout << "File: " << argv[1] << "\n\n";

  try {
    // Load file into memory
    databento::DbnParser parser(argv[1]);
    parser.load_into_memory();

    std::cout << "File loaded: " << parser.num_records() << " records\n";
    std::cout << "File size: " << parser.size() / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Starting benchmark...\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Direct memory access - no callback overhead
    const uint8_t* data = parser.data();
    const size_t total = parser.num_records();
    const size_t offset = parser.metadata_offset();
    const size_t rec_size = parser.record_size();
    
    uint64_t checksum = 0;
    
    // Ultra-fast loop with minimal operations
    for (size_t i = 0; i < total; ++i) {
      const uint8_t* record = data + offset + (i * rec_size);
      
      // Read only essential fields (same as Rust benchmark)
      uint64_t ts_event = databento::read_u64_le(record);
      uint32_t instrument_id = databento::read_u32_le(record + 8);
      
      // Accumulate to prevent compiler optimization removal
      checksum ^= ts_event;
      checksum ^= instrument_id;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    // Print results
    double rate = total / elapsed;
    double throughput = (total * 48.0) / (elapsed * 1024 * 1024 * 1024);

    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "ULTRA-FAST RESULTS\n";
    std::cout << std::string(70, '=') << "\n";
    std::cout << "Total records: " << total << "\n";
    std::cout << "Elapsed time:  " << std::fixed << std::setprecision(6) << elapsed << " seconds\n";
    std::cout << "Rate:          " << std::fixed << std::setprecision(0) << rate << " records/sec\n";
    std::cout << "Throughput:    " << std::fixed << std::setprecision(2) << throughput << " GB/s\n";
    std::cout << "Checksum:      " << std::hex << checksum << std::dec << " (prevents optimization)\n";
    std::cout << std::string(70, '=') << "\n";

    // Performance rating
    uint64_t rate_millions = static_cast<uint64_t>(rate / 1'000'000);
    if (rate >= 200'000'000) {
      std::cout << "🏆 EXCELLENT! " << rate_millions << "M records/sec - Competitive with Rust!\n";
    } else if (rate >= 150'000'000) {
      std::cout << "🚀 GREAT! " << rate_millions << "M records/sec\n";
    } else if (rate >= 100'000'000) {
      std::cout << "✅ GOOD! " << rate_millions << "M records/sec\n";
    } else {
      std::cout << "⚠️  Lower than expected. Make sure you compiled with -O3 -march=native\n";
    }

  } catch (const std::exception& e) {
    std::cerr << "❌ Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}


