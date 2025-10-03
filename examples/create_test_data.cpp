#include "databento/dbn_parser.hpp"
#include <fstream>
#include <iostream>

using namespace databento;

int main() {
    std::vector<MBOMsg> records(1000);
    
    for (size_t i = 0; i < records.size(); ++i) {
        records[i].hd.length = sizeof(MBOMsg);
        records[i].hd.rtype = 160;
        records[i].hd.publisher_id = 1;
        records[i].hd.product_id = 100;
        records[i].hd.ts_event = 1000000000 + i;
        records[i].order_id = i + 1;
        records[i].price = 1000000 + i * 100;
        records[i].size = 100 + i;
        records[i].flags = 0;
        records[i].channel_id = 1;
        records[i].action = 1;
        records[i].side = 1;
        records[i].ts_recv = 1000000100 + i;
        records[i].ts_in_delta = 100;
        records[i].sequence = i;
        records[i].symbol_id = 1234;
    }
    
    std::ofstream file("examples/sample_data.dbn", std::ios::binary);
    file.write(reinterpret_cast<char*>(records.data()), records.size() * sizeof(MBOMsg));
    file.close();
    
    std::cout << "Created sample_data.dbn with " << records.size() << " records" << std::endl;
    return 0;
}
