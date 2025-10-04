// Parallel Multi-Threaded File Loader
// Uses 4+ threads to fully utilize NVMe bandwidth
// Target: 2000+ MB/s (4x improvement from 500 MB/s)

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdint>
#include <stdexcept>

// Parallel load using multiple threads
class ParallelLoader {
public:
    std::vector<uint8_t> load(const char* filepath, size_t num_threads = 4) {
        // Get file size
        int fd = open(filepath, O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file");
        }
        
        struct stat sb;
        fstat(fd, &sb);
        size_t file_size = sb.st_size;
        close(fd);
        
        // Allocate buffer
        std::vector<uint8_t> buffer(file_size);
        
        // Calculate chunk size per thread
        size_t chunk_size = file_size / num_threads;
        
        // Launch threads
        std::vector<std::thread> threads;
        
        for (size_t t = 0; t < num_threads; ++t) {
            size_t offset = t * chunk_size;
            size_t size = (t == num_threads - 1) ? (file_size - offset) : chunk_size;
            
            threads.emplace_back([filepath, offset, size, &buffer]() {
                int fd = open(filepath, O_RDONLY);
                if (fd < 0) return;
                
                // Seek to offset
                lseek(fd, offset, SEEK_SET);
                
                // Read chunk
                size_t total_read = 0;
                while (total_read < size) {
                    ssize_t n = read(fd, buffer.data() + offset + total_read, size - total_read);
                    if (n <= 0) break;
                    total_read += n;
                }
                
                close(fd);
            });
        }
        
        // Wait for all threads
        for (auto& t : threads) {
            t.join();
        }
        
        return buffer;
    }
};

int main() {
    const char* filepath = "/home/nvidia/github-v1/tradning/rithmic_v1/dbn/data.dbn";
    
    std::cout << "\n════════════════════════════════════════════════════════════\n";
    std::cout << "Parallel Multi-Threaded Loader\n";
    std::cout << "File: " << filepath << "\n";
    std::cout << "════════════════════════════════════════════════════════════\n\n";
    
    // Test different thread counts
    for (int num_threads : {1, 2, 4, 8, 16}) {
        std::cout << "Testing with " << num_threads << " threads...\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        ParallelLoader loader;
        auto buffer = loader.load(filepath, num_threads);
        
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        
        double mb = buffer.size() / (1024.0 * 1024.0);
        double speed_mbs = mb / elapsed;
        
        std::cout << "  Time: " << std::fixed << std::setprecision(2) << elapsed << "s | ";
        std::cout << "Speed: " << std::fixed << std::setprecision(0) << speed_mbs << " MB/s\n\n";
    }
    
    std::cout << "════════════════════════════════════════════════════════════\n";
    
    return 0;
}

