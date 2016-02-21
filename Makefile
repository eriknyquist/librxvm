RVM_NAME := regexvm
RVM_C_SRCS := $(wildcard src/*.c)
RVM_OBJS := ${RVM_C_SRCS:.c=.o}
RVM_INCLUDE_DIR := include

MAX_CHARC_LEN := 512
MAX_NEST_PARENS := 512

OPTS := \
MAX_CHARC_LEN=$(MAX_CHARC_LEN) \
MAX_NEST_PARENS=$(MAX_NEST_PARENS)

MEMCHECK_REGEX := \
"ww|(xx)aa(yy)?(bb*[abC-Z]|\\.|\\*|(.\\\\cc+(dd?[+*.?])*(ab*)?)+)?"
MEMCHECK_PATTERN := "xxaayy@\\ccccccccccccccccccccccdd?d?dd?d?d*d+d?ab"

VALGRIND_BIN := valgrind
VALGRIND_ARGS := \
-v --leak-check=full --show-leak-kinds=all \
--track-origins=yes --leak-check-heuristics=all \
./$(RVM_NAME) $(MEMCHECK_REGEX) $(MEMCHECK_PATTERN)

MACROS := $(addprefix -D , $(OPTS))
CFLAGS := -Wall -pedantic -I$(RVM_INCLUDE_DIR) $(MACROS)
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -D DEBUG -g -O0

all: CFLAGS += $(RELEASE_FLAGS)
all: $(RVM_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(RVM_NAME)

$(RVM_NAME): $(RVM_OBJS)
	$(CC) $(RVM_OBJS) -o $(RVM_NAME) -I$(RVM_INCLUDE_DIR)

memcheck: debug
	$(VALGRIND_BIN) $(VALGRIND_ARGS)

clean:
	@- $(RM) $(RVM_NAME)
	@- $(RM) $(RVM_OBJS)

distclean: clean
