CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude
SRC     = src/main.c src/shell.c \
          src/room_attendance.c src/room_records.c \
          src/room_election.c src/room_readability.c \
          src/room_games.c src/room_ledger.c
OBJ     = $(SRC:.c=.o)
TARGET  = vault

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: all
	./$(TARGET)
