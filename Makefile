CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -g -std=c++23
CPPFLAGS = -Iinclude -MMD -MP

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

TARGETS = $(BIN_DIR)/server $(BIN_DIR)/client

.PHONY: all clean

all: $(TARGETS) 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BIN_DIR)/server: $(BUILD_DIR)/kafka_server.o $(BUILD_DIR)/request.o $(BUILD_DIR)/response.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(BIN_DIR)/client: $(BUILD_DIR)/kafka_client.o $(BUILD_DIR)/request.o $(BUILD_DIR)/response.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
