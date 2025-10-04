# compiler
CC = gcc

# compiler flags
CFLAGS = -Wall -g

# linker flags
LDFLAGS = -lcjson

# object files
OBJS = main.o metadata.o


# target binary
pen: $(OBJS)
	$(CC) $(CFLAGS) -o pen $(OBJS) $(LDFLAGS)

# compile .c files to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# clean up build files
clean:
	rm -f *.o pen
