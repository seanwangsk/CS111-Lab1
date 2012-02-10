# CS 111 Lab 1 Makefile

CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused #-Werror
LAB = 1
LIBS = -lsqlite3
DISTDIR = lab1-$(USER)

all: timetrash

TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))

TIMETRASH_SOURCES = \
  alloc.c \
  command-internals.c \
  execute-command.c \
  main.c \
  read-command.c \
  print-command.c \
  mysqlite.c
TIMETRASH_OBJECTS = $(subst .c,.o,$(TIMETRASH_SOURCES))

DIST_SOURCES = \
  $(TIMETRASH_SOURCES) alloc.h command.h command-internals.h mysqlite.h Makefile \
  $(TESTS) check-dist README

timetrash: $(TIMETRASH_OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(TIMETRASH_OBJECTS)

alloc.o: alloc.h
mysqlite.o: mysqlite.h 
execute-command.o main.o print-command.o read-command.o: command.h mysqlite.h

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES) check-dist
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	./check-dist $(DISTDIR)
	mv $@.tmp $@

check: $(TEST_BASES)

$(TEST_BASES): timetrash
	./$@.sh

clean:
	rm -fr *.o *~ *.bak *.tar.gz core *.core *.tmp timetrash $(DISTDIR)

.PHONY: all dist check $(TEST_BASES) clean
