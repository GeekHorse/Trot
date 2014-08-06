.PHONY: usage all fast small debug debug2 debug3 single singleFast singleSmall singleDebug singleCoverage profile test vtest coverage clean trotLib trotTest

ARGS =

CFLAGS += -Wall -Werror -Wextra

# targets to test against c89, c99, and c11 standards
c89: CFLAGS += -std=c89 -pedantic
c89: all

c99: CFLAGS += -std=c99 -pedantic
c99: all

c11: CFLAGS += -std=c11 -pedantic
c11: all

export CFLAGS
export LDFLAGS

usage:
	@echo "targets:"
	@echo "  fast     - build fast version"
	@echo "  small    - build small version"
	@echo "  debug    - build debug version, with smaller TROT_MAX_CHILDREN"
	@echo "  debug2   - build debug version, with smaller TROT_MAX_CHILDREN,"
	@echo "             and necessary flags to generate test coverage."
	@echo "  debug3   - build debug version, with smaller TROT_MAX_CHILDREN,"
	@echo "             TROT_ENABLE_LOGGING, PARANOID checks,"
	@echo "             and necessary flags to generate test coverage."
	@echo "  single   - build a combined trotSingle.c file"
	@echo "  profile  - build for profiling"
	@echo "  test     - run tests"
	@echo "  vtest    - run tests in valgrind"
	@echo "  coverage - generate coverage report with gcov and lcov"
	@echo "  clean    - cleans up files"

all: trotLib trotTest

fast: ARGS = fast
fast: all

small: ARGS = small
small: all

debug3: ARGS = debug3
debug3: CFLAGS += -DTROT_ENABLE_LOGGING
debug3: CFLAGS += -DBE_PARANOID
debug3: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug3: CFLAGS += -DTROT_DEBUG=3
debug3: all

debug2: ARGS = debug2
debug2: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug2: CFLAGS += -DTROT_DEBUG=2
debug2: all

debug: ARGS = debug
debug: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug: CFLAGS += -DTROT_DEBUG=1
debug: fast

single: ARGS = single
single: all

singleFast: ARGS = singleFast
singleFast: all

singleSmall: ARGS = singleSmall
singleSmall: all

singleDebug: ARGS = singleDebug
singleDebug: CFLAGS += -DTROT_MAX_CHILDREN=5000
singleDebug: CFLAGS += -DTROT_DEBUG=1
singleDebug: all

profile: ARGS = profile
profile: all

test: trotTest
	trotTest/trotTest -t all

vtest:
	valgrind --leak-check=full trotTest/trotTest -t all

coverage:
	cd trotLib && $(MAKE) coverage

singleCoverage:
	cd trotLib && $(MAKE) singleCoverage

clean:
	rm -rf html
	rm -f gmon.out
	cd trotLib && $(MAKE) clean
	cd trotTest && $(MAKE) clean

trotTest:
	cd trotTest && $(MAKE) $(ARGS)

trotLib:
	cd trotLib && $(MAKE) $(ARGS)
