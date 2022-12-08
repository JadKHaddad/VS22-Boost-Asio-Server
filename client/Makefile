TARGET = out

IDIR = .
SDIR = .
ODIR = .

SRC = $(wildcard $(SDIR)/*.cpp)
OBJ = $(SRC:$(SDIR)/%.c=$(ODIR)/%.o)

CXX = g++
STD = -std=c++14
CFLAGS += -Wall
INCLUDE = -I../include
LIBS = -lboost_system -lpthread

$(TARGET): $(OBJ)
	$(CXX) $(INCLUDE) $^ $(LIBS) -o $@ 

$(ODIR)/%.o: $(SDIR)/%.c
	$(CXX) $(CFLAGS) $(STD) -c $< -o $@

clean:
	$(RM) $(TARGET)