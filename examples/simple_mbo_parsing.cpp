// Simple MBO parsing example
// Demonstrates basic usage of the databento-cpp library

#include <databento/parser.hpp>
#include <iostream>
#include <iomanip>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    std::cerr << "Example: " << argv[0] << " ES_FUT_20250101.dbn\n";
    return 1;
  }

  std::cout << "Simple MBO Parser Example\n";
  std::cout << "File: " << argv[1] << "\n\n";

  try {
    // Simple callback that prints first 10 records
    uint64_t count = 0;
    auto callback = [&count](const databento::MboMsg& msg) {
      if (count < 10) {
        std::cout << "Record " << count << ":\n";
        std::cout << "  ts_event:      " << msg.ts_event << " ns\n";
        std::cout << "  instrument_id: " << msg.instrument_id << "\n";
        std::cout << "  action:        " << msg.action << "\n";
        std::cout << "  side:          " << msg.side << "\n";
        std::cout << "  price:         $" << std::fixed << std::setprecision(2) 
                  << databento::price_to_double(msg.price) << "\n";
        std::cout << "  size:          " << msg.size << "\n";
        std::cout << "  order_id:      " << msg.order_id << "\n\n";
      }
      ++count;
    };

    // Parse and measure performance
    auto stats = databento::parse_file_mbo(argv[1], callback);
    stats.print();

    std::cout << "\n✅ Successfully parsed " << count << " MBO records\n";

  } catch (const std::exception& e) {
    std::cerr << "❌ Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}


