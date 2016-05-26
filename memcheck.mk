TESTS := regexvm_test

RVM_C_SRCS := $(wildcard src/*.c)
TEST_SRCS := $(wildcard tests/src/test_*.c)
RVM_OBJS := ${RVM_C_SRCS:.c=.o}
TEST_OBJS := ${TEST_SRCS:.c=.o}

RVM_INC := src
TEST_INC := tests/src
MEMCHECK := tests/scripts/memcheck.sh

CFLAGS := -Wall -Wno-trigraphs -I$(RVM_INC) -I$(TEST_INC) -g -O0


all: $(TESTS)
	./$(MEMCHECK) $(TESTS)

$(TESTS): $(RVM_OBJS) $(TEST_OBJS)
	$(CC) $(RVM_OBJS) $(TEST_OBJS) -o $@

clean:
	@- $(RM) $(TESTS) $(RVM_OBJS) $(TEST_OBJS)

distclean: clean
