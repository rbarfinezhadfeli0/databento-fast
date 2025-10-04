#include "databento/parser.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
      num_records_(0),
      mmap_addr_(nullptr),
      mmap_fd_(-1),
      using_mmap_(false) {
}

DbnParser::~DbnParser() {
  cleanup_mmap();
  // buffer_ automatically cleans up
}

void DbnParser::cleanup_mmap() {
  if (using_mmap_ && mmap_addr_ != nullptr && mmap_addr_ != MAP_FAILED) {
    munmap(mmap_addr_, size_);
    mmap_addr_ = nullptr;
  }
  
  if (mmap_fd_ >= 0) {
    close(mmap_fd_);
    mmap_fd_ = -1;
  }
  
  using_mmap_ = false;
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
  using_mmap_ = false;

  // Calculate number of records
  if (size_ > metadata_offset_) {
    const size_t data_size = size_ - metadata_offset_;
    num_records_ = data_size / record_size_;
  }
}

void DbnParser::load_with_mmap() {
  // Clean up any existing mapping
  cleanup_mmap();
  
  // Open file
  mmap_fd_ = open(filepath_.c_str(), O_RDONLY);
  if (mmap_fd_ < 0) {
    throw std::runtime_error("Failed to open file for mmap: " + filepath_);
  }
  
  // Get file size
  struct stat sb;
  if (fstat(mmap_fd_, &sb) < 0) {
    close(mmap_fd_);
    mmap_fd_ = -1;
    throw std::runtime_error("Failed to get file size: " + filepath_);
  }
  
  size_ = sb.st_size;
  
  // Memory map the file
  mmap_addr_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, mmap_fd_, 0);
  if (mmap_addr_ == MAP_FAILED) {
    close(mmap_fd_);
    mmap_fd_ = -1;
    throw std::runtime_error("Failed to mmap file: " + filepath_);
  }
  
  // Advise kernel about access pattern
  madvise(mmap_addr_, size_, MADV_SEQUENTIAL | MADV_WILLNEED);
  
  data_ = static_cast<const uint8_t*>(mmap_addr_);
  using_mmap_ = true;
  
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
