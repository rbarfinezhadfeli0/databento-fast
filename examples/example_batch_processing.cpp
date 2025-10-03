// Batch processing example - middle ground between callbacks and direct access
#include <databento/parser.hpp>
#include <iostream>
#include <numeric>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    return 1;
  }

  std::cout << "Batch Processing Example\n";
  std::cout << "Processes records in batches for better cache locality\n\n";

  databento::DbnParser parser(argv[1]);
  parser.load_into_memory();

  databento::BatchProcessor batch_proc(512 * 1024); // 512K records per batch

  // Calculate VWAP (Volume Weighted Average Price) per batch
  uint64_t total_count = 0;
  
  auto start = std::chrono::high_resolution_clock::now();

  auto batch_callback = [&total_count](const std::vector<databento::MboMsg>& batch) {
    // Calculate batch statistics
    double total_price = 0.0;
    uint64_t total_volume = 0;
    
    for (const auto& msg : batch) {
      if (msg.action == 'A' || msg.action == 'M') {
        total_price += databento::price_to_double(msg.price) * msg.size;
        total_volume += msg.size;
      }
    }
    
    if (total_volume > 0) {
      double vwap = total_price / total_volume;
      std::cout << "Batch " << (total_count / batch.size()) 
                << ": VWAP = " << vwap 
                << ", Volume = " << total_volume 
                << ", Records = " << batch.size() << "\n";
    }
    
    total_count += batch.size();
  };

  batch_proc.process_batches<databento::MboMsg>(parser, batch_callback);

  auto end = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double>(end - start).count();

  std::cout << "\nProcessed " << total_count << " records in " 
            << elapsed << " seconds\n";
  std::cout << "Rate: " << static_cast<uint64_t>(total_count / elapsed) 
            << " records/sec\n";

  return 0;
}

