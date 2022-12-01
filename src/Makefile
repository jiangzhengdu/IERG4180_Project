#CC = g++
#CFLAGS = -Wall
#
#
#
#server : Server_Linux.cpp util.cpp
#	$(CC) $(CFLAGS) -o server Server_Linux.cpp util.cpp
#
#client : client.cpp util.cpp
#	$(CC) $(CFLAGS) -o client client.cpp util.cpp
#
#
#.PHONY: clean
#clean :
#	$(RM) *.o *.i server client
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)

target1 = server
target2 = client
CC = g++
CFLAGS = -Wall
src = $(wildcard *.c)
deps = $(wildcard *.h)
obj = $(patsubst %.c,%.o,$(src))

all:$(target1) $(target2)

$(target1): threadpool.o server.o util.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcrypto

$(target2): client.o util.o
	$(CC) $(CFLAGS) $^ -o $@ -Wall -lpthread  -I/usr/lib/ -lssl  -lcrypto

%.o: %.c $(deps)
	$(CC) $(CFLAGS) -c $< -o $@ -I/usr/lib/  -lssl  -lcrypto

.PHONY:clean
clean:
	-rm -rf $(target1) $(target2) $(obj) *.o


else ifeq ($(OS), Windows_NT)
#target1 = server
target2 = client.exe
CC = g++
CFLAGS = -Wall
src = $(wildcard *.c)
deps = $(wildcard *.h)
obj = $(patsubst %.c,%.o,$(src))

all:$(target2)

#$(target1): server.o util.o
#	$(CC) $(CFLAGS) $^ -o $@

$(target2): client.o util.o
	$(CC) $(CFLAGS) $^ -o $@  -lwsock32 -I.

%.o: %.c $(deps)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY:clean
clean:
	-rm -rf $(target2) $(obj) *.o
endif

