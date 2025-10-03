// Standalone test that creates its own test data
// Can be run without external DBN files

#include <databento/parser.hpp>
#include <databento/dbn.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <cmath>

// Create a minimal test DBN file
std::string create_test_file() {
  std::string path = "/tmp/standalone_test.dbn";
  std::ofstream file(path, std::ios::binary);
  
  if (!file) {
    throw std::runtime_error("Failed to create test file");
  }
  
  // Write 200 bytes of metadata (simplified)
  std::vector<uint8_t> metadata(200, 0);
  metadata[0] = 1; // version
  file.write(reinterpret_cast<const char*>(metadata.data()), 200);

  // Write 10 test MBO records (48 bytes each)
  for (int i = 0; i < 10; ++i) {
    databento::MboMsg msg{};
    msg.ts_event = 1000000000ULL + i * 1000;
    msg.instrument_id = 1234 + i;
    msg.action = 'A';
    msg.side = (i % 2 == 0) ? 'B' : 'A';
    msg.flags = 0;
    msg.depth = 0;
    msg.price = 5000'000'000'000LL + i * 1'000'000'000LL; // 5000.00 + i
    msg.size = 100 + i * 10;
    msg.channel_id = 1;
    msg.order_id = 10000ULL + i;
    msg.sequence = i;
    msg.ts_in_delta = 0;

    file.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
  }

  file.close();
  return path;
}

bool test_basic_loading() {
  std::cout << "Test 1: Basic file loading... ";
  try {
    std::string path = create_test_file();
    databento::DbnParser parser(path);
    parser.load_into_memory();
    
    if (parser.num_records() != 10) {
      std::cout << "FAILED (wrong record count: " << parser.num_records() << ")\n";
      return false;
    }
    
    if (parser.record_size() != 48) {
      std::cout << "FAILED (wrong record size: " << parser.record_size() << ")\n";
      return false;
    }
    
    std::cout << "PASSED\n";
    std::remove(path.c_str());
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

bool test_direct_access() {
  std::cout << "Test 2: Direct memory access... ";
  try {
    std::string path = create_test_file();
    databento::DbnParser parser(path);
    parser.load_into_memory();
    
    // Test first record
    const uint8_t* record0 = parser.get_record(0);
    databento::MboMsg msg0 = databento::parse_mbo(record0);
    
    if (msg0.instrument_id != 1234) {
      std::cout << "FAILED (wrong instrument_id: " << msg0.instrument_id << ")\n";
      return false;
    }
    
    if (msg0.action != 'A') {
      std::cout << "FAILED (wrong action: " << msg0.action << ")\n";
      return false;
    }
    
    // Test last record
    const uint8_t* record9 = parser.get_record(9);
    databento::MboMsg msg9 = databento::parse_mbo(record9);
    
    if (msg9.instrument_id != 1243) {
      std::cout << "FAILED (wrong last instrument_id: " << msg9.instrument_id << ")\n";
      return false;
    }
    
    std::cout << "PASSED\n";
    std::remove(path.c_str());
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

bool test_callback_parsing() {
  std::cout << "Test 3: Callback parsing... ";
  try {
    std::string path = create_test_file();
    databento::DbnParser parser(path);
    
    std::vector<databento::MboMsg> messages;
    parser.parse_mbo([&messages](const databento::MboMsg& msg) {
      messages.push_back(msg);
    });
    
    if (messages.size() != 10) {
      std::cout << "FAILED (wrong count: " << messages.size() << ")\n";
      return false;
    }
    
    // Check first record
    if (messages[0].instrument_id != 1234 || messages[0].size != 100) {
      std::cout << "FAILED (wrong first record)\n";
      return false;
    }
    
    // Check last record
    if (messages[9].instrument_id != 1243 || messages[9].size != 190) {
      std::cout << "FAILED (wrong last record)\n";
      return false;
    }
    
    std::cout << "PASSED\n";
    std::remove(path.c_str());
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

bool test_batch_processing() {
  std::cout << "Test 4: Batch processing... ";
  try {
    std::string path = create_test_file();
    databento::DbnParser parser(path);
    parser.load_into_memory();
    
    databento::BatchProcessor batch_proc(5); // Small batch for test
    
    uint64_t total_count = 0;
    uint64_t batch_count = 0;
    
    auto callback = [&](const std::vector<databento::MboMsg>& batch) {
      batch_count++;
      total_count += batch.size();
      
      if (batch.size() > 5) {
        throw std::runtime_error("Batch too large");
      }
    };
    
    batch_proc.process_batches<databento::MboMsg>(parser, callback);
    
    if (total_count != 10) {
      std::cout << "FAILED (wrong total count: " << total_count << ")\n";
      return false;
    }
    
    if (batch_count != 2) {
      std::cout << "FAILED (wrong batch count: " << batch_count << ")\n";
      return false;
    }
    
    std::cout << "PASSED\n";
    std::remove(path.c_str());
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

bool test_price_conversion() {
  std::cout << "Test 5: Price conversion... ";
  try {
    int64_t price_fixed = 5000'000'000'000LL; // 5000.00
    double price_double = databento::price_to_double(price_fixed);
    
    if (std::abs(price_double - 5000.0) > 0.01) {
      std::cout << "FAILED (wrong conversion: " << price_double << ")\n";
      return false;
    }
    
    // Round trip
    int64_t price_back = databento::double_to_price(price_double);
    if (price_back != price_fixed) {
      std::cout << "FAILED (round trip failed)\n";
      return false;
    }
    
    std::cout << "PASSED\n";
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

bool test_performance() {
  std::cout << "Test 6: Performance benchmark... ";
  try {
    std::string path = create_test_file();
    databento::DbnParser parser(path);
    parser.load_into_memory();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Run 1 million iterations to get measurable time
    const size_t iterations = 1'000'000;
    const size_t total = parser.num_records();
    uint64_t checksum = 0;
    
    for (size_t iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < total; ++i) {
        const uint8_t* record = parser.get_record(i);
        uint64_t ts = databento::read_u64_le(record);
        uint32_t id = databento::read_u32_le(record + 8);
        checksum ^= ts ^ id;
      }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    uint64_t total_records = iterations * total;
    double rate = total_records / elapsed;
    
    std::cout << "PASSED\n";
    std::cout << "   Rate: " << static_cast<uint64_t>(rate / 1'000'000) << "M records/sec\n";
    std::cout << "   Checksum: 0x" << std::hex << checksum << std::dec << "\n";
    
    std::remove(path.c_str());
    return true;
  } catch (const std::exception& e) {
    std::cout << "FAILED (exception: " << e.what() << ")\n";
    return false;
  }
}

int main() {
  std::cout << "=========================================\n";
  std::cout << "Standalone Test Suite\n";
  std::cout << "databento-fast library\n";
  std::cout << "=========================================\n\n";
  
  int passed = 0;
  int total = 6;
  
  if (test_basic_loading()) passed++;
  if (test_direct_access()) passed++;
  if (test_callback_parsing()) passed++;
  if (test_batch_processing()) passed++;
  if (test_price_conversion()) passed++;
  if (test_performance()) passed++;
  
  std::cout << "\n=========================================\n";
  std::cout << "Results: " << passed << "/" << total << " tests passed\n";
  std::cout << "=========================================\n\n";
  
  if (passed == total) {
    std::cout << "✅ ALL TESTS PASSED!\n\n";
    std::cout << "The databento-cpp library is working correctly.\n";
    std::cout << "You can now:\n";
    std::cout << "  1. Use it in your C++ projects\n";
    std::cout << "  2. Try the other examples with real DBN files\n";
    std::cout << "  3. Install Python bindings with ./install_python.sh\n";
    return 0;
  } else {
    std::cout << "❌ SOME TESTS FAILED!\n";
    return 1;
  }
}

