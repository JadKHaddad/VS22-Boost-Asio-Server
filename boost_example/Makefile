CXX = g++
STD = -std=c++14
CFLAGS += -Wall
LIBS = -lboost_system -pthread

server:
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o server server.cpp

client:
	$(CXX) $(CFLAGS) $(STD) $(LIBS) -o client client.cpp

clean:
	$(RM) $(TARGET)