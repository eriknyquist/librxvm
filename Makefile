REGEXC_NAME := regexc
REGEXC_C_SRCS := $(wildcard src/*.c)
REGEXC_OBJS := ${REGEXC_C_SRCS:.c=.o}
REGEXC_INCLUDE_DIR := include
CFLAGS := -Wall -I$(REGEXC_INCLUDE_DIR)

$(REGEXC_NAME): $(REGEXC_OBJS)
	$(CC) $(REGEXC_OBJS) -o $(REGEXC_NAME) -I$(REGEXC_INCLUDE_DIR)

clean:
	@- $(RM) $(REGEXC_NAME)
	@- $(RM) $(REGEXC_OBJS)

distclean: clean
