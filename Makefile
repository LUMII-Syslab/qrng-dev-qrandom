PWD := .

SRC_DIR = $(PWD)/src
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

INC_DIR = $(PWD)/include

LIB_DIR = $(PWD)/lib
LDFLAGS = -L $(LIB_DIR) -l qrng

BIN_DIR = $(PWD)/bin
EXE_NAME = test.out

DATA_DIR = $(PWD)/data

CXX_FLAGS += -I $(INC_DIR)
CXX_FLAGS += $(LDFLAGS)
CXX_FLAGS += -o $(BIN_DIR)/$(EXE_NAME)

$(EXE_NAME):
	mkdir -p $(BIN_DIR)
	cp $(DATA_DIR)/* $(BIN_DIR)
	g++ $(CXX_FLAGS) $(SRC_FILES)