#include "databento/parser.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <cstring>

namespace databento {

// ============================================================================
// DbnParser Implementation
// ============================================================================

DbnParser::DbnParser(const std::string& filepath)
    : filepath_(filepath),
      data_(nullptr),
      size_(0),
      metadata_offset_(200),  // Standard DBN metadata size
      record_size_(48),       // MBO/Trade record size
      num_records_(0) {
}

DbnParser::~DbnParser() {
  // buffer_ automatically cleans up
}

void DbnParser::load_into_memory() {
  std::ifstream file(filepath_, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + filepath_);
  }

  size_ = file.tellg();
  file.seekg(0, std::ios::beg);

  buffer_.resize(size_);
  file.read(reinterpret_cast<char*>(buffer_.data()), size_);
  
  if (!file) {
    throw std::runtime_error("Failed to read file: " + filepath_);
  }
  
  file.close();

  data_ = buffer_.data();

  // Calculate number of records
  if (size_ > metadata_offset_) {
    const size_t data_size = size_ - metadata_offset_;
    num_records_ = data_size / record_size_;
  }
}

void DbnParser::parse_mbo(MboCallback callback) {
  if (!data_) {
    load_into_memory();
  }

  const uint8_t* ptr = data_ + metadata_offset_;
  for (size_t i = 0; i < num_records_; ++i) {
    MboMsg msg;
    std::memcpy(&msg, ptr, sizeof(MboMsg));
    callback(msg);
    ptr += record_size_;
  }
}

void DbnParser::parse_trade(TradeCallback callback) {
  if (!data_) {
    load_into_memory();
  }

  const uint8_t* ptr = data_ + metadata_offset_;
  for (size_t i = 0; i < num_records_; ++i) {
    TradeMsg msg;
    std::memcpy(&msg, ptr, sizeof(TradeMsg));
    callback(msg);
    ptr += record_size_;
  }
}

const uint8_t* DbnParser::get_record(size_t index) const {
  if (index >= num_records_) {
    throw std::out_of_range("Record index out of range");
  }
  return data_ + metadata_offset_ + (index * record_size_);
}

const uint8_t* DbnParser::get_batch(size_t start_index, size_t count) const {
  if (start_index + count > num_records_) {
    throw std::out_of_range("Batch extends beyond data");
  }
  return data_ + metadata_offset_ + (start_index * record_size_);
}

// ============================================================================
// ParseStats Implementation
// ============================================================================

void ParseStats::print() const {
  std::cout << "\n" << std::string(70, '=') << "\n";
  std::cout << "Parse Statistics\n";
  std::cout << std::string(70, '=') << "\n";
  std::cout << "Total records:  " << total_records << "\n";
  std::cout << "Elapsed time:   " << elapsed_seconds << " seconds\n";
  std::cout << "Records/sec:    " << static_cast<uint64_t>(records_per_second) << " rec/s\n";
  std::cout << "Throughput:     " << throughput_gbps << " GB/s\n";
  std::cout << std::string(70, '=') << "\n";
}

// ============================================================================
// High-Level Utility Functions
// ============================================================================

ParseStats parse_file_mbo(const std::string& filepath, MboCallback callback) {
  auto start = std::chrono::high_resolution_clock::now();

  DbnParser parser(filepath);
  parser.load_into_memory();
  parser.parse_mbo(callback);

  auto end = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double>(end - start).count();

  ParseStats stats;
  stats.total_records = parser.num_records();
  stats.elapsed_seconds = elapsed;
  stats.records_per_second = stats.total_records / elapsed;
  stats.throughput_gbps = (stats.total_records * 48.0) / (elapsed * 1024 * 1024 * 1024);

  return stats;
}

ParseStats parse_file_trade(const std::string& filepath, TradeCallback callback) {
  auto start = std::chrono::high_resolution_clock::now();

  DbnParser parser(filepath);
  parser.load_into_memory();
  parser.parse_trade(callback);

  auto end = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double>(end - start).count();

  ParseStats stats;
  stats.total_records = parser.num_records();
  stats.elapsed_seconds = elapsed;
  stats.records_per_second = stats.total_records / elapsed;
  stats.throughput_gbps = (stats.total_records * 48.0) / (elapsed * 1024 * 1024 * 1024);

  return stats;
}

} // namespace databento
