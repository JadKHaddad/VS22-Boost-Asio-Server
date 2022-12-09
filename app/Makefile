CXX = g++
STD = -std=c++14
CFLAGS += -Wall -Wextra -pedantic -Wcast-align -Wpointer-arith -Wcast-qual -Wno-missing-braces -Werror
LIBS = -lboost_system -pthread


all: server client

server: server.o helper.o
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o server server.o helper.o

server.o: server.cpp helper.cpp
	$(CXX) $(CFLAGS) $(STD) -c server.cpp helper.cpp

client: client.o helper.o
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o client client.o helper.o

client.o: client.cpp helper.cpp
	$(CXX) $(CFLAGS) $(STD) -c client.cpp helper.cpp

clean:
	$(RM) *.o server client *.exe
