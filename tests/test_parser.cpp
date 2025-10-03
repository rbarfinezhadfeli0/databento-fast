#include <gtest/gtest.h>
#include <databento/parser.hpp>
#include <databento/dbn.hpp>
#include <vector>
#include <fstream>
#include <cstdio>

// ============================================================================
// Test Helper: Create minimal test DBN file
// ============================================================================

class TestDbnFile {
public:
  TestDbnFile() : path_("/tmp/test_databento_cpp.dbn") {
    create_test_file();
  }

  ~TestDbnFile() {
    std::remove(path_.c_str());
  }

  const std::string& path() const { return path_; }

private:
  std::string path_;

  void create_test_file() {
    std::ofstream file(path_, std::ios::binary);
    
    // Write 200 bytes of metadata (simplified)
    std::vector<uint8_t> metadata(200, 0);
    metadata[0] = 1; // version
    file.write(reinterpret_cast<const char*>(metadata.data()), 200);

    // Write 10 test MBO records (48 bytes each)
    for (int i = 0; i < 10; ++i) {
      databento::MboMsg msg{};
      msg.ts_event = 1000000000ULL + i * 1000;
      msg.instrument_id = 1234 + i;
      msg.action = 'A';
      msg.side = (i % 2 == 0) ? 'B' : 'A';
      msg.flags = 0;
      msg.depth = 0;
      msg.price = 5000'000'000'000LL + i * 1'000'000'000LL; // 5000.00 + i
      msg.size = 100 + i * 10;
      msg.channel_id = 1;
      msg.order_id = 10000ULL + i;
      msg.sequence = i;
      msg.ts_in_delta = 0;

      file.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
    }

    file.close();
  }
};

// ============================================================================
// DbnParser Tests
// ============================================================================

TEST(DbnParserTest, LoadFile) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  parser.load_into_memory();

  EXPECT_EQ(parser.num_records(), 10);
  EXPECT_EQ(parser.record_size(), 48);
  EXPECT_GT(parser.size(), 0);
  EXPECT_EQ(parser.metadata_offset(), 200);
}

TEST(DbnParserTest, FileNotFound) {
  databento::DbnParser parser("/nonexistent/file.dbn");
  
  EXPECT_THROW(parser.load_into_memory(), std::runtime_error);
}

TEST(DbnParserTest, ParseMbo) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  
  std::vector<databento::MboMsg> messages;
  parser.parse_mbo([&messages](const databento::MboMsg& msg) {
    messages.push_back(msg);
  });

  ASSERT_EQ(messages.size(), 10);

  // Check first record
  EXPECT_EQ(messages[0].instrument_id, 1234);
  EXPECT_EQ(messages[0].action, 'A');
  EXPECT_EQ(messages[0].side, 'B');
  EXPECT_EQ(messages[0].size, 100);
  EXPECT_EQ(messages[0].ts_event, 1000000000ULL);

  // Check last record
  EXPECT_EQ(messages[9].instrument_id, 1243);
  EXPECT_EQ(messages[9].size, 190);
  EXPECT_EQ(messages[9].side, 'A');
}

TEST(DbnParserTest, DirectAccess) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  parser.load_into_memory();

  // Test get_record
  const uint8_t* record0 = parser.get_record(0);
  EXPECT_NE(record0, nullptr);

  databento::MboMsg msg0 = databento::parse_mbo(record0);
  EXPECT_EQ(msg0.instrument_id, 1234);
  EXPECT_EQ(msg0.size, 100);
  EXPECT_EQ(msg0.action, 'A');

  // Test get_record at end
  const uint8_t* record9 = parser.get_record(9);
  databento::MboMsg msg9 = databento::parse_mbo(record9);
  EXPECT_EQ(msg9.instrument_id, 1243);
  EXPECT_EQ(msg9.size, 190);
}

TEST(DbnParserTest, OutOfRangeThrows) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  parser.load_into_memory();

  EXPECT_THROW(parser.get_record(100), std::out_of_range);
  EXPECT_THROW(parser.get_batch(0, 100), std::out_of_range);
}

TEST(DbnParserTest, BatchAccess) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  parser.load_into_memory();

  // Get batch of 5 records
  const uint8_t* batch = parser.get_batch(0, 5);
  EXPECT_NE(batch, nullptr);

  // Verify first record in batch
  databento::MboMsg msg = databento::parse_mbo(batch);
  EXPECT_EQ(msg.instrument_id, 1234);
  
  // Verify second record in batch
  databento::MboMsg msg1 = databento::parse_mbo(batch + 48);
  EXPECT_EQ(msg1.instrument_id, 1235);
}

// ============================================================================
// Binary Reader Tests
// ============================================================================

TEST(BinaryReadersTest, ReadU32LE) {
  uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
  uint32_t val = databento::read_u32_le(data);
  EXPECT_EQ(val, 0x04030201U);
}

TEST(BinaryReadersTest, ReadU64LE) {
  uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  uint64_t val = databento::read_u64_le(data);
  EXPECT_EQ(val, 0x0807060504030201ULL);
}

// ============================================================================
// Price Conversion Tests
// ============================================================================

TEST(PriceConversionTest, PriceToDouble) {
  int64_t price_fixed = 5000'000'000'000LL; // 5000.00
  double price_double = databento::price_to_double(price_fixed);
  EXPECT_NEAR(price_double, 5000.0, 0.01);
}

TEST(PriceConversionTest, DoubleToPrice) {
  double price_double = 1234.56;
  int64_t price_fixed = databento::double_to_price(price_double);
  EXPECT_EQ(price_fixed, 1234'560'000'000LL);
}

TEST(PriceConversionTest, RoundTrip) {
  int64_t original = 9999'999'000'000LL;
  double as_double = databento::price_to_double(original);
  int64_t back = databento::double_to_price(as_double);
  EXPECT_EQ(back, original);
}

// ============================================================================
// Batch Processor Tests
// ============================================================================

TEST(BatchProcessorTest, ProcessBatches) {
  TestDbnFile test_file;
  
  databento::DbnParser parser(test_file.path());
  parser.load_into_memory();

  databento::BatchProcessor batch_proc(5); // Small batch for test

  uint64_t total_count = 0;
  uint64_t batch_count = 0;

  auto callback = [&](const std::vector<databento::MboMsg>& batch) {
    batch_count++;
    total_count += batch.size();
    EXPECT_LE(batch.size(), 5);
    
    // Verify first record of first batch
    if (batch_count == 1) {
      EXPECT_EQ(batch[0].instrument_id, 1234);
    }
  };

  batch_proc.process_batches<databento::MboMsg>(parser, callback);

  EXPECT_EQ(total_count, 10);
  EXPECT_EQ(batch_count, 2); // 10 records / 5 per batch = 2 batches
}

TEST(BatchProcessorTest, SetBatchSize) {
  databento::BatchProcessor batch_proc(1024);
  EXPECT_EQ(batch_proc.batch_size(), 1024);
  
  batch_proc.set_batch_size(2048);
  EXPECT_EQ(batch_proc.batch_size(), 2048);
}

// ============================================================================
// High-Level API Tests
// ============================================================================

TEST(HighLevelAPITest, ParseFileMbo) {
  TestDbnFile test_file;
  
  uint64_t count = 0;
  auto callback = [&count](const databento::MboMsg& msg) {
    (void)msg;
    ++count;
  };
  
  auto stats = databento::parse_file_mbo(test_file.path(), callback);
  
  EXPECT_EQ(stats.total_records, 10);
  EXPECT_EQ(count, 10);
  EXPECT_GT(stats.records_per_second, 0);
  EXPECT_GT(stats.elapsed_seconds, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
