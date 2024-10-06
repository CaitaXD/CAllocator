#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <assert.h>
#include <stdint.h>

#ifndef ALLOCATOR_API
#define ALLOCATOR_API static inline
#endif

#define ARRAY_LITERAL(size__, ...) (uint8_t[(size__)]){ __VA_ARGS__ }

// Allocator API

struct allocator_t {
    void * (*alloc)(const struct allocator_t *allocator, size_t size);

    void (*dealloc)(const struct allocator_t *allocator, void *ptr);
};

ALLOCATOR_API void *alloc(const struct allocator_t *allocator, size_t size);

ALLOCATOR_API void dealloc(const struct allocator_t *allocator, void *ptr);

// Arena Allocator API

struct arena_allocator_t {
    struct allocator_t allocator;
    uintptr_t min_address;
    uintptr_t max_address;
    uintptr_t current_address;
};

ALLOCATOR_API struct arena_allocator_t arena_allocator_init(void *address, size_t bytes);

ALLOCATOR_API void *arena_allocator_alloc(struct arena_allocator_t *allocator, size_t size);

ALLOCATOR_API void arena_allocator_dealloc(const struct arena_allocator_t *allocator, const void *ptr);

ALLOCATOR_API void arena_allocator_clear(struct arena_allocator_t *allocator);

#define stack_arena(size__) arena_allocator_init(ARRAY_LITERAL(size__), size__)

#ifndef ALLOCATOR_DEFAULT_ALLOC
#   define ALLOCATOR_DEFAULT_ALLOC(SIZE) malloc(SIZE)
#endif

#ifndef ALLOCATOR_DEFAULT_FREE
#   define ALLOCATOR_DEFAULT_FREE(PTR) free(PTR)
#endif

#ifdef ALLOCATOR_IMPLEMENTATION

void *alloc(const struct allocator_t *allocator, const size_t size) {
    if (allocator == NULL || allocator->alloc == NULL) {
        return ALLOCATOR_DEFAULT_ALLOC(size);
    }
    return allocator->alloc(allocator, size);
}

void dealloc(const struct allocator_t *allocator, void *ptr) {
    if (allocator == NULL || allocator->dealloc == NULL) {
        ALLOCATOR_DEFAULT_FREE(ptr);
        return;
    }
    allocator->dealloc(allocator, ptr);
}

// Arena Allocator API

struct arena_allocator_t arena_allocator_init(void *address, const size_t bytes) {
    return (struct arena_allocator_t){
        .allocator = {
            .alloc = (void *) arena_allocator_alloc,
            .dealloc = (void *) arena_allocator_dealloc
        },
        .current_address = (uintptr_t) address,
        .min_address = (uintptr_t) address,
        .max_address = (uintptr_t) address + bytes,
    };
}

void *arena_allocator_alloc(struct arena_allocator_t *allocator, const size_t size) {
    assert(allocator != NULL && "Allocator is not initialized");
    if (allocator->current_address + size > allocator->max_address) {
        return NULL;
    }
    const uintptr_t ptr = allocator->current_address;
    allocator->current_address += size;
    return (void *) ptr;
}

void arena_allocator_dealloc(const struct arena_allocator_t *allocator, const void *ptr) {
    assert(allocator != NULL && "Arena allocator was not initialized");
    const uintptr_t current = allocator->current_address;
    const uintptr_t max = allocator->max_address;
    const uintptr_t address = (uintptr_t) ptr;
    assert(address >= current && address <= max && "Pointer is out of the reserved region");
}

void arena_allocator_clear(struct arena_allocator_t *allocator) {
    allocator->current_address = allocator->min_address;
}

#endif //ALLOCATOR_IMPLEMENTATION
#endif //ALLOCATOR_H
