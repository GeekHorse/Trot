.PHONY: all fast small debug debug2 debug3 single profile clean

CFLAGS += -I../trotLib
LDFLAGS += ../trotLib/libTrot.a

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: trotTest

fast: CFLAGS += -O3
fast: all

small: fast

debug3: debug2

debug2: CFLAGS += -g
debug2: LDFLAGS += -fprofile-arcs
debug2: all

debug: fast

single: fast

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: fast

clean:
	rm -f trotTest
	rm -f *.o

trotTest : $(OBJECTS) $(HEADERS) ../trotLib/libTrot.a
	$(CC) *.o $(LDFLAGS) -o trotTest

%.o : %.c *.h
	$(CC) $(CFLAGS) -c $<

