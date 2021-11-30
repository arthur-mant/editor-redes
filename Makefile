CXX = g++
CXXFLAGS = -O2 -Wall

EXE = server client
CXXFILES = $(wildcard *.cpp)
OBJECTS = $(CXXFILES:.cpp=.o)

all : $(EXE)
	@echo "Building..."

server : server.o common.o
	$(CXX) $^ $(CXXFLAGS) -o $@

client : client.o common.o
	$(CXX) $^ $(CXXFLAGS) -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean :
	rm -rf $(EXE) $(OBJECTS)
