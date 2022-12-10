CXX = g++
STD = -std=c++14
CFLAGS = -Wall -Wextra -pedantic -Wcast-align -Wpointer-arith -Wcast-qual -Wno-missing-braces -Werror
LIBS = -lboost_system -pthread -lncurses


all: server client

server: server.o helper.o
	$(CXX) $(CFLAGS) $(STD) -o server server.o helper.o $(LIBS)

server.o: server.cpp helper.cpp
	$(CXX) $(CFLAGS) $(STD) -c server.cpp helper.cpp

client: client.o helper.o
	$(CXX) $(CFLAGS) $(STD) -o client client.o helper.o $(LIBS)

client.o: client.cpp helper.cpp
	$(CXX) $(CFLAGS) $(STD) -c client.cpp helper.cpp

release: CFLAGS += -O3

release: server client

clean:
	$(RM) *.o server client *.exe

remake: clean all

remake_release: clean release
