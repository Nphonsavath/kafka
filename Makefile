CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -g -std=c++23

.PHONY: all clean

all: server client

request.o: request.cpp request.hpp kafka_protocol.hpp
	$(CXX) $(CXXFLAGS) -c request.cpp -o $@

response.o: response.cpp response.hpp kafka_protocol.hpp
	$(CXX) $(CXXFLAGS) -c response.cpp -o $@

server.o: kafka_server.cpp request.hpp response.hpp kafka_protocol.hpp
	$(CXX) $(CXXFLAGS) -c kafka_server.cpp -o $@

client.o: kafka_client.cpp request.hpp response.hpp kafka_protocol.hpp
	$(CXX) $(CXXFLAGS) -c kafka_client.cpp -o $@

server: server.o request.o response.o
	$(CXX) $(CXXFLAGS) $^ -o $@

client: client.o request.o response.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o server client
