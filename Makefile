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

MAX_CHARC_LEN := 512
MAX_NEST_PARENS := 512

OPTS := \
MAX_CHARC_LEN=$(MAX_CHARC_LEN) \
MAX_NEST_PARENS=$(MAX_NEST_PARENS)

MCHK_REGEX := \
"ww|(xx)aa(yy)?(bb*[abC-Z]|\\.|\\*|(.\\\\cc+(dd?[+*.?])*(ab*)?)+)?"
MCHK_PATTERN := "xxaayy@\\ccccccccccccccccccccccdd?d?dd?d?d*d+d?ab"
MCHK_ARGS := $(MCHK_REGEX) $(MCHK_PATTERN)

VALGRIND_BIN := valgrind
VALGRIND_ARGS := \
-v --leak-check=full --show-leak-kinds=all \
--track-origins=yes --leak-check-heuristics=all

MACROS := $(addprefix -D , $(OPTS))
CFLAGS := -Wall -pedantic -I$(RVM_INC) $(MACROS)
TESTFLAGS := -Wall -I$(RVM_INC) -I$(TEST_INC) $(MACROS) -O3
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -D DEBUG -g -O0

all: CFLAGS += $(RELEASE_FLAGS)
all: testapp

debug: CFLAGS += $(DEBUG_FLAGS)
debug: testapp

testapp: $(RVM_OBJS) $(TESTAPP_OBJS)
	$(CC) $(RVM_OBJS) $(TESTAPP_OBJS) -o $(TESTAPP_NAME)

test: CFLAGS = $(TESTFLAGS)
test: $(RVM_OBJS) $(TEST_OBJS)
	$(CC) $(RVM_OBJS) $(TEST_OBJS) -o $(TESTS)
	@- ./$(TESTS)

memcheck: debug
	$(VALGRIND_BIN) $(VALGRIND_ARGS) ./$(TESTAPP_NAME) $(MCHK_ARGS)

clean:
	@- $(RM) $(EXE_NAME)
	@- $(RM) $(RVM_OBJS)
	@- $(RM) $(TEST_OBJS)
	@- $(RM) $(TESTAPP_OBJS)

Gdistclean: clean
