CC = gcc
AR = ar
CFLAGS = -g -Wall `pkg-config --cflags purple`
LIBS = `pkg-config --libs purple`
MAIN = libpurplebackports.a

all: $(MAIN)

$(MAIN): *.c *.h
	$(CC) $(CFLAGS) -c *.c $(LIBS)
	$(AR) rcs $(MAIN) *.o
