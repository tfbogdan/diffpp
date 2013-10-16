CC=g++
CFLAGS=-c -std=c++11 -I../.. 
CFLAGS_DEBUG=$(CFLAGS) -ggdb
CFLAGS_RELEASE=$(CFLAGS) -O3
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test

all: release

debug: main.o.dbg test.out.dbg

release: main.o test.out
	
clean:
	rm -f release/*.o
	rm -f release/test
clean_debug:
	rm -f debug/*.o
	rm -f debug/test
main.o: 
	$(CC) $(CFLAGS_RELEASE) main.cpp -o release/main.o

main.o.dbg:
	$(CC) $(CFLAGS_DEBUG) main.cpp -o debug/main.o

test.out:
	mkdir -p release
	$(CC) release/main.o -o release/test 

test.out.dbg:
	mkdir -p debug
	$(CC) debug/main.o -o debug/test 
