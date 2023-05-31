CXX				:= g++
RM				:= rm -rf
CXXFLAGS		:= -std=c++20 -Werror -Wall -Wpedantic
TEST_FLAGS		:= -lgtest -lgtest_main -pthread
TARGET			:= ipkcpc
BUILD			:= ./build
OBJ_DIR			:= $(BUILD)/objects
TESTS_DIR		:= ./tests
SRC_FILES		:=					\
	$(wildcard src/*.cpp)			\
	$(wildcard src/tcp/*.cpp)		\
	$(wildcard src/udp/*.cpp)		\
	
OBJECTS 		:= $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)
#	Rules for compiling on Windows
ifeq ($(OS),Windows_NT)
.PHONY: all clean build

all: clean build

build:
	@md build build\tcp build\udp
	$(CXX) $(CXXFLAGS) -c .\src\tcp\$(TARGET)_tcp.cpp -o .\build\tcp\$(TARGET)_tcp.o -lws2_32
	$(CXX) $(CXXFLAGS) -c .\src\udp\$(TARGET)_udp.cpp -o .\build\udp\$(TARGET)_udp.o -lws2_32
	$(CXX) $(CXXFLAGS) -c .\src\$(TARGET).cpp -o .\build\$(TARGET).o -lws2_32
	$(CXX) $(CXXFLAGS) -o $(TARGET) .\build\tcp\$(TARGET)_tcp.o .\build\udp\$(TARGET)_udp.o .\build\$(TARGET).o -lws2_32

clean:
	@if exist $(TARGET).exe del $(TARGET).exe
	@if exist .\build\tcp rmdir .\build\tcp /s /q
	@if exist .\build\udp rmdir .\build\udp /s /q
	@if exist .\build rmdir .\build /s /q

#	Rules for compiling on linux
else
.PHONY: all test clean build

all: clean build ./$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

./$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

build:
	@mkdir -p $(OBJ_DIR)

test: all $(TESTS_DIR)/tests.cpp
	@$(CXX) $(TESTS_DIR)/tests.cpp $(TEST_FLAGS) -o $(TESTS_DIR)/tests
	@./$(TESTS_DIR)/tests

clean:
	@$(RM) $(TARGET)
	@$(RM) $(TESTS_DIR)/tests
	@$(RM) $(OBJ_DIR)
endif

