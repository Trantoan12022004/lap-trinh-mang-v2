CC = gcc
CFLAGS = -Wall
LDFLAGS = -ljson-c

TARGET = client
OBJS = client.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f $(TARGET) $(OBJS)