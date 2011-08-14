.PHONY: all debug debug2 clean

CFLAGS += -Wall -Werror -Wextra
CFLAGS += -I../trotLib
LDFLAGS += ../trotLib/libTrot.a

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: CFLAGS += -O3
all: trotTest

debug2: debug

debug: CFLAGS += -DTEST_PRECOND=1 -g
debug: LDFLAGS += -fprofile-arcs
debug: trotTest

clean:
	rm -f trotTest
	rm -f *.o

trotTest : $(OBJECTS) $(HEADERS) ../trotLib/libTrot.a
	$(CC) *.o $(LDFLAGS) -o trotTest

%.o : %.c *.h
	$(CC) $(CFLAGS) -c $<

