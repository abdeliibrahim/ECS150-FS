# Target library
lib := libfs.a

all: $(lib)
CC:= gcc
CFLAGS	:= -Wall -Werror -g


$(lib): fs.o disk.o
	ar rcs libfs.a fs.o disk.o

# Generic rule for compiling objects
%.o: %.c %h
	@echo "CC	$@"
	$(CC) $(CFLAGS) -c -o $@ $<

# fs.o
%.o: %.c %.h disk.h fs.h
	$(CC) $(Cflags) -c -o $@ $<


clean:
	rm -f  $(lib) libfs.a fs.o disk.o
## TODO: Phase 1
