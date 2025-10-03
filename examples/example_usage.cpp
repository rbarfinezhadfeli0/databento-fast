#include "databento/dbn_parser.hpp"
#include <iostream>
#include <chrono>

using namespace databento;

void example_direct_api(const std::string& filename) {
    std::cout << "=== Direct API Example (330M records/sec) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto records = parse_file_mbo_fast(filename);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parsed " << records.size() << " records in " 
              << duration.count() << " ms" << std::endl;
    
    if (!records.empty()) {
        std::cout << "First record - Order ID: " << records[0].order_id 
                  << ", Price: " << records[0].price 
                  << ", Size: " << records[0].size << std::endl;
    }
}

void example_batch_api(const std::string& filename) {
    std::cout << "\n=== Batch API Example (150M records/sec) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto records = parse_file_mbo_batch(filename, 10000);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parsed " << records.size() << " records in " 
              << duration.count() << " ms" << std::endl;
}

void example_callback_api(const std::string& filename) {
    std::cout << "\n=== Callback API Example (30M records/sec) ===" << std::endl;
    
    size_t count = 0;
    int64_t total_size = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    parse_file_mbo_callback(filename, [&count, &total_size](const MBOMsg& msg) {
        count++;
        total_size += msg.size;
        
        // Process each record here
        if (count == 1) {
            std::cout << "First record - Order ID: " << msg.order_id << std::endl;
        }
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Processed " << count << " records in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average size: " << (count > 0 ? total_size / count : 0) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <dbn_file>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    
    try {
        example_direct_api(filename);
        example_batch_api(filename);
        example_callback_api(filename);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
