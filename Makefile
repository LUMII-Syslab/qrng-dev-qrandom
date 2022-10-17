PWD := $(CURDIR)

SRC_DIR = $(PWD)/u-src
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

KDIR = /lib/modules/$(shell uname -r)/build
KMOD_DIR = $(PWD)/k-src

CONFIG_DIR = $(PWD)/config
CONF_FILES = $(CONFIG_DIR)/ca.truststore $(CONFIG_DIR)/qrng.properties $(CONFIG_DIR)/token.keystore  

all: $(EXECUTABLE) KMODULE

$(EXECUTABLE):
	mkdir -p $(BIN_DIR)
	g++ $(CXX_FLAGS) $(SRC_FILES)


KMODULE:
	make -C $(KDIR) M=$(KMOD_DIR)


install: $(EXECUTABLE) $(CONF_FILES)
	cp $(CONFIG_DIR)/* $(BIN_DIR)

clean:
	make -C $(KDIR) M=$(KMOD_DIR) clean
	rm -rf $(PWD)/bin