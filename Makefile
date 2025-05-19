ifndef FOLDER
$(error FOLDER is not set. Usage: make FOLDER=folder_name [target])
endif

CC = clang

# Platform-specific CFLAGS and LDFLAGS for OpenCL
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS = -I/usr/local/include -DCL_TARGET_OPENCL_VERSION=120
    LDFLAGS = -framework OpenCL
else
    CFLAGS = -DCL_TARGET_OPENCL_VERSION=120
    LDFLAGS = -lOpenCL
endif

SRC_DIR = $(FOLDER)
BUILD_DIR = build
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
BIN_NAME = $(notdir $(SRC_DIR))
OUT_PATH = $(BUILD_DIR)/$(BIN_NAME)

.PHONY: all clean

all: $(OUT_PATH)

$(OUT_PATH): $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(SRC_FILES) -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BUILD_DIR)/*
