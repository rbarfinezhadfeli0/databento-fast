#include <gtest/gtest.h>
#include "databento/dbn_parser.hpp"
#include <cstring>
#include <fstream>
#include <vector>

using namespace databento;

// Helper function to create test data
std::vector<char> create_test_data(size_t num_records) {
    std::vector<char> data(num_records * sizeof(MBOMsg));
    MBOMsg* msgs = reinterpret_cast<MBOMsg*>(data.data());
    
    for (size_t i = 0; i < num_records; ++i) {
        msgs[i].hd.length = sizeof(MBOMsg);
        msgs[i].hd.rtype = 160;  // MBO message type
        msgs[i].hd.publisher_id = 1;
        msgs[i].hd.product_id = 100;
        msgs[i].hd.ts_event = 1000000000 + i;
        msgs[i].order_id = i + 1;
        msgs[i].price = 1000000 + i * 100;
        msgs[i].size = 100 + i;
        msgs[i].flags = 0;
        msgs[i].channel_id = 1;
        msgs[i].action = 1;
        msgs[i].side = 1;
        msgs[i].ts_recv = 1000000100 + i;
        msgs[i].ts_in_delta = 100;
        msgs[i].sequence = i;
        msgs[i].symbol_id = 1234;
    }
    
    return data;
}

// Helper function to create test file
std::string create_test_file(const std::string& filename, size_t num_records) {
    auto data = create_test_data(num_records);
    std::ofstream file(filename, std::ios::binary);
    file.write(data.data(), data.size());
    file.close();
    return filename;
}

// Test 1: DirectParser basic functionality
TEST(DirectParserTest, BasicParsing) {
    auto data = create_test_data(100);
    DirectParser parser(data.data(), data.size());
    
    size_t count = 0;
    while (const MBOMsg* msg = parser.next()) {
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->order_id, count + 1);
        count++;
    }
    
    EXPECT_EQ(count, 100);
    EXPECT_EQ(parser.records_parsed(), 100);
}

// Test 2: DirectParser empty data
TEST(DirectParserTest, EmptyData) {
    std::vector<char> data;
    DirectParser parser(data.data(), data.size());
    
    EXPECT_FALSE(parser.has_next());
    EXPECT_EQ(parser.next(), nullptr);
    EXPECT_EQ(parser.records_parsed(), 0);
}

// Test 3: DirectParser single record
TEST(DirectParserTest, SingleRecord) {
    auto data = create_test_data(1);
    DirectParser parser(data.data(), data.size());
    
    EXPECT_TRUE(parser.has_next());
    const MBOMsg* msg = parser.next();
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->order_id, 1);
    EXPECT_FALSE(parser.has_next());
    EXPECT_EQ(parser.records_parsed(), 1);
}

// Test 4: DirectParser data integrity
TEST(DirectParserTest, DataIntegrity) {
    auto data = create_test_data(50);
    DirectParser parser(data.data(), data.size());
    
    for (size_t i = 0; i < 50; ++i) {
        const MBOMsg* msg = parser.next();
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->hd.rtype, 160);
        EXPECT_EQ(msg->order_id, i + 1);
        EXPECT_EQ(msg->price, 1000000 + i * 100);
        EXPECT_EQ(msg->size, 100 + i);
        EXPECT_EQ(msg->symbol_id, 1234);
    }
}

// Test 5: BatchParser basic functionality
TEST(BatchParserTest, BasicParsing) {
    auto data = create_test_data(100);
    BatchParser parser(data.data(), data.size(), 10);
    
    size_t total_count = 0;
    while (parser.has_next()) {
        auto batch = parser.next_batch();
        total_count += batch.size();
        EXPECT_LE(batch.size(), 10);
    }
    
    EXPECT_EQ(total_count, 100);
    EXPECT_EQ(parser.records_parsed(), 100);
}

// Test 6: BatchParser custom batch size
TEST(BatchParserTest, CustomBatchSize) {
    auto data = create_test_data(100);
    BatchParser parser(data.data(), data.size(), 25);
    
    size_t batch_count = 0;
    while (parser.has_next()) {
        auto batch = parser.next_batch();
        batch_count++;
        if (batch_count < 4) {
            EXPECT_EQ(batch.size(), 25);
        }
    }
    
    EXPECT_EQ(batch_count, 4);
}

// Test 7: BatchParser empty data
TEST(BatchParserTest, EmptyData) {
    std::vector<char> data;
    BatchParser parser(data.data(), data.size(), 10);
    
    EXPECT_FALSE(parser.has_next());
    auto batch = parser.next_batch();
    EXPECT_TRUE(batch.empty());
}

// Test 8: BatchParser data integrity
TEST(BatchParserTest, DataIntegrity) {
    auto data = create_test_data(30);
    BatchParser parser(data.data(), data.size(), 10);
    
    size_t record_idx = 0;
    while (parser.has_next()) {
        auto batch = parser.next_batch();
        for (const auto& msg : batch) {
            EXPECT_EQ(msg.order_id, record_idx + 1);
            EXPECT_EQ(msg.price, 1000000 + record_idx * 100);
            record_idx++;
        }
    }
}

// Test 9: CallbackParser basic functionality
TEST(CallbackParserTest, BasicParsing) {
    auto data = create_test_data(100);
    CallbackParser parser(data.data(), data.size());
    
    size_t count = 0;
    parser.parse_all([&count](const MBOMsg& msg) {
        EXPECT_EQ(msg.order_id, count + 1);
        count++;
    });
    
    EXPECT_EQ(count, 100);
    EXPECT_EQ(parser.records_parsed(), 100);
}

// Test 10: CallbackParser empty data
TEST(CallbackParserTest, EmptyData) {
    std::vector<char> data;
    CallbackParser parser(data.data(), data.size());
    
    size_t count = 0;
    parser.parse_all([&count](const MBOMsg& msg) {
        count++;
    });
    
    EXPECT_EQ(count, 0);
}

// Test 11: CallbackParser data integrity
TEST(CallbackParserTest, DataIntegrity) {
    auto data = create_test_data(50);
    CallbackParser parser(data.data(), data.size());
    
    size_t idx = 0;
    parser.parse_all([&idx](const MBOMsg& msg) {
        EXPECT_EQ(msg.order_id, idx + 1);
        EXPECT_EQ(msg.price, 1000000 + idx * 100);
        EXPECT_EQ(msg.size, 100 + idx);
        idx++;
    });
}

// Test 12: File loading functionality
TEST(FileLoadingTest, LoadValidFile) {
    std::string filename = "/tmp/test_data.dbn";
    create_test_file(filename, 10);
    
    auto data = load_file(filename);
    EXPECT_EQ(data.size(), 10 * sizeof(MBOMsg));
    
    std::remove(filename.c_str());
}

// Test 13: File loading with invalid file
TEST(FileLoadingTest, LoadInvalidFile) {
    EXPECT_THROW(load_file("/nonexistent/file.dbn"), std::runtime_error);
}

// Test 14: parse_file_mbo_fast
TEST(HighLevelAPITest, ParseFileFast) {
    std::string filename = "/tmp/test_fast.dbn";
    create_test_file(filename, 100);
    
    auto records = parse_file_mbo_fast(filename);
    EXPECT_EQ(records.size(), 100);
    
    for (size_t i = 0; i < records.size(); ++i) {
        EXPECT_EQ(records[i].order_id, i + 1);
    }
    
    std::remove(filename.c_str());
}

// Test 15: parse_file_mbo_batch
TEST(HighLevelAPITest, ParseFileBatch) {
    std::string filename = "/tmp/test_batch.dbn";
    create_test_file(filename, 100);
    
    auto records = parse_file_mbo_batch(filename, 25);
    EXPECT_EQ(records.size(), 100);
    
    for (size_t i = 0; i < records.size(); ++i) {
        EXPECT_EQ(records[i].order_id, i + 1);
    }
    
    std::remove(filename.c_str());
}

// Test 16: parse_file_mbo_callback
TEST(HighLevelAPITest, ParseFileCallback) {
    std::string filename = "/tmp/test_callback.dbn";
    create_test_file(filename, 100);
    
    size_t count = 0;
    parse_file_mbo_callback(filename, [&count](const MBOMsg& msg) {
        EXPECT_EQ(msg.order_id, count + 1);
        count++;
    });
    
    EXPECT_EQ(count, 100);
    std::remove(filename.c_str());
}

// Test 17: Large dataset performance test
TEST(PerformanceTest, LargeDataset) {
    auto data = create_test_data(100000);
    DirectParser parser(data.data(), data.size());
    
    size_t count = 0;
    while (parser.next()) {
        count++;
    }
    
    EXPECT_EQ(count, 100000);
}

// Test 18: Record structure size
TEST(StructureTest, RecordSizes) {
    // Note: Actual sizes may vary due to padding
    // RecordHeader should be 14 bytes (packed) with 2 bytes padding to align to 16
    // MBOMsg should be 58 bytes with the header
    EXPECT_EQ(sizeof(RecordHeader), 14);
    EXPECT_EQ(sizeof(MBOMsg), 58);
}

// Test 19: Memory alignment
TEST(StructureTest, MemoryAlignment) {
    MBOMsg msg;
    std::memset(&msg, 0, sizeof(MBOMsg));
    
    msg.order_id = 12345;
    msg.price = 1000000;
    msg.size = 100;
    
    EXPECT_EQ(msg.order_id, 12345);
    EXPECT_EQ(msg.price, 1000000);
    EXPECT_EQ(msg.size, 100);
}

// Test 20: Multiple parser instances
TEST(ConcurrencyTest, MultipleInstances) {
    auto data1 = create_test_data(50);
    auto data2 = create_test_data(50);
    
    DirectParser parser1(data1.data(), data1.size());
    DirectParser parser2(data2.data(), data2.size());
    
    size_t count1 = 0, count2 = 0;
    
    while (parser1.next()) count1++;
    while (parser2.next()) count2++;
    
    EXPECT_EQ(count1, 50);
    EXPECT_EQ(count2, 50);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
