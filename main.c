#include <stdio.h>
#include <stdlib.h>

#define ALLOCATOR_IMPLEMENTATION
#include "Allocator.h"

int main(void)
{
    struct arena_allocator_t a = stack_arena(sizeof(int));
    int* ptr = arena_allocator_alloc(&a, sizeof(int));
    *ptr = 42;
    printf("%p\n", ptr);
    printf("%d\n", *ptr);

    int *null = arena_allocator_alloc(&a, sizeof(int));
    printf("%p\n", null);
    return 0;
}
