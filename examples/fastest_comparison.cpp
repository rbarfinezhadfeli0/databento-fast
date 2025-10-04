// Compare: Regular Load vs Memory Mapping
// Shows which is faster for your 15GB file

#include <databento/parser.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>

void benchmark_method(const char* method_name, databento::DbnParser& parser) {
    const size_t total = parser.num_records();
    const uint8_t* base = parser.data() + parser.metadata_offset();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    uint64_t checksum = 0;
    for (size_t i = 0; i < total; ++i) {
        checksum ^= databento::read_u64_le(base + i * 48);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    if (checksum == 0) std::cerr << "";  // Prevent optimization
    
    std::cout << method_name << ": " << static_cast<uint64_t>(total / elapsed) << " rec/s in " 
              << std::fixed << std::setprecision(2) << elapsed << "s\n";
}

int main() {
    const char* filepath = "/home/nvidia/github-v1/tradning/rithmic_v1/dbn/data.dbn";
    
    // Test 1: Memory mapping
    std::cout << "\n=== Method 1: Memory Mapping (mmap) ===\n";
    auto t1 = std::chrono::high_resolution_clock::now();
    databento::DbnParser parser1(filepath);
    parser1.load_with_mmap();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Load: " << std::chrono::duration<double>(t2 - t1).count() << "s\n";
    benchmark_method("Process", parser1);
    
    // Test 2: Regular load
    std::cout << "\n=== Method 2: Regular Load (read) ===\n";
    t1 = std::chrono::high_resolution_clock::now();
    databento::DbnParser parser2(filepath);
    parser2.load_into_memory();
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Load: " << std::chrono::duration<double>(t2 - t1).count() << "s\n";
    benchmark_method("Process", parser2);
    
    std::cout << "\n";
    return 0;
}

