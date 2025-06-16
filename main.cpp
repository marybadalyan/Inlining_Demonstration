#include <cstdint>
#include <chrono>
#include <iostream>
#include "config.h"
#include <fstream>
#include <string>


namespace Color {
    const char* RESET = "\033[0m";
    const char* BOLD_CYAN = "\033[1;36m";
    const char* BOLD_YELLOW = "\033[1;33m";
    const char* BOLD_GREEN = "\033[1;32m";
    const char* BOLD_MAGENTA = "\033[1;35m";
}


// Optimizartion cam also aid in loop unrolling 
FORCE_INLINE int64_t sum_array(int32_t* a, int size) {
    int64_t sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += a[i];
    }
    return sum;
}

// from optimizing away the calls to sum_array.
int64_t run(int32_t* a, int size) {
    int64_t total_sum = 0;
    for(int j = 0; j < 100; ++j) {
        // Accumulate the result to ensure the call is not dead code.
        total_sum += sum_array(a, size);
    }
    return total_sum;
}

int main(int argc, char* argv[]) {
    int32_t my_array[1024];
    for(int i = 0; i < 1024; ++i) {
        my_array[i] = i % 10;
    }

    if (argc < 3) {
        std::cerr << "Error: Please provide the assembly file path AND optimization level as arguments.\n";
        return 1;
    }
    std::string assemblyFilePath = argv[1];
    std::string optLevel = argv[2];

    int64_t final_result = 0; // Variable to store the result

    auto start = std::chrono::high_resolution_clock::now();
    final_result = run(my_array, 1024); // Store the result
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << Color::BOLD_CYAN << "Assembly Path:      " << assemblyFilePath << Color::RESET << std::endl;
    std::cout << Color::BOLD_MAGENTA << "Optimization Level: " << optLevel <<   Color::RESET << std::endl;
    std::cout << Color::BOLD_YELLOW << "Time Taken:         " << elapsed.count() << " seconds" << Color::RESET <<  std::endl;
    // By printing the result, we guarantee to the compiler that the work was necessary.
    std::cout << Color::BOLD_YELLOW << "Final Result:       " << final_result << Color::RESET << std::endl;


    std::ifstream asmFile(assemblyFilePath);
    if (!asmFile) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    // CORRECTED: The mangled name for `run(int*, int)` on GCC/Clang is _Z3runPii
    #if defined(_MSC_VER)
        // Note: MSVC mangling can be complex. This may need adjustment.
        const std::string mangledRunName = "?run@@YA_JPEAHH@Z";
    #else
        const std::string mangledRunName = "_Z3runPii";   // GCC/Clang for run(int*, int)
    #endif

    // In main(), after defining mangledRunName

    #if defined(_MSC_VER)
        const std::string mangledSumArrayName = "?sum_array@@YA_JPEAHH@Z"; // MSVC name
    #else
        const std::string mangledSumArrayName = "_Z9sum_arrayPii";       // GCC/Clang name
    #endif

   

    std::string line;
    bool inRunFunc = false;
    int count = 0;
    int call_count = 0;
    int compare_count = 0; // Separate counters for clarity
    int jump_count = 0;

    while (std::getline(asmFile, line)) {
        if (!inRunFunc) {
            if (line.find(mangledRunName) != std::string::npos) {
                inRunFunc = true;
                std::cout << "\n--- Assembly for run() ---" << std::endl;
            }
        } else {
            if (line.empty() || line.find("ENDP") != std::string::npos || line.find(".cfi_endproc") != std::string::npos) {
                break;
            }
            if (!line.empty() && (line[0] == '\t' || line[0] == ' ')) {
                std::cout << line << std::endl;
                count++;

                // Count compare instructions
                if (line.find("cmp") != std::string::npos) {
                    compare_count++;
                }
                // Count jump instructions
                if (line.find("\tj") != std::string::npos) { // Check for tab + j
                    jump_count++;
                }

                // Find and categorize call instructions
                if (line.find("call") != std::string::npos) {
                    call_count++;

                    if (line.find(mangledSumArrayName) != std::string::npos) {
                    
                        std::cout << Color::BOLD_CYAN << "\t^---- Call to sum_array" << Color::RESET << std::endl;
                    }
                
                }
            }
        }
    }
    asmFile.close();

    std::cout << "--- End of Assembly ---\n" << std::endl;
    std::cout << Color::BOLD_GREEN << "Assembly Lines:     " << count <<  " instructions" << Color::RESET <<  std::endl;
    std::cout << Color::BOLD_GREEN << "Comparing Instructions: " << compare_count << " instructions" << Color::RESET << std::endl;
    std::cout << Color::BOLD_GREEN << "Function Call Instructions: " << call_count << " instructions" << Color::RESET << std::endl;
    return 0;
}