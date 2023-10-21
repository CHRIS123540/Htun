# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall

# Target binary name
TARGET = hopa

# Source files
SRCS = main.c udp_utils.c

# Object files generated from source files
OBJS = $(SRCS:.c=.o)

all: $(TARGET)
	rm -f $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)  -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

