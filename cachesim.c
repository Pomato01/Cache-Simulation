
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"

// Statistics to keep track of
counter_t accesses = 0;     // Total number of cache accesses
counter_t hits = 0;         // Total number of cache hits
counter_t misses = 0;       // Total number of cache misses
counter_t writebacks = 0;   // Total number of writebacks

/**
 * Function to perform a very basic log2
 * 
 */
int simple_log_2(int x) {
    int val = 0;
    while (x > 1) {
        x /= 2;
        val++;
    }
    return val; 
}

cache_set_t* cache;     // Data structure for the cache
int block_size;         // Block size
int cache_size;         // Cache size
int ways;               // Ways
int num_sets;           // Number of sets
int num_offset_bits;    // Number of offset bits
int num_index_bits;     // Number of index bits. 

/**
 * Function to intialize your cache simulator with the given cache parameters. 
 * 
 * @param _block_size is the block size in bytes
 * @param _cache_size is the cache size in bytes
 * @param _ways is the associativity
 */
void cachesim_init(int _block_size, int _cache_size, int _ways) {
    // Set cache parameters to global variables
    block_size = _block_size;
    cache_size = _cache_size;
    ways = _ways;


    num_sets = cache_size/(block_size * ways);
    num_offset_bits = simple_log_2(block_size);
    num_index_bits = simple_log_2(num_sets);

    cache = (cache_set_t*) malloc(num_sets * sizeof(cache_set_t));

    for (int i = 0; i < num_sets; i++) {
        cache[i].size = ways;
        cache[i].stack = init_lru_stack(ways);
        cache[i].blocks = (cache_block_t*) malloc(ways * sizeof(cache_block_t));
        for (int j = 0; j < ways; j++) {
            cache[i].blocks[j].valid = 0;
            cache[i].blocks[j].dirty = 0;
        }
    }
}

/**
 * Function to perform a memory access to cache
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param access_type is the type of access - 0 (data read), 1 (data write) or 2 (instruction read)
 */
void cachesim_access(addr_t physical_addr, int access_type) {

    addr_t offset_mask = (1 << num_offset_bits) - 1;
    addr_t index_mask = (1 << (num_offset_bits + num_index_bits)) - 1;

    addr_t offset = physical_addr & offset_mask;
    addr_t index = (physical_addr & index_mask) >> num_offset_bits;
    addr_t tag = physical_addr >> (num_offset_bits + num_index_bits);

    cache_set_t* set = &cache[index];

    accesses++;
    for (int i = 0; i < ways; i++) {
        if (set->blocks[i].valid && set->blocks[i].tag == tag) {
            hits++;
            lru_stack_set_mru(set->stack, i);
            if (access_type == MEMWRITE) {
                set->blocks[i].dirty = 1;
            }
            return;
        }
    }

    misses++;
    int cycled_out = -1;
    for (int i = 0; i < ways; i++) {
        if (!set->blocks[i].valid) {
            cycled_out = i;
            break;
        }
    }
    if (cycled_out == -1) {
        cycled_out = lru_stack_get_lru(set->stack);
    }

    if (set->blocks[cycled_out].valid && set->blocks[cycled_out].dirty) {
        writebacks++;
    }

    set->blocks[cycled_out].tag = tag;
    set->blocks[cycled_out].valid = 1;
    set->blocks[cycled_out].dirty = 0;
    if (access_type == MEMWRITE) {
        set->blocks[cycled_out].dirty = 1;
    }

    lru_stack_set_mru(set->stack, cycled_out);

}

/**
 * free up any dynamically allocated
 */
void cachesim_cleanup() {
    for (int i = 0; i < num_sets; i++) {
        lru_stack_cleanup(cache[i].stack);
        free(cache[i].blocks);
    }
    free(cache);
}

void cachesim_print_stats() {
    printf("%llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);  
}

FILE *open_trace(const char *filename) {
    return fopen(filename, "r");
}

/**
 * Read in next line of the trace
 * 
 */
int next_line(FILE* trace) {
    if (feof(trace) || ferror(trace)) return 0;
    else {
        int t;
        unsigned long long address, instr;
        fscanf(trace, "%d %llx %llx\n", &t, &address, &instr);
        cachesim_access(address, t);
    }
    return 1;
}

/**
 * Main function
 */
int main(int argc, char **argv) {
    FILE *input;

    if (argc != 5) {
        fprintf(stderr, "Usage:\n  %s <trace> <block size(bytes)>"
                        " <cache size(bytes)> <ways>\n", argv[0]);
        return 1;
    }
    
    input = open_trace(argv[1]);
    cachesim_init(atol(argv[2]), atol(argv[3]), atol(argv[4]));
    while (next_line(input));
    cachesim_print_stats();
    cachesim_cleanup();
    fclose(input);
    return 0;
}

