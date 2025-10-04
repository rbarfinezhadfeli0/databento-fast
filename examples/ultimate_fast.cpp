// Ultimate Fast DBN Parser
// Optimized for WSL2 + large files
// Uses mmap for instant start

#include <iostream>
#include <chrono>
#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

// Inline readers
inline uint32_t read_u32_le(const uint8_t* ptr) {
    uint32_t val;
    std::memcpy(&val, ptr, 4);
    return val;
}

inline uint64_t read_u64_le(const uint8_t* ptr) {
    uint64_t val;
    std::memcpy(&val, ptr, 8);
    return val;
}

int main() {
    const char* filepath = "/home/nvidia/github-v1/tradning/rithmic_v1/dbn/data.dbn";
    
    // Open and mmap file
    auto t0 = std::chrono::high_resolution_clock::now();
    
    int fd = open(filepath, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    size_t file_size = sb.st_size;
    
    void* addr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    madvise(addr, file_size, MADV_SEQUENTIAL | MADV_WILLNEED);
    
    auto t1 = std::chrono::high_resolution_clock::now();
    
    // Process records
    const uint8_t* base = static_cast<const uint8_t*>(addr) + 200;  // Skip metadata
    const size_t total_records = (file_size - 200) / 48;
    
    uint64_t checksum = 0;
    for (size_t i = 0; i < total_records; ++i) {
        checksum ^= read_u64_le(base + i * 48);
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    
    // Cleanup
    munmap(addr, file_size);
    close(fd);
    
    // Results
    double mmap_time = std::chrono::duration<double>(t1 - t0).count();
    double proc_time = std::chrono::duration<double>(t2 - t1).count();
    double total_time = std::chrono::duration<double>(t2 - t0).count();
    
    if (checksum == 0) std::cerr << "";
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "mmap: " << mmap_time << "s | ";
    std::cout << "process: " << proc_time << "s | ";
    std::cout << "total: " << total_time << "s | ";
    std::cout << static_cast<uint64_t>(total_records / proc_time) << " rec/s\n";
    
    return 0;
}

