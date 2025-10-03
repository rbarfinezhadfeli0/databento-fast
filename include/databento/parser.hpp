#pragma once

#include "dbn.hpp"
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace databento {

// ============================================================================
// Callback Types
// ============================================================================

using MboCallback = std::function<void(const MboMsg&)>;
using TradeCallback = std::function<void(const TradeMsg&)>;

// ============================================================================
// Fast DBN File Parser
// ============================================================================

class DbnParser {
public:
  explicit DbnParser(const std::string& filepath);
  ~DbnParser();

  // Load file into memory (required before parsing)
  void load_into_memory();

  // Parse entire file with callback
  void parse_mbo(MboCallback callback);
  void parse_trade(TradeCallback callback);

  // Direct memory access (zero-copy, maximum performance)
  const uint8_t* data() const { return data_; }
  size_t size() const { return size_; }
  size_t num_records() const { return num_records_; }
  size_t record_size() const { return record_size_; }
  size_t metadata_offset() const { return metadata_offset_; }

  // Get record at index (zero-copy)
  const uint8_t* get_record(size_t index) const;

  // Get batch of records (zero-copy)
  const uint8_t* get_batch(size_t start_index, size_t count) const;

private:
  std::string filepath_;
  const uint8_t* data_;
  size_t size_;
  size_t metadata_offset_;
  size_t record_size_;
  size_t num_records_;
  std::vector<uint8_t> buffer_;
};

// ============================================================================
// Batch Processor (Optimized for Cache Locality)
// ============================================================================

class BatchProcessor {
public:
  static constexpr size_t DEFAULT_BATCH_SIZE = 524288; // 512K records

  explicit BatchProcessor(size_t batch_size = DEFAULT_BATCH_SIZE)
      : batch_size_(batch_size) {}

  // Process in batches
  template<typename RecordType, typename Callback>
  void process_batches(DbnParser& parser, Callback callback) {
    if (!parser.data()) {
      parser.load_into_memory();
    }

    const size_t total = parser.num_records();
    const size_t rec_size = parser.record_size();

    for (size_t i = 0; i < total; i += batch_size_) {
      const size_t batch_count = std::min(batch_size_, total - i);
      std::vector<RecordType> batch;
      batch.reserve(batch_count);

      const uint8_t* batch_data = parser.get_batch(i, batch_count);
      
      for (size_t j = 0; j < batch_count; ++j) {
        const uint8_t* record = batch_data + (j * rec_size);
        if constexpr (std::is_same_v<RecordType, MboMsg>) {
          batch.push_back(parse_mbo(record));
        } else if constexpr (std::is_same_v<RecordType, TradeMsg>) {
          batch.push_back(parse_trade(record));
        }
      }

      callback(batch);
    }
  }

  void set_batch_size(size_t size) { batch_size_ = size; }
  size_t batch_size() const { return batch_size_; }

private:
  size_t batch_size_;
};

// ============================================================================
// Statistics
// ============================================================================

struct ParseStats {
  uint64_t total_records;
  double elapsed_seconds;
  double records_per_second;
  double throughput_gbps;

  void print() const;
};

// ============================================================================
// High-Level Utility Functions
// ============================================================================

ParseStats parse_file_mbo(const std::string& filepath, MboCallback callback);
ParseStats parse_file_trade(const std::string& filepath, TradeCallback callback);

} // namespace databento
