CFLAGS = -std=c99 -g
CC = /usr/bin/gcc $(CFLAGS)

.PHONY: all clean

all: clean main

clean:
	rm -rf *.o *.out ./main

common.o: common.c common.h
	$(CC) -c common.c

error.o: error.c error.h common.h
	$(CC) -c error.c

readjson.o: readjson.c readjson.h common.h
	$(CC) -c readjson.c

writejson.o: writejson.c writejson.h common.h
	$(CC) -c writejson.c

main.o: main.c common.h error.h readjson.h writejson.h
	$(CC) -c main.c

main: common.o error.o readjson.o writejson.o main.o
	$(CC) common.o error.o readjson.o writejson.o main.o -o main
