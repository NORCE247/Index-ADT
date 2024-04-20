CC=gcc
BIN=index
BIN2=indexVersion2

MAP_SRC:=hashmap.c
INDEX_SRC:=index.c
INDEX_SRC2:=indexVersion2.c
MAIN_SRC:=main.c

SRC:=common.c ui.c linkedlist.c trie.c $(MAP_SRC) $(INDEX_SRC)
SRC2:=common.c ui.c linkedlist.c trie.c $(MAP_SRC) $(INDEX_SRC2)
SRC:=$(patsubst %.c,src/%.c, $(SRC))
SRC2:=$(patsubst %.c,src/%.c, $(SRC2))

INCLUDE=include

CFLAGS=-Wall -Wextra -Wpedantic -std=gnu17 -O0 -g
LDFLAGS=-lm -lncurses -DLOG_LEVEL=0

.PHONY: all
.PHONY: index
.PHONY: indexVersion2
.PHONY: test
.PHONY: bench
.PHONY: clean

all: index test bench indexVersion2

index: src/*.c src/main.c Makefile
	$(CC) -o $(BIN) $(CFLAGS) $(SRC) src/main.c -I$(INCLUDE) $(LDFLAGS)

indexVersion2: src/*.c src/main.c Makefile
	$(CC) -o $(BIN2) $(CFLAGS) $(SRC2) src/main.c -I$(INCLUDE) $(LDFLAGS)

test: $(SRC) Makefile
	$(CC) -o test_index $(CFLAGS) $(SRC) src/test.c -I$(INCLUDE) $(LDFLAGS)
	$(CC) -o test_index2 $(CFLAGS) $(SRC2) src/test.c -I$(INCLUDE) $(LDFLAGS)

bench: $(SRC) Makefile
	$(CC) -o bench_index $(CFLAGS) $(SRC) src/benchmark.c -I$(INCLUDE) $(LDFLAGS)
	$(CC) -o bench_index2 $(CFLAGS) $(SRC2) src/benchmark.c -I$(INCLUDE) $(LDFLAGS)

run: test bench
	./test_index
	./bench_index 1> benchVersion1.txt
	./test_index2
	./bench_index2 1> benchVersion2.txt
	top -l 100 | grep bench_index2 >> top_output2.txt &

clean:
	rm -rf index test_index bench_index indexVersion2 test_index2 bench_index2 doc/*
