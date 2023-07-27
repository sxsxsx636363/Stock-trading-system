CC=g++
CFLAGS=-O3
EXTRAFLAGS=-pthread -lboost_filesystem

all: client

client: client.cpp client.h main.cpp ../server/socket.h ../server/socket.cpp ../server/tinyxml2.cpp ../server/tinyxml2.h ../server/exception.h
	$(CC) $(CFLAGS) -o client client.cpp main.cpp ../server/tinyxml2.cpp ../server/socket.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o client

clobber:
	rm -f *~ *.o