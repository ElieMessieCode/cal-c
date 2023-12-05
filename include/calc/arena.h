#ifndef CALC_ARENA_H
#define CALC_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CalcArena {
    uint8_t *buffer;
    size_t   capacity;
    size_t   offset;
} CalcArena;

void calc_arena_init(CalcArena *arena, void *buffer, size_t capacity);
void *calc_arena_alloc(CalcArena *arena, size_t size, size_t alignment);
char *calc_arena_strndup(CalcArena *arena, const char *str, size_t len);
void calc_arena_reset(CalcArena *arena);
size_t calc_arena_used(const CalcArena *arena);
size_t calc_arena_available(const CalcArena *arena);

#ifdef __cplusplus
}
#endif

#endif
