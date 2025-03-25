ifndef FOLDER
$(error FOLDER is not set. Usage: make FOLDER=folder_name [target])
endif

CC = clang
CFLAGS = -I/usr/local/include -DCL_TARGET_OPENCL_VERSION=120
LDFLAGS = -framework OpenCL
SRC_DIR = $(FOLDER)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
TARGETS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGETS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS)
