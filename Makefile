RVM_NAME := regexvm
RVM_C_SRCS := $(wildcard src/*.c)
RVM_OBJS := ${RVM_C_SRCS:.c=.o}
RVM_INCLUDE_DIR := include

MAX_CHARC_LEN := 512
MAX_NEST_PARENS := 512

OPTS := \
MAX_CHARC_LEN=$(MAX_CHARC_LEN) \
MAX_NEST_PARENS=$(MAX_NEST_PARENS)

MACROS := $(addprefix -D , $(OPTS))
CFLAGS := -Wall -I$(RVM_INCLUDE_DIR) $(MACROS)
DEBUG_FLAGS := -D DEBUG -g

all: $(RVM_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(RVM_NAME)

$(RVM_NAME): $(RVM_OBJS)
	$(CC) $(RVM_OBJS) -o $(RVM_NAME) -I$(RVM_INCLUDE_DIR)

clean:
	@- $(RM) $(RVM_NAME)
	@- $(RM) $(RVM_OBJS)

distclean: clean
