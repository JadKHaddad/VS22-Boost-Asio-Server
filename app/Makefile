# debian version 11.5
# gcc version 10.2.1 20210110 (Debian 10.2.1-6)

CXX = g++
STD = -std=c++14
CFLAGS = -Wall -Wextra -pedantic -Wcast-align -Wpointer-arith -Wcast-qual -Wno-missing-braces -Werror -Wformat -Wformat-security
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


release: CFLAGS += -O3 -static
release: LIBS += -ltinfo # neccessary for static linking of ncurses
release: server client

clean:
	$(RM) *.o server client

remake: clean all

remake_release: clean release
