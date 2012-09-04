.PHONY: usage all debug debug2 test vtest coverage clean trotLib trotTest

ARGS =

CFLAGS += -Wall -Werror -Wextra

# Uncomment one of the following to test that our code compiles against a standard
#CFLAGS += -std=c89 -pedantic
#CFLAGS += -std=c99 -pedantic
#CFLAGS += -std=c1x -pedantic

export CFLAGS

usage:
	@echo "targets:"
	@echo "  all      - build normal"
	@echo "  debug    - build for debug"
	@echo "  profile  - build for profiling"
	@echo "  test     - run tests"
	@echo "  vtest    - run tests in valgrind"
	@echo "  coverage - generate coverage report with gcov and lcov"
	@echo "  clean    - cleans up files"

all: trotLib trotTest

debug2: ARGS = debug2
debug2: CFLAGS += -DBE_PARANOID
debug2: all

debug: ARGS = debug
debug: all

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
