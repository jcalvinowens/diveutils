CFLAGS = -O2 -std=c11

WFLAGS = -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wno-switch \
	 -Wmissing-declarations -Werror=implicit -Wdeclaration-after-statement \
	 -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict \
	 -Wnull-dereference -Wjump-misses-init -Wdouble-promotion -Wshadow \
	 -Wformat=2 -Wstrict-aliasing -Wno-unknown-warning-option \
	 -Wno-format-nonliteral

all: altitude-calculator ndl-calculator
obj = args.o haldanian.o

altitude-calculator: altitude-calculator.o
altitude-calculator: constants.h $(obj)
	$(CC) $(CFLAGS) $(obj) altitude-calculator.o -lm -o $@

ndl-calculator: ndl-calculator.o
ndl-calculator: constants.h $(obj)
	$(CC) $(CFLAGS) $(obj) ndl-calculator.o -lm -o $@

constants.h: constants.py
	./constants.py > constants.h

%.o: %.c
	$(CC) $< $(WFLAGS) $(CFLAGS) -c -o $@

clean:
	rm -f *.o
	rm -f altitude-calculator
	rm -f ndl-calculator
