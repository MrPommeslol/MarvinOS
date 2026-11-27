#include "memory_management.h"
#include "../../limine-protocol/include/limine.h"

void *get_end_of_memory_block (memory_block *memory_block) {
    return (void *)((char *)(memory_block->block_begin) + memory_block->size_of_block);
}

bool is_block_a_in_block_b (
    memory_block *memory_block_a,
    memory_block *memory_block_b
) {
    void *end_of_a = get_end_of_memory_block(memory_block_a);
    bool does_a_end_before_b_begins = end_of_a < memory_block_b->block_begin;
    return does_a_end_before_b_begins;
}

bool is_overlap_between_memory_blocks (
    memory_block *memory_block_a,
    memory_block *memory_block_b
) {
    bool is_block_a_before_block_b = memory_block_a->block_begin < memory_block_b->block_begin;
    if (is_block_a_before_block_b) {
        return is_block_a_in_block_b (memory_block_a, memory_block_b);
    } else {
        return is_block_a_in_block_b (memory_block_b, memory_block_a);
    }
}


void copy_memory_block (memory_block source, memory_block *destination) {
    destination->block_begin = source.block_begin;
    destination->size_of_block = source.size_of_block;
}

bool memory_management::is_max_allocation_reached () {
    return (MAXIMUM_MEMORY_ALLOCATION <= number_of_allocations);
}

bool memory_management::is_memory_block_free (memory_block *memory_block) {
    for (int i=0; i < number_of_allocations; i++) {
        bool do_blocks_overlap = is_overlap_between_memory_blocks (memory_block_information + i, memory_block);
        if (do_blocks_overlap) return false;
    }
    return true;
}

memory_block *memory_management::get_collision_block (memory_block *memory_block) {
    for (int i=0; i < number_of_allocations; i++) {
        bool do_blocks_overlap = is_overlap_between_memory_blocks (memory_block_information + i, memory_block);
        if (do_blocks_overlap) {
            return &memory_block_information[i];
        }
    }
    return nullptr;
}

bool memory_management::is_memory_entry_useable (int index) {
    return memory_map.entries[index]->type == LIMINE_MEMMAP_USABLE;
}

int memory_management::get_size_of_memory_entry (int index) {
    return memory_map.entries[index]->length;
}

void memory_management::generate_memory_map_request () {
    memory_map_request = {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
        .response = nullptr,
    };
}

void *memory_management::get_free_spot_in_memory_entry (limine_memmap_entry *entry, int size) {
    void *begin_of_entry = (void *)(entry->base);
    void *end_of_entry   = (void *)(entry->base + entry->length);

    memory_block testblock = {
        .block_begin = begin_of_entry,
        .size_of_block = size,
    };

    while (testblock.block_begin < end_of_entry) {
        memory_block *conflicting_block = get_collision_block(&testblock);
        if (!conflicting_block) return testblock.block_begin;
        testblock.block_begin = get_end_of_memory_block(conflicting_block);
    }
    return nullptr;
}

void memory_management::read_memory_map_request () {
    limine_memmap_response *ptr_on_response = memory_map_request.response;
    memory_map = *ptr_on_response;
}

void *memory_management::find_free_memory_block(unsigned int requested_size) {
    for (int i = 0; i < memory_map.entry_count; i++) {
        bool is_memory_entry_big_enough = requested_size <= get_size_of_memory_entry(i);
        if (is_memory_entry_useable(i) && is_memory_entry_big_enough) {
            limine_memmap_entry *memory_entry = memory_map.entries[i];
            void *position_for_new_block = get_free_spot_in_memory_entry(memory_entry, requested_size);
            if (position_for_new_block) return position_for_new_block;
        }
    }
    return nullptr;
}

void memory_management::mark_block_as_allocated (void *block_begin, unsigned int requested_size) {
    memory_block *memory_block = &memory_block_information[number_of_allocations];
    memory_block->block_begin = block_begin;
    memory_block->size_of_block = requested_size;
}

memory_management::memory_management () {
    generate_memory_map_request();
    read_memory_map_request();
}

void *memory_management::malloc (unsigned int requested_block_size) {
    if(is_max_allocation_reached()) return nullptr;
    void *block = find_free_memory_block(requested_block_size);
    if (!block) return nullptr;
    mark_block_as_allocated (block, requested_block_size);
    return block;
}

void memory_management::free (void *destination, int size) {

}
