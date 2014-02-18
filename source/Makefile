.PHONY: usage all fast small debug debug2 debug3 profile test vtest coverage clean trotLib trotTest

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

usage:
	@echo "targets:"
	@echo "  fast     - build fast version"
	@echo "  small    - build small version"
	@echo "  debug    - build fast version, with smaller TROT_MAX_CHILDREN"
	@echo "             useful when running vtest"
	@echo "  debug2   - build debug version, with smaller TROT_MAX_CHILDREN"
	@echo "             useful when running test"
	@echo "  debug3   - build debug version, with smaller TROT_MAX_CHILDREN, and PARANOID checks"
	@echo "             useful to run with PARANOID checks, for extra protection against"
	@echo "             code changes"
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
debug3: CFLAGS += -DBE_PARANOID
debug3: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug3: all

debug2: ARGS = debug2
debug2: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug2: all

debug: ARGS = debug
debug: CFLAGS += -DTROT_MAX_CHILDREN=5000
debug: fast

profile: ARGS = profile
profile: all

test: all
	trotTest/trotTest -t all

vtest: all
	valgrind --leak-check=full trotTest/trotTest -t all

coverage:
	cd trotLib && $(MAKE) coverage

clean:
	rm -rf html
	rm -f gmon.out
	cd trotLib && $(MAKE) clean
	cd trotTest && $(MAKE) clean

trotTest: trotLib
	cd trotTest && $(MAKE) $(ARGS)

trotLib:
	cd trotLib && $(MAKE) $(ARGS)
