CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: server

server: Stock_database.cpp request.cpp request.h server.cpp server.h socket.cpp socket.h tinyxml2.cpp tinyxml2.h exception.h
	$(CC) $(CFLAGS) -o server Stock_database.cpp request.cpp server.cpp socket.cpp tinyxml2.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o server

clobber:
	rm -f *~ *.o