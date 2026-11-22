CC = gcc
CFLAGS ?= -g -O0 -w
SRC = src/main.c src/util.c
OUT = vuln-demo

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT) *.o
