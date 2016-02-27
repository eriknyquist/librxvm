TESTS := regexvm_test
TESTAPP_NAME := regexvm
TESTAPP_SRCS := tests/testapp/test_main.c
TESTAPP_OBJS := ${TESTAPP_SRCS:.c=.o}
RVM_C_SRCS := $(wildcard src/*.c)
TEST_SRCS := $(wildcard tests/src/*.c)
RVM_OBJS := ${RVM_C_SRCS:.c=.o}
TEST_OBJS := ${TEST_SRCS:.c=.o}
RVM_INC := include
TEST_INC := tests/include
MEMCHECK := tests/scripts/memcheck.sh

MAX_CHARC_LEN := 512
MAX_NEST_PARENS := 512

OPTS := \
MAX_CHARC_LEN=$(MAX_CHARC_LEN) \
MAX_NEST_PARENS=$(MAX_NEST_PARENS)

MACROS := $(addprefix -D , $(OPTS))
CFLAGS := -Wall -pedantic -I$(RVM_INC) $(MACROS)
TESTFLAGS := -Wall -I$(RVM_INC) -I$(TEST_INC) $(MACROS) -O0 -g
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -D DEBUG -g -O0

all: CFLAGS += $(RELEASE_FLAGS)
all: testapp

debug: CFLAGS += $(DEBUG_FLAGS)
debug: testapp

testapp: $(RVM_OBJS) $(TESTAPP_OBJS)
	$(CC) $(RVM_OBJS) $(TESTAPP_OBJS) -o $(TESTAPP_NAME)

test: CFLAGS = $(TESTFLAGS)
test: testapp $(TEST_OBJS)
	$(CC) $(RVM_OBJS) $(TEST_OBJS) -o $(TESTS)
	./$(MEMCHECK) $(TESTAPP_NAME)
	./$(MEMCHECK) $(TESTS)
	./$(TESTS)

clean:
	@- $(RM) $(EXE_NAME) $(TESTS) $(TESTAPP_NAME)
	@- $(RM) $(RVM_OBJS) $(TEST_OBJS) $(TESTAPP_OBJS)

distclean: clean
