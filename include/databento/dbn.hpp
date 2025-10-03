#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace databento {

// ============================================================================
// Record Types
// ============================================================================

enum class RType : uint8_t {
  Mbo = 0x00,          // Market By Order
  Trade = 0x01,        // Trade
  Mbp1 = 0x02,         // Market By Price - 1 level
  Mbp10 = 0x03,        // Market By Price - 10 levels
  Ohlcv1S = 0x10,      // OHLCV 1 second
  Ohlcv1M = 0x11,      // OHLCV 1 minute
  Ohlcv1H = 0x12,      // OHLCV 1 hour
  Ohlcv1D = 0x13,      // OHLCV 1 day
  Definition = 0x20,   // Instrument definition
  Imbalance = 0x21,    // Imbalance
  Error = 0x22,        // Error
  SymbolMapping = 0x23,// Symbol mapping
  System = 0x24,       // System message
  Statistics = 0x25,   // Statistics
};

// Action codes
enum class Action : char {
  Add = 'A',
  Cancel = 'C',
  Modify = 'M',
  Clear = 'R',
  Trade = 'T',
  Fill = 'F',
};

// Side codes
enum class Side : char {
  Ask = 'A',
  Bid = 'B',
  None = 'N',
};

// Flags
constexpr uint8_t F_LAST = 0x80;
constexpr uint8_t F_TOB = 0x01;

// ============================================================================
// Record Structures (48 bytes each)
// ============================================================================

#pragma pack(push, 1)

// MBO Record (Market By Order)
struct MboMsg {
  uint64_t ts_event;      // Event timestamp (ns)
  uint32_t instrument_id; // Instrument ID
  char action;            // Action: A, C, M, etc.
  char side;              // Side: B, A
  uint8_t flags;          // Flags
  uint8_t depth;          // Depth
  int64_t price;          // Price (fixed point 1e-9)
  uint32_t size;          // Size
  uint32_t channel_id;    // Channel ID
  uint64_t order_id;      // Order ID
  uint32_t sequence;      // Sequence number
  uint8_t ts_in_delta;    // Gateway receive delta
  uint8_t reserved[3];    // Reserved
};

// Trade Record
struct TradeMsg {
  uint64_t ts_event;      // Event timestamp (ns)
  uint32_t instrument_id; // Instrument ID
  char action;            // Action
  char side;              // Side
  uint8_t flags;          // Flags
  uint8_t depth;          // Depth
  int64_t price;          // Price
  uint32_t size;          // Size
  uint32_t channel_id;    // Channel ID
  uint64_t order_id;      // Order ID
  uint32_t sequence;      // Sequence number
  uint8_t ts_in_delta;    // Gateway receive delta
  uint8_t reserved[3];    // Reserved
};

#pragma pack(pop)

// ============================================================================
// Inline Binary Readers (Maximum Performance)
// ============================================================================

inline uint16_t read_u16_le(const uint8_t* ptr) {
  uint16_t val;
  std::memcpy(&val, ptr, sizeof(val));
  return val;
}

inline uint32_t read_u32_le(const uint8_t* ptr) {
  uint32_t val;
  std::memcpy(&val, ptr, sizeof(val));
  return val;
}

inline uint64_t read_u64_le(const uint8_t* ptr) {
  uint64_t val;
  std::memcpy(&val, ptr, sizeof(val));
  return val;
}

inline int64_t read_i64_le(const uint8_t* ptr) {
  int64_t val;
  std::memcpy(&val, ptr, sizeof(val));
  return val;
}

// ============================================================================
// Fast Record Parsers
// ============================================================================

inline MboMsg parse_mbo(const uint8_t* data) {
  MboMsg msg;
  std::memcpy(&msg, data, sizeof(MboMsg));
  return msg;
}

inline TradeMsg parse_trade(const uint8_t* data) {
  TradeMsg msg;
  std::memcpy(&msg, data, sizeof(TradeMsg));
  return msg;
}

// ============================================================================
// Price Conversion Utilities
// ============================================================================

inline double price_to_double(int64_t price) {
  return price / 1e9;
}

inline int64_t double_to_price(double price) {
  return static_cast<int64_t>(price * 1e9);
}

} // namespace databento
