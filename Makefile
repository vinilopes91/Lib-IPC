CC = gcc
DEBUG = -g
OPT = -O0
WARN = -Wall
WERROR = -Werror
PTHREAD = -pthread
FPIC = -fPIC
SHARED = -shared
CCFLAGS = ${DEBUG} ${OPT} ${WARN} ${PTHREAD}
LIBFLAGS = ${WARN} ${WERROR} ${FPIC}
OBJS = ipc.o libipc.so
LD_PATH = -L/home/leonardommj/Lib-IPC
LIPC = -lipc

all: sendA-example sendS-example

sendA-example: src/sendA-example.c libipc.so
	${CC} ${LD_PATH} ${CCFLAGS} -o sendA-example.exe src/sendA-example.c ${LIPC}

sendS-example: src/sendS-example.c libipc.so
	${CC} ${LD_PATH} ${CCFLAGS} -o sendS-example.exe src/sendS-example.c ${LIPC}

libipc.so: ipc.o
	${CC} ${SHARED} -o libipc.so src/ipc.o

ipc.o: src/ipc.c src/ipc.h
	${CC} -c ${LIBFLAGS} -o src/ipc.o src/ipc.c

clean:
	rm -f src/*.o

# export LD_LIBRARY_PATH=/home/viniciuslopes/Lib-IPC
# gcc -c -Wall -Werror -fpic -o src/ipc.o src/ipc.c
# gcc -shared -o libipc.so src/ipc.o
# gcc -L/home/viniciuslopes/Lib-IPC -g -O0 -Wall -Werror -pthread -o test.exe src/main.c -lipc
