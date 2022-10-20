PWD := $(CURDIR)

SRC_DIR = $(PWD)/src
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

INC_DIR = $(PWD)/include

LIB_DIR = $(PWD)/lib
LDFLAGS = -L $(LIB_DIR) -l qrng

BIN_DIR = $(PWD)/bin
EXE_NAME = qrng.out
EXECUTABLE = $(BIN_DIR)/$(EXE_NAME)

CXX_FLAGS += -I $(INC_DIR)
CXX_FLAGS += $(LDFLAGS)
CXX_FLAGS += -o $(BIN_DIR)/$(EXE_NAME)

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC_FILES)
	mkdir -p $(BIN_DIR)
	g++ $(CXX_FLAGS) $(SRC_FILES)

install: $(EXECUTABLE)

clean:
	rm -rf $(PWD)/bin