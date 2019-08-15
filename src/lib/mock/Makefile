CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)

mock: $(SRCS)
			$(CC) -c -o mock.o $(SRCS) $(LDFLAGS)
clean:
	rm -f *.o *.out
.PHONY:  clean