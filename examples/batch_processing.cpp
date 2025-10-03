// Batch processing example
// Process records in large batches for better cache locality

#include <databento/parser.hpp>
#include <iostream>
#include <numeric>
#include <chrono>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    std::cerr << "Example: " << argv[0] << " ES_FUT_20250101.dbn\n";
    return 1;
  }

  std::cout << "Batch Processing Example\n";
  std::cout << "Processes records in 512K batches for better cache locality\n";
  std::cout << "File: " << argv[1] << "\n\n";

  try {
    databento::DbnParser parser(argv[1]);
    parser.load_into_memory();

    std::cout << "Loaded " << parser.num_records() << " records\n\n";

    databento::BatchProcessor batch_proc(512 * 1024); // 512K records per batch

    // Calculate VWAP (Volume Weighted Average Price) per batch
    uint64_t total_count = 0;
    uint64_t batch_num = 0;
    
    auto start = std::chrono::high_resolution_clock::now();

    auto batch_callback = [&](const std::vector<databento::MboMsg>& batch) {
      // Calculate batch statistics
      double total_price_weighted = 0.0;
      uint64_t total_volume = 0;
      uint64_t add_count = 0;
      uint64_t cancel_count = 0;
      uint64_t modify_count = 0;
      
      for (const auto& msg : batch) {
        if (msg.action == 'A') {
          ++add_count;
          total_price_weighted += databento::price_to_double(msg.price) * msg.size;
          total_volume += msg.size;
        } else if (msg.action == 'C') {
          ++cancel_count;
        } else if (msg.action == 'M') {
          ++modify_count;
        }
      }
      
      if (batch_num < 5) {  // Print first 5 batches
        std::cout << "Batch " << batch_num << ":\n";
        std::cout << "  Records: " << batch.size() << "\n";
        std::cout << "  Adds:    " << add_count << "\n";
        std::cout << "  Cancels: " << cancel_count << "\n";
        std::cout << "  Modifies:" << modify_count << "\n";
        if (total_volume > 0) {
          double vwap = total_price_weighted / total_volume;
          std::cout << "  VWAP:    $" << vwap << "\n";
          std::cout << "  Volume:  " << total_volume << "\n";
        }
        std::cout << "\n";
      }
      
      total_count += batch.size();
      ++batch_num;
    };

    batch_proc.process_batches<databento::MboMsg>(parser, batch_callback);

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "Batch Processing Results\n";
    std::cout << std::string(70, '=') << "\n";
    std::cout << "Total records: " << total_count << "\n";
    std::cout << "Total batches: " << batch_num << "\n";
    std::cout << "Batch size:    " << batch_proc.batch_size() << "\n";
    std::cout << "Elapsed time:  " << elapsed << " seconds\n";
    std::cout << "Rate:          " << static_cast<uint64_t>(total_count / elapsed) << " records/sec\n";
    std::cout << std::string(70, '=') << "\n";

    std::cout << "\n✅ Successfully processed " << total_count << " records in " 
              << batch_num << " batches\n";

  } catch (const std::exception& e) {
    std::cerr << "❌ Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}


