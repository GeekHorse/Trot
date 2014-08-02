.PHONY: all fast small debug debug2 debug3 single singleFast singleSmall singleDebug singleCoverage profile coverage clean

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: libTrot.a

fast: CFLAGS += -O3
fast: all

small: CFLAGS += -Os
small: all

debug3: debug2

debug2: CFLAGS += -fprofile-arcs -ftest-coverage -g
debug2: all

debug: fast

singleFast: CFLAGS += -O3
singleFast: single

singleSmall: CFLAGS += -Os
singleSmall: single

singleDebug: CFLAGS += -fprofile-arcs -ftest-coverage -g
singleDebug: single

single:
	rm -f trotSingle.c.temp
	rm -f trotSingle.c
	cat trotInternal.h > trotSingle.c.temp
	cat *.c >> trotSingle.c.temp
	cat trotSingle.c.temp | grep -v trotInternal.h > trotSingle.c
	rm trotSingle.c.temp
	$(CC) $(CFLAGS) -c trotSingle.c
	ar rcs libTrot.a trotSingle.o

profile: CFLAGS += -pg
profile: fast

coverage:
	gcov -o . *.c
	mkdir lcov
	lcov --directory . --capture --output-file ./lcov/trot.info
	genhtml ./lcov/trot.info -o ./lcov
	@echo "Load ./trotLib/lcov/index.html in your browser for lcov data."

singleCoverage:
	gcov -o . ./trotSingle.c
	mkdir lcov
	lcov --directory . --capture --output-file ./lcov/trot.info
	genhtml ./lcov/trot.info -o ./lcov
	@echo "Load ./trotLib/lcov/index.html in your browser for lcov data."

clean:
	rm -f trotSingle.c
	rm -f libTrot.a
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -rf lcov

libTrot.a: $(OBJECTS) $(HEADERS)
	ar rcs libTrot.a *.o

%.o : %.c *.h
	$(CC) $(CFLAGS) -c $<

