TESTS := regexvm_test
NAME := regexvm
LIB := lib$(NAME).a
TESTAPP := $(NAME)
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
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -D DEBUG -g -O0
CFLAGS := -Wall $(MACROS) -I$(RVM_INC)
STATIC := -static -L. -l$(NAME)
TESTFLAGS := -I$(TEST_INC)

all: CFLAGS += $(RELEASE_FLAGS)
all: lib

debug: CFLAGS += $(DEBUG_FLAGS)
debug: lib

lib: CFLAGS += -pedantic
lib: $(RVM_OBJS)
	$(AR) rcs $(LIB) $(RVM_OBJS)

testbin: CFLAGS += $(STATIC) $(TESTFLAGS)
testbin: $(TEST_OBJS)
	$(CC) $(TEST_OBJS) $(STATIC) -o $(TESTS)

test: all testbin
	./$(TESTS)

memcheck: CFLAGS += $(TESTFLAGS)
memcheck: $(RVM_OBJS) $(TEST_OBJS) $(TESTAPP_OBJS)
	$(CC) $(RVM_OBJS) $(TESTAPP_OBJS) -o $(TESTAPP)
	$(CC) $(RVM_OBJS) $(TEST_OBJS) -o $(TESTS)
	./$(MEMCHECK) $(TESTAPP)
	./$(MEMCHECK) $(TESTS)

clean:
	@- $(RM) $(EXE_NAME) $(TESTS) $(TESTAPP) $(LIB)
	@- $(RM) $(RVM_OBJS) $(TEST_OBJS) $(TESTAPP_OBJS)

distclean: clean
