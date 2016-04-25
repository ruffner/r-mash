CC=gcc
CCX=g++
CFLAGS=-g -O1 -Wall
CXXFLAGS=-std=c++0x
LDLIBS=-lpthread

all: rrsh-server rrsh-client

rrsh-server: rrsh-server.cc csapp.o parser.o RRSHServerSession.o
rrsh-client: rrsh-client.cc csapp.o
parser.o: rrsh.h parser.c
csapp.o: csapp.h csapp.c
RRSHServerSession.o: RRSHServerSession.h RRSHServerSession.cc

clean:
	rm -f *.o *~ rrsh-server rrsh-client csapp.o RRSHServerSession.o

