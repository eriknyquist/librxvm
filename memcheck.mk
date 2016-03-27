TESTS := regexvm_test

RVM_C_SRCS := $(wildcard src/*.c)
TEST_SRCS := $(wildcard tests/src/*.c)
RVM_OBJS := ${RVM_C_SRCS:.c=.o}
TEST_OBJS := ${TEST_SRCS:.c=.o}

RVM_INC := src
TEST_INC := tests/src
MEMCHECK := tests/scripts/memcheck.sh

CFLAGS := -Wall -I$(RVM_INC) -I$(TEST_INC) -g -O0


all: $(RVM_OBJS) $(TEST_OBJS)
	$(CC) $(RVM_OBJS) $(TEST_OBJS) -o $(TESTS)
	./$(MEMCHECK) $(TESTS)

clean:
	@- $(RM) $(TESTS) $(RVM_OBJS) $(TEST_OBJS)

distclean: clean
