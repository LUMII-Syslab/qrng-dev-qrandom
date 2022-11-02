PWD := $(CURDIR)

SRC_DIR = $(PWD)/src
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

INC_DIR = $(SRC_DIR)
INC_FILES = $(wildcard $(INC_DIR)/*.hpp)

BUILD_DIR = $(PWD)/build

EXE_NAME = qrng.out
EXECUTABLE = $(BUILD_DIR)/$(EXE_NAME)

CXX_FLAGS += -I $(INC_DIR)
CXX_FLAGS += -l qrng

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC_FILES) $(INC_FILES)
	mkdir -p $(BUILD_DIR)
	g++ $(CXX_FLAGS) $(SRC_FILES) -o $@

install: $(EXECUTABLE)

clean:
	rm -rf $(PWD)/bin