.PHONY: all debug2 debug coverage clean

CFLAGS += -Wall -Werror -Wextra

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: CFLAGS += -O3
all: libTrot.a

debug2: CFLAGS += -DPRINT_ERR=1
debug2: debug

debug: CFLAGS += -DTEST_PRECOND=1 -fprofile-arcs -ftest-coverage -g
debug: libTrot.a

profile: CFLAGS += -pg
profile: all

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

