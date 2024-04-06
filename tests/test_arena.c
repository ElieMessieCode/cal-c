#include "test.h"
#include "calc/arena.h"

TEST_BEGIN(test_arena_init_and_alloc)
    uint8_t buffer[1024];
    CalcArena arena;
    calc_arena_init(&arena, buffer, sizeof(buffer));

    ASSERT_EQ_INT(calc_arena_used(&arena), 0);
    ASSERT_EQ_INT(calc_arena_available(&arena), 1024);

    void *ptr1 = calc_arena_alloc(&arena, 32, 8);
    ASSERT_TRUE(ptr1 != NULL);
    ASSERT_EQ_INT(calc_arena_used(&arena), 32);

    void *ptr2 = calc_arena_alloc(&arena, 64, 16);
    ASSERT_TRUE(ptr2 != NULL);
    ASSERT_TRUE((uintptr_t)ptr2 % 16 == 0);

    char *s = calc_arena_strndup(&arena, "hello world", 5);
    ASSERT_TRUE(s != NULL);
    ASSERT_EQ_STR(s, "hello");

    calc_arena_reset(&arena);
    ASSERT_EQ_INT(calc_arena_used(&arena), 0);
    ASSERT_EQ_INT(calc_arena_available(&arena), 1024);
TEST_END()

TEST_BEGIN(test_arena_out_of_memory)
    uint8_t buffer[64];
    CalcArena arena;
    calc_arena_init(&arena, buffer, sizeof(buffer));

    void *p1 = calc_arena_alloc(&arena, 48, 8);
    ASSERT_TRUE(p1 != NULL);

    void *p2 = calc_arena_alloc(&arena, 32, 8);
    ASSERT_TRUE(p2 == NULL);

    calc_arena_reset(&arena);
    void *p3 = calc_arena_alloc(&arena, 32, 8);
    ASSERT_TRUE(p3 != NULL);
TEST_END()

int main(void) {
    printf("Running test_arena...\n");
    RUN_TEST(test_arena_init_and_alloc);
    RUN_TEST(test_arena_out_of_memory);
    TEST_REPORT();
}
