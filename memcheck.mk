TESTS := rxvm_test

RXVM_C_SRCS := $(wildcard librxvm/*.c)
TEST_SRCS := $(wildcard tests/src/test_*.c) tests/src/randexp.c
RXVM_OBJS := ${RXVM_C_SRCS:.c=.o}
TEST_OBJS := ${TEST_SRCS:.c=.o}

RXVM_INC := librxvm
TEST_INC := tests/src
MEMCHECK := tests/scripts/memcheck.sh

CFLAGS := -Wall -Wno-trigraphs -I$(RXVM_INC)

testobjs: CFLAGS += -I$(TEST_INC)
testobjs: $(TEST_OBJS)

$(TESTS): $(RXVM_OBJS) testobjs
	$(CC) $(RXVM_OBJS) $(TEST_OBJS) -o $@

debug: CFLAGS += -g -O0
debug: $(TESTS)

release: CFLAGS += -O3
release: $(TESTS)

all: debug
	./$(MEMCHECK) $(TESTS)

clean:
	@- $(RM) $(TESTS) $(RXVM_OBJS) $(TEST_OBJS)

distclean: clean
