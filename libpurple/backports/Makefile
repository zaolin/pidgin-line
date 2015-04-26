CC = gcc
AR = ar
CFLAGS = -g -Wall -fPIC `pkg-config --cflags purple`
LIBS = `pkg-config --libs purple`
MAIN = libpurplebackports.a

all: $(MAIN)

$(MAIN): *.c *.h
	$(CC) $(CFLAGS) -c *.c $(LIBS)
	$(AR) rcs $(MAIN) *.o

.PHONY: clean
clean:
	rm -f $(MAIN)
	rm -f *.o
