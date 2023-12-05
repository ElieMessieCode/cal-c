#include "calc/arena.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void calc_arena_init(CalcArena *arena, void *buffer, size_t capacity) {
    if (arena == NULL) {
        return;
    }
    arena->buffer = (uint8_t *)buffer;
    arena->capacity = capacity;
    arena->offset = 0;
}

void *calc_arena_alloc(CalcArena *arena, size_t size, size_t alignment) {
    if (arena == NULL || arena->buffer == NULL || size == 0) {
        return NULL;
    }

    if (alignment == 0) {
        alignment = sizeof(void *) * 2;
    }

    uintptr_t current = (uintptr_t)(arena->buffer + arena->offset);
    uintptr_t mask = (uintptr_t)(alignment - 1);
    uintptr_t aligned = (current + mask) & ~mask;
    size_t new_offset = (size_t)(aligned - (uintptr_t)arena->buffer) + size;

    if (new_offset > arena->capacity) {
        return NULL;
    }

    arena->offset = new_offset;
    void *ptr = (void *)aligned;
    memset(ptr, 0, size);
    return ptr;
}

char *calc_arena_strndup(CalcArena *arena, const char *str, size_t len) {
    if (arena == NULL || str == NULL) {
        return NULL;
    }
    char *copy = (char *)calc_arena_alloc(arena, len + 1, 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, str, len);
    copy[len] = '\0';
    return copy;
}

void calc_arena_reset(CalcArena *arena) {
    if (arena != NULL) {
        arena->offset = 0;
    }
}

size_t calc_arena_used(const CalcArena *arena) {
    return arena != NULL ? arena->offset : 0;
}

size_t calc_arena_available(const CalcArena *arena) {
    if (arena == NULL || arena->offset >= arena->capacity) {
        return 0;
    }
    return arena->capacity - arena->offset;
}
