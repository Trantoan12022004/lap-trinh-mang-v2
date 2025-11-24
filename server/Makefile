CC = gcc
CFLAGS = -Wall -pthread -I/usr/include/postgresql
LDFLAGS = -lpq -ljson-c -lssl -lcrypto -luuid

TARGET = server
OBJS = server.o auth_handler.o database.o

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

server.o: server.c
    $(CC) $(CFLAGS) -c server.c

auth_handler.o: auth_handler.c
    $(CC) $(CFLAGS) -c auth_handler.c

database.o: database.c
    $(CC) $(CFLAGS) -c database.c

clean:
    rm -f $(TARGET) $(OBJS)