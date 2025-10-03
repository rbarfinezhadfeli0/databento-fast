// Simple MBO parsing example
#include <databento/parser.hpp>
#include <iostream>
#include <iomanip>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbn_file>\n";
    return 1;
  }

  std::cout << "Parsing MBO file: " << argv[1] << "\n\n";

  // Simple callback that prints first 10 records
  uint64_t count = 0;
  auto callback = [&count](const databento::MboMsg& msg) {
    if (count < 10) {
      std::cout << "Record " << count << ":\n";
      std::cout << "  ts_event:      " << msg.ts_event << "\n";
      std::cout << "  instrument_id: " << msg.instrument_id << "\n";
      std::cout << "  action:        " << msg.action << "\n";
      std::cout << "  side:          " << msg.side << "\n";
      std::cout << "  price:         " << std::fixed << std::setprecision(2) 
                << databento::price_to_double(msg.price) << "\n";
      std::cout << "  size:          " << msg.size << "\n";
      std::cout << "  order_id:      " << msg.order_id << "\n\n";
    }
    ++count;
  };

  // Parse and measure performance
  auto stats = databento::parse_file_mbo(argv[1], callback);
  stats.print();

  return 0;
}

