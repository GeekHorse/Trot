.PHONY: all fast small debug2 debug profile coverage clean

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: libTrot.a

fast: CFLAGS += -O3
fast: all

small: CFLAGS += -Os
small: all

debug2: CFLAGS += -DPRINT_ERR=1
debug2: debug

debug: CFLAGS += -DTROT_DEBUG=1 -fprofile-arcs -ftest-coverage -g
debug: all

profile: CFLAGS += -pg
profile: fast

coverage:
	gcov -o . *.c
	mkdir lcov
	lcov --directory . --capture --output-file ./lcov/trot.info
	genhtml ./lcov/trot.info -o ./lcov
	@echo "Load ./trotLib/lcov/index.html in your browser for lcov data."

clean:
	rm -f libTrot.a
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -rf lcov

libTrot.a: $(OBJECTS) $(HEADERS)
	ar rcs libTrot.a *.o

%.o : %.c *.h
	$(CC) $(CFLAGS) -c $<

