# Compiler and tools
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Ilib/Unity/src -Iapp -Ilib/FreeRTOS-Kernel/include -Ilib/FreeRTOS-Kernel/portable/GCC/ARM_CM0 -Iconfig

# Include directories
INCLUDES = -I./ -I./app -I./app/tasks -I./app/ssd1306 -I./app/i2c -I./app/scd40 -I./lib/FreeRTOS-Kernel/include -I./lib/FreeRTOS-Kernel/portable/GCC/ARM_CM0 -I./config

# Source files for FreeRTOS
FREERTOS_SRC = \
    lib/FreeRTOS-Kernel/event_groups.c \
    lib/FreeRTOS-Kernel/list.c \
    lib/FreeRTOS-Kernel/queue.c \
    lib/FreeRTOS-Kernel/stream_buffer.c \
    lib/FreeRTOS-Kernel/tasks.c \
    lib/FreeRTOS-Kernel/timers.c \
    lib/FreeRTOS-Kernel/portable/MemMang/heap_3.c \
    lib/FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c

# Source files for the tests
TEST_SRC = \
    tests/test_update_display_task.c \
    tests/mock_display.c \
    tests/mock_oled.c \
    tests/mock_cyw43.c \
    lib/Unity/src/unity.c

# Object files
OBJS = $(FREERTOS_SRC:.c=.o) $(TEST_SRC:.c=.o)

# Output file
TARGET = tests.elf

# Default target
all: $(TARGET)

# Link the test executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the tests
run_tests: $(TARGET)
	./$(TARGET)

