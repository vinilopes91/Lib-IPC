TARGET=test
CC=gcc
DEBUG=-g
OPT=-O0
WARN=-Wall
WERROR=-Werror
FPIC=-fpic
SHARED=-shared
CCFLAGS=$(DEBUG) $(OPT) $(WARN)
OBJS= ipc.o libipc.so
LD_PATH=-L/home/viniciuslopes/Lib-IPC
LIPC=-lipc

all: main

main: src/main.c libipc.so
	$(CC) $(LD_PATH) $(WARN) -o main src/main.c $(LIPC)
 
libipc.so: src/ipc.o
	$(CC) $(SHARED) -o libipc.so src/ipc.o

ipc.o: src/ipc.c
	$(CC) -c $(WARN) $(WERROR) $(FPIC) -o ./ipc.o src/ipc.c
 
clean:
	rm -f src/*.o

# export LD_LIBRARY_PATH=/home/viniciuslopes/Lib-IPC
