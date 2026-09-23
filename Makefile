CC = cc

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -Iincludes

TARGET = build/stn-miner

SOURCES = \
	src/main.c \
	src/stn_config.c \
	src/stn_backend.c \
	src/stn_display.c \
	src/stn_gpu.c \
	src/stn_log.c \
	src/stn_miner.c \
	src/stn_protocol.c \
	src/stn_hash.c \
	src/stn_cpu.c \
	platforms/linux/stn_gpu_linux.c \
	platforms/linux/stn_platform_linux.c

OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p build
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)