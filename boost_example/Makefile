CXX = g++
STD = -std=c++14
CFLAGS += -Wall
LIBS = -lboost_system -pthread


all: server client

server: server.o
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o server server.o

server.o: server.cpp
	$(CXX) $(CFLAGS) $(STD) -c server.cpp

client: client.o
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o client client.o

client.o: client.cpp
	$(CXX) $(CFLAGS) $(STD) -c client.cpp

clean:
	rm *.o *.exe server client
