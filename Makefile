CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lcjson

OBJS = main.o metadata.o

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

pen: $(OBJS)
	$(CC) $(CFLAGS) -o pen $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

install: pen
	# mkdir -p $(BINDIR)
	# cp pen $(BINDIR)/pen
	install -Dm755 pen $(BINDIR)/pen

uninstall:
	rm -f $(BINDIR)/pen

clean:
	rm -f *.o pen

