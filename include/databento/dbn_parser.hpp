#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace databento {

// DBN Record Structures
#pragma pack(push, 1)

struct RecordHeader {
    uint8_t length;
    uint8_t rtype;
    uint16_t publisher_id;
    uint16_t product_id;
    uint64_t ts_event;
};

struct MBOMsg {
    RecordHeader hd;
    uint64_t order_id;
    int64_t price;
    uint32_t size;
    uint8_t flags;
    uint8_t channel_id;
    uint8_t action;
    uint8_t side;
    uint64_t ts_recv;
    uint32_t ts_in_delta;
    uint32_t sequence;
    uint32_t symbol_id;
};

#pragma pack(pop)

// Direct API - Zero-copy parsing (330M records/sec)
class DirectParser {
public:
    DirectParser(const char* data, size_t size);
    
    const MBOMsg* next();
    bool has_next() const;
    size_t records_parsed() const;
    
private:
    const char* data_;
    size_t size_;
    size_t offset_;
    size_t records_count_;
};

// Batch API - Efficient batch processing (150M records/sec)
class BatchParser {
public:
    BatchParser(const char* data, size_t size, size_t batch_size = 10000);
    
    std::vector<MBOMsg> next_batch();
    bool has_next() const;
    size_t records_parsed() const;
    
private:
    const char* data_;
    size_t size_;
    size_t offset_;
    size_t batch_size_;
    size_t records_count_;
};

// Callback API - Event-driven processing (30M records/sec)
class CallbackParser {
public:
    using Callback = std::function<void(const MBOMsg&)>;
    
    CallbackParser(const char* data, size_t size);
    
    void parse_all(Callback callback);
    size_t records_parsed() const;
    
private:
    const char* data_;
    size_t size_;
    size_t records_count_;
};

// File loading utilities
std::vector<char> load_file(const std::string& filename);

// High-level API functions
std::vector<MBOMsg> parse_file_mbo_fast(const std::string& filename);
std::vector<MBOMsg> parse_file_mbo_batch(const std::string& filename, size_t batch_size = 10000);
void parse_file_mbo_callback(const std::string& filename, CallbackParser::Callback callback);

} // namespace databento
