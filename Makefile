CC = g++
CFLAGS := -O2 -std=c++11

all:
	$(CC) $(CFLAGS) Test.cpp -o Test.exe

.PHONY: clean
clean:
	rm Test.exe Test.o Test.obj 2>/dev/null
