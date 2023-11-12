EXECUTABLE = out/lamai
BUILD_DIR = out/

all:
	$(MAKE) -C runtime
	$(MAKE) -C byterun
	# $(MAKE) -C interpreter
	$(MAKE) -C runtime unit_tests.o
	$(MAKE) -C runtime invariants_check.o
	$(MAKE) -C runtime invariants_check_debug_print.o

clean:
	# $(MAKE) clean -C interpreter
	$(MAKE) clean -C runtime
	$(MAKE) clean -C byterun