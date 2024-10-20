#include <iostream>
#include <fstream>
#include <vector>
#include "cache.h"

using namespace std;

// Function to calculate the base-2 logarithm of an integer
int log2(int base) {
    int count = 0;
    while (base > 1) {
        base /= 2; // Divide base by 2 in each iteration
        count++;   // Increment count for each division
    }
    return count;
}

// Simulates a direct mapped cache
void directMapped(ofstream& fout, vector<trace> traces) {
    // Array of cache sizes to simulate with
    const int CACHE_SIZES[4] = {1024, 4096, 16384, 32768};
    const int LINE_SIZE = 32; // Line size of cache

    // Loop through each cache size to simulate
    for (int i = 0; i < sizeof(CACHE_SIZES) / sizeof(CACHE_SIZES[0]); i++) {
        // Create cache table for current size
        cache table[CACHE_SIZES[i] / LINE_SIZE];
        int count = 0; // Count for cache hits

        // Iterate over each trace
        for (int j = 0; j < traces.size(); j++) {
            // Calculate index based on address
            int index = (traces[j].address >> log2(LINE_SIZE)) & ((1 << log2(CACHE_SIZES[i] / LINE_SIZE)) - 1);

            // Check if cache hit or miss
            if (table[index].isValid && table[index].tag == traces[j].address >> log2(CACHE_SIZES[i])) count++;
            else {
                // Update cache line on miss
                table[index].isValid = true;
                table[index].tag = traces[j].address >> log2(CACHE_SIZES[i]);
            }
        }
        
        // Output the hit rate for the current cache size
        fout << count << ',' << traces.size() << "; ";
    }
    
    fout << endl;
}

// Simulates a set associative cache
void setAssociative(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Total size of cache
    const int LINE_SIZE = 32;     // Size of each line/block in the cache

    // Iterate through different set sizes
    for (int i = 2; i <= 16; i *= 2) {
        // Create a 2D array for cache simulation: rows for sets, columns for ways
        cache table[CACHE_SIZE / (LINE_SIZE * i)][i];
        int count = 0; // Cache hits counter
        int lru[CACHE_SIZE / (LINE_SIZE * i)][i]; // LRU tracking array
        
        // Initialize LRU information
        for (int j = 0; j < CACHE_SIZE / (LINE_SIZE * i); j++) {
            for (int k = 0; k < i; k++) lru[j][k] = -1;
        }
        
        // Process each trace
        for (int j = 0; j < traces.size(); j++) {
            bool isFound = false;
            // Calculate index based on cache formula
            int index = (traces[j].address >> log2(LINE_SIZE)) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
            
            // Search for the trace in the set
            for (int k = 0; !isFound && k < i; k++) {
                if (table[index][k].isValid && table[index][k].tag == traces[j].address >> log2(CACHE_SIZE / i)) {
                    isFound = true;
                    count++; // Increment hit count
                    lru[index][k] = j; // Update LRU
                }
            }
            
            // Handle cache miss
            if (!isFound) {
                int lruIndex = 0; // LRU index
                // Find the least recently used line
                for (int k = 1; k < i; k++) {
                    if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                }
                
                // Update the LRU line with the new trace
                table[index][lruIndex].isValid = true;
                table[index][lruIndex].tag = traces[j].address >> log2(CACHE_SIZE / i);
                
                lru[index][lruIndex] = j; // Update LRU index
            }
        }
        
        // Output the hit rate for the current configuration
        fout << count << ',' << traces.size() << "; ";
    }
    
    fout << endl;
}

// Simulates a fully associative cache using LRU replacement policy
void fullyAssociativeLru(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Cache size
    const int LINE_SIZE = 32;     // Line size
    
    cache table[CACHE_SIZE / LINE_SIZE]; // Cache table
    int count = 0; // Hit counter
    int lru[CACHE_SIZE / LINE_SIZE]; // LRU array
    
    // Initialize LRU indices
    for (int i = 0; i < CACHE_SIZE / LINE_SIZE; i++) lru[i] = -1;
    
    // Process each trace
    for (int i = 0; i < traces.size(); i++) {
        bool isFound = false;
        
        // Search for the trace in the cache
        for (int j = 0; !isFound && j < CACHE_SIZE / LINE_SIZE; j++) {
            if (table[j].isValid && table[j].tag == traces[i].address >> log2(LINE_SIZE)) {
                isFound = true;
                count++; // Increment hit counter
                lru[j] = i; // Update LRU
            }
        }
        
        // Handle cache miss
        if (!isFound) {
            int lruIndex = 0; // LRU index
            
            // Find the least recently used line
            for (int j = 1; j < CACHE_SIZE / LINE_SIZE; j++) {
                if (lru[j] < lru[lruIndex]) lruIndex = j;
            }
            
            // Update the LRU line with the new trace
            table[lruIndex].isValid = true;
            table[lruIndex].tag = traces[i].address >> log2(LINE_SIZE);
            
            lru[lruIndex] = i; // Update LRU index
        }
    }
    
    // Output the hit rate
    fout << count << ',' << traces.size() << "; " << endl;
}

// Simulates a fully associative cache using a Hot-Cold replacement policy
void fullyAssociativeHotCold(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Cache size
    const int LINE_SIZE = 32;     // Line size
    bool lru[(CACHE_SIZE / LINE_SIZE) - 1]; // Array to manage the Hot-Cold LRU status
    cache table[CACHE_SIZE / LINE_SIZE]; // Cache table
    int count = 0; // Hit counter

    // Initialize the Hot-Cold LRU array
    for (int i = 0; i < (CACHE_SIZE / LINE_SIZE) - 1; i++) lru[i] = false;

    // Process each trace
    for (int i = 0; i < traces.size(); i++) {
        bool isFound = false;
        
        // Search for the trace in the cache
        for (int j = 0; !isFound && j < (CACHE_SIZE / LINE_SIZE); j++) {
            if (table[j].isValid && table[j].tag == traces[i].address >> log2(LINE_SIZE)) {
                isFound = true;
                count++; // Increment hit counter
                
                // Update LRU path starting from the corresponding leaf to the root
                for (int k = j + (CACHE_SIZE / LINE_SIZE) - 1; k > 0; k = (k - 1) / 2) {
                    if (k % 2 == 0) lru[(k - 1)/2] = true;
					else lru[(k - 1)/2] = false;
                }
            }
        }
        
        // Handle cache miss
        if (!isFound) {
            int j = 0;
            
            // Traverse the Hot-Cold LRU tree to find the coldest element
            while (j < (CACHE_SIZE / LINE_SIZE) - 1) {
                if (!lru[j]) {
                    lru[j] = true;
                    j = (j * 2) + 2; // Move to the right child
                } 
                else {
                    lru[j] = false;
                    j = (j * 2) + 1; // Move to the left child
                }
            }
            
            // Update the coldest element with the new trace
            table[j - (CACHE_SIZE / LINE_SIZE) + 1].isValid = true;
			table[j - (CACHE_SIZE / LINE_SIZE) + 1].tag = traces[i].address >> log2(LINE_SIZE);
        }
    }
    
    // Output the hit rate
    fout << count << ',' << traces.size() << "; " << endl;
}

// Wrapper function to simulate both LRU and Hot-Cold fully associative cache behaviors
void fullyAssociative(ofstream& fout, vector<trace> traces) {
    fullyAssociativeLru(fout, traces);
    fullyAssociativeHotCold(fout, traces);
}

// Simulates set associative cache with no allocation on write miss
void setAssociativeNoAllocationWriteMiss(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Cache size
    const int LINE_SIZE = 32;     // Line size
    
    // Iterate through different set sizes
    for (int i = 2; i <= 16; i *= 2) {
        cache table[CACHE_SIZE / (LINE_SIZE * i)][i]; // 2D cache table
        int count = 0; // Hit counter
        int lru[CACHE_SIZE / (LINE_SIZE * i)][i]; // LRU tracking array
        
        // Initialize LRU information
        for (int j = 0; j < CACHE_SIZE / (LINE_SIZE * i); j++) {
            for (int k = 0; k < i; k++) lru[j][k] = -1;
        }
        
        // Process each trace
        for (int j = 0; j < traces.size(); j++) {
            bool isFound = false;
            int index = (traces[j].address >> log2(LINE_SIZE)) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
            
            // Search for the trace in the set
            for (int k = 0; !isFound && k < i; k++) {
                if (table[index][k].isValid && table[index][k].tag == traces[j].address >> log2(CACHE_SIZE / i)) {
                    isFound = true;
                    count++; // Increment hit counter
                    lru[index][k] = j; // Update LRU
                }
            }
            
            // Handle cache miss differently for store ('S') operations
            if (!isFound && traces[j].type != 'S') {
                int lruIndex = 0;
                // Find the least recently used line
                for (int k = 1; k < i; k++) {
                    if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                }
                
                // Update the LRU line with the new trace if not a store operation
                table[index][lruIndex].isValid = true;
                table[index][lruIndex].tag = traces[j].address >> log2(CACHE_SIZE / i);
                
                lru[index][lruIndex] = j; // Update LRU index
            }
        }
        
        // Output the hit rate for the current configuration
        fout << count << ',' << traces.size() << "; ";
    }
    
    fout << endl;
}

// Simulates set associative cache with next-line prefetching
void setAssociativeNextLinePrefetching(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Total cache size
    const int LINE_SIZE = 32;     // Size of each cache line/block

    // Iterate through different set sizes
    for (int i = 2; i <= 16; i *= 2) {
        cache table[CACHE_SIZE / (LINE_SIZE * i)][i]; // Cache table with multiple sets and ways
        int count = 0; // Hit counter
        int lru[CACHE_SIZE / (LINE_SIZE * i)][i]; // LRU tracking array
        
        // Initialize LRU information
        for (int j = 0; j < CACHE_SIZE / (LINE_SIZE * i); j++) {
            for (int k = 0; k < i; k++) lru[j][k] = -1;
        }
        
        // Process each trace
        for (int j = 0; j < traces.size(); j++) {
            bool isFound = false;
            // Calculate index for current trace
            int index = (traces[j].address >> log2(LINE_SIZE)) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
            
            // Search for the current trace in cache
            for (int k = 0; !isFound && k < i; k++) {
                if (table[index][k].isValid && table[index][k].tag == traces[j].address >> log2(CACHE_SIZE / i)) {
                    isFound = true;
                    count++; // Increment hit counter
                    lru[index][k] = j * 2; // Update LRU, multiplying by 2 to differentiate from prefetch updates
                }
            }
            
            // If miss, find the LRU line to replace
            if (!isFound) {
                int lruIndex = 0;
                for (int k = 1; k < i; k++) {
                    if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                }
                
                // Update the cache line
                table[index][lruIndex].isValid = true;
                table[index][lruIndex].tag = traces[j].address >> log2(CACHE_SIZE / i);
                lru[index][lruIndex] = j * 2; // Set LRU
            }

            // Prefetch the next line
            isFound = false;
            index = ((traces[j].address >> log2(LINE_SIZE)) + 1) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
            for (int k = 0; !isFound && k < i; k++) {
                if (table[index][k].isValid && table[index][k].tag == (traces[j].address + LINE_SIZE) >> log2(CACHE_SIZE / i)) {
                    isFound = true;
                    lru[index][k] = (j * 2) + 1; // Update LRU for prefetch
                }
            }

            // If the prefetch line is not in the cache, fetch it
            if (!isFound) {
                int lruIndex = 0;
                for (int k = 1; k < i; k++) {
                    if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                }

                // Update the cache line with prefetched address
                table[index][lruIndex].isValid = true;
                table[index][lruIndex].tag = (traces[j].address + LINE_SIZE) >> log2(CACHE_SIZE / i);
                lru[index][lruIndex] = (j * 2) + 1; // Set LRU
            }
        }
        
        // Output the hit rate
        fout << count << ',' << traces.size() << "; ";
    }
    
    fout << endl;
}

// Simulates prefetching on miss strategy in a set associative cache
void prefetchMiss(ofstream& fout, vector<trace> traces) {
    const int CACHE_SIZE = 16384; // Total cache size
    const int LINE_SIZE = 32;     // Line size

    // Iterate through different set sizes
    for (int i = 2; i <= 16; i *= 2) {
        cache table[CACHE_SIZE / (LINE_SIZE * i)][i]; // Cache table
        int count = 0; // Hit counter
        int lru[CACHE_SIZE / (LINE_SIZE * i)][i]; // LRU tracking
        
        // Initialize LRU information
        for (int j = 0; j < CACHE_SIZE / (LINE_SIZE * i); j++) {
            for (int k = 0; k < i; k++) lru[j][k] = -1;
        }
        
        // Process each trace
        for (int j = 0; j < traces.size(); j++) {
            bool isFound = false;
            // Calculate index for current trace
            int index = (traces[j].address >> log2(LINE_SIZE)) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
            
            // Search for the current trace in cache
            for (int k = 0; !isFound && k < i; k++) {
                if (table[index][k].isValid && table[index][k].tag == traces[j].address >> log2(CACHE_SIZE / i)) {
                    isFound = true;
                    count++; // Increment hit counter
                    lru[index][k] = j * 2; // Update LRU, multiplying by 2 for differentiation
                }
            }

            // If miss, update and prefetch the next line
            if (!isFound) {
                int lruIndex = 0;
                for (int k = 1; k < i; k++) {
                    if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                }

                // Update the cache line with the current trace
                table[index][lruIndex].isValid = true;
                table[index][lruIndex].tag = traces[j].address >> log2(CACHE_SIZE / i);
                lru[index][lruIndex] = j * 2; // Set LRU

                // Calculate index for the next line to prefetch
                isFound = false;
                index = ((traces[j].address >> log2(LINE_SIZE)) + 1) & ((1 << log2(CACHE_SIZE / (LINE_SIZE * i))) - 1);
                for (int k = 0; !isFound && k < i; k++) {
                    if (table[index][k].isValid && table[index][k].tag == (traces[j].address + LINE_SIZE) >> log2(CACHE_SIZE / i)) {
                        isFound = true;
                        lru[index][k] = (j * 2) + 1; // Update LRU
                    }
                }

                // If the prefetch line is not in the cache, fetch it
                if (!isFound) {
                    lruIndex = 0;
                    for (int k = 1; k < i; k++) {
                        if (lru[index][k] < lru[index][lruIndex]) lruIndex = k;
                    }

                    // Update the cache line with the prefetched address
                    table[index][lruIndex].isValid = true;
                    table[index][lruIndex].tag = (traces[j].address + LINE_SIZE) >> log2(CACHE_SIZE / i);
                    lru[index][lruIndex] = (j * 2) + 1; // Set LRU
                }
            }
        }
        
        // Output the hit rate
        fout << count << ',' << traces.size() << "; ";
    }
    
    fout << endl;
}