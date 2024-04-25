# Targets & general dependencies
PROGRAM = cachex
HEADERS = main.h cache.h 
OBJS = main.o cache.o 
ADD_OBJS = 

# compilers, linkers, utilities, and flags
CC = gcc
CFLAGS = -Wall -g
COMPILE = $(CC) $(CFLAGS)
LINK = $(CC) $(CFLAGS) -o $@ 

# implicit rule to build .o from .c files
%.o: %.c $(HEADERS)
	$(COMPILE) -c -o $@ $<


# explicit rules
all: $(PROGRAM) 

$(PROGRAM): $(OBJS) $(ADD_OBJS)
	$(LINK) $(OBJS) $(ADD_OBJS)

clean:
	rm -f *.o $(PROGRAM)
