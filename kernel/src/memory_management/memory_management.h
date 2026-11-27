#ifndef _MEMORY_MANAGEMENT

#define _MEMORY_MANAGEMENT

#include "../../limine-protocol/include/limine.h"

#define MAXIMUM_MEMORY_ALLOCATION 1048576

typedef struct memory_block {
    void *block_begin;
    int size_of_block;
} memory_block;

void *get_end_of_memory_block (memory_block *memory_block);

bool is_block_a_in_block_b (memory_block *memory_block_a, memory_block *memory_block_b);
bool is_overlap_between_memory_blocks (memory_block *memory_block_a, memory_block *memory_block_b);

void copy_memory_block (memory_block *source, memory_block *destination);

class memory_management {
    private:
        memory_block memory_block_information[MAXIMUM_MEMORY_ALLOCATION];

        int number_of_allocations = 0;

        limine_memmap_response memory_map;
        limine_memmap_request memory_map_request;

        bool is_max_allocation_reached ();

        bool is_memory_block_free (memory_block *memory_block);
        memory_block *get_collision_block (memory_block *memory_block);

        bool is_memory_entry_useable (int index);
        int get_size_of_memory_entry (int index);
        void *get_free_spot_in_memory_entry (limine_memmap_entry *entry, int size);

        void generate_memory_map_request();
        void read_memory_map_request();

        void *find_free_memory_block(unsigned int size);
        void mark_block_as_allocated (void *block_begin, unsigned int size);

    public:
        memory_management ();

        void *malloc (unsigned int requested_block_size);

        void free (void *destination, int size);
};

memory_management memory_management_unit = memory_management();

void *malloc (unsigned int requested_block_size) {
    return memory_management_unit.malloc(requested_block_size);
}

void free (void *destination, int size) {
    memory_management_unit.free(destination, size);
}

#endif
