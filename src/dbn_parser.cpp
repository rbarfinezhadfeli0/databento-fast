#include "databento/dbn_parser.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace databento {

// DirectParser implementation
DirectParser::DirectParser(const char* data, size_t size)
    : data_(data), size_(size), offset_(0), records_count_(0) {
}

const MBOMsg* DirectParser::next() {
    if (!has_next()) {
        return nullptr;
    }
    
    const MBOMsg* msg = reinterpret_cast<const MBOMsg*>(data_ + offset_);
    offset_ += sizeof(MBOMsg);
    records_count_++;
    
    return msg;
}

bool DirectParser::has_next() const {
    return offset_ + sizeof(MBOMsg) <= size_;
}

size_t DirectParser::records_parsed() const {
    return records_count_;
}

// BatchParser implementation
BatchParser::BatchParser(const char* data, size_t size, size_t batch_size)
    : data_(data), size_(size), offset_(0), batch_size_(batch_size), records_count_(0) {
}

std::vector<MBOMsg> BatchParser::next_batch() {
    std::vector<MBOMsg> batch;
    batch.reserve(batch_size_);
    
    while (batch.size() < batch_size_ && has_next()) {
        const MBOMsg* msg = reinterpret_cast<const MBOMsg*>(data_ + offset_);
        batch.push_back(*msg);
        offset_ += sizeof(MBOMsg);
        records_count_++;
    }
    
    return batch;
}

bool BatchParser::has_next() const {
    return offset_ + sizeof(MBOMsg) <= size_;
}

size_t BatchParser::records_parsed() const {
    return records_count_;
}

// CallbackParser implementation
CallbackParser::CallbackParser(const char* data, size_t size)
    : data_(data), size_(size), records_count_(0) {
}

void CallbackParser::parse_all(Callback callback) {
    size_t offset = 0;
    
    while (offset + sizeof(MBOMsg) <= size_) {
        const MBOMsg* msg = reinterpret_cast<const MBOMsg*>(data_ + offset);
        callback(*msg);
        offset += sizeof(MBOMsg);
        records_count_++;
    }
}

size_t CallbackParser::records_parsed() const {
    return records_count_;
}

// File loading utilities
std::vector<char> load_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }
    
    return buffer;
}

// High-level API functions
std::vector<MBOMsg> parse_file_mbo_fast(const std::string& filename) {
    auto data = load_file(filename);
    DirectParser parser(data.data(), data.size());
    
    std::vector<MBOMsg> records;
    records.reserve(data.size() / sizeof(MBOMsg));
    
    while (const MBOMsg* msg = parser.next()) {
        records.push_back(*msg);
    }
    
    return records;
}

std::vector<MBOMsg> parse_file_mbo_batch(const std::string& filename, size_t batch_size) {
    auto data = load_file(filename);
    BatchParser parser(data.data(), data.size(), batch_size);
    
    std::vector<MBOMsg> records;
    records.reserve(data.size() / sizeof(MBOMsg));
    
    while (parser.has_next()) {
        auto batch = parser.next_batch();
        records.insert(records.end(), batch.begin(), batch.end());
    }
    
    return records;
}

void parse_file_mbo_callback(const std::string& filename, CallbackParser::Callback callback) {
    auto data = load_file(filename);
    CallbackParser parser(data.data(), data.size());
    parser.parse_all(callback);
}

} // namespace databento
