.PHONY: usage all debug debug2 test vtest coverage clean trotLib trotTest

ARGS =

usage:
	@echo "targets:"
	@echo "  all      - build normal"
	@echo "  debug    - build for debug"
	@echo "  test     - run tests"
	@echo "  vtest    - run tests in valgrind"
	@echo "  coverage - generate coverage report with gcov and lcov"
	@echo "  clean    - cleans up files"

all: trotLib trotTest

debug2: ARGS = debug2
debug2: all

debug: ARGS = debug
debug: all

test: all
	trotTest/trotTest -t all

vtest: all
	valgrind --leak-check=full trotTest/trotTest -t all

coverage:
	cd trotLib && $(MAKE) coverage

clean:
	cd trotLib && $(MAKE) clean
	cd trotTest && $(MAKE) clean

trotTest: trotLib
	cd trotTest && $(MAKE) $(ARGS)

trotLib:
	cd trotLib && $(MAKE) $(ARGS)
