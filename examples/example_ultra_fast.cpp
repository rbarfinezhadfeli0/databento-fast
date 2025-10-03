// Ultra-fast parsing example (200M+ records/sec)
// Direct memory access without callbacks
#include <databento/parser.hpp>
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    return 1;
  }

  std::cout << "Ultra-Fast DBN Parser (Direct Memory Access)\n";
  std::cout << "Target: 200M+ records/sec\n\n";

  // Load file into memory
  databento::DbnParser parser(argv[1]);
  parser.load_into_memory();

  std::cout << "File loaded: " << parser.num_records() << " records\n";
  std::cout << "Starting benchmark...\n\n";

  auto start = std::chrono::high_resolution_clock::now();

  // Direct memory access - no callback overhead
  const uint8_t* data = parser.data();
  const size_t total = parser.num_records();
  const size_t rec_size = parser.record_size();
  
  uint64_t checksum = 0;
  
  // Ultra-fast loop with minimal operations
  for (size_t i = 0; i < total; ++i) {
    const uint8_t* record = data + (i * rec_size);
    
    // Read only essential fields (same as Rust/C++ benchmark)
    uint64_t ts_event = databento::read_u64_le(record);
    uint32_t instrument_id = databento::read_u32_le(record + 8);
    
    // Accumulate to prevent optimization removal
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
  std::cout << "Elapsed time:  " << elapsed << " seconds\n";
  std::cout << "Rate:          " << static_cast<uint64_t>(rate) << " records/sec\n";
  std::cout << "Throughput:    " << throughput << " GB/s\n";
  std::cout << "Checksum:      " << checksum << "\n";
  std::cout << std::string(70, '=') << "\n";

  if (rate >= 200'000'000) {
    std::cout << "🚀 EXCELLENT! " << static_cast<uint64_t>(rate / 1'000'000) 
              << "M records/sec\n";
  } else if (rate >= 100'000'000) {
    std::cout << "✅ GREAT! " << static_cast<uint64_t>(rate / 1'000'000) 
              << "M records/sec\n";
  }

  return 0;
}

