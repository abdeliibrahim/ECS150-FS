# Target library
lib := libfs.a

all: $(lib)

CC:= gcc
CFLAGS	:= -Wall -Werror





# Generic rule for compiling objects
%.o: %.c
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<
	
## TODO: Phase 1
