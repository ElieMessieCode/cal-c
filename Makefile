CC ?= gcc
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror -O2 -Iinclude
LDFLAGS ?= -lm

SRCS = src/arena.c src/error.c src/value.c src/vars.c src/lexer.c src/ast.c src/parser.c src/eval.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = $(wildcard tests/test_*.c)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all clean test format

all: bin/calc

bin/calc: app/main.c $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ app/main.c $(OBJS) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		echo "Running $$t..."; \
		$$t || exit 1; \
	done

tests/%: tests/%.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LDFLAGS)

clean:
	rm -f src/*.o bin/calc tests/test_*

format:
	clang-format -i include/calc/*.h src/*.c app/*.c tests/*.c fuzz/*.c
