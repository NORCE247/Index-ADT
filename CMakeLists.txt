cmake_minimum_required(VERSION 3.28)
project(Index_ADT C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(Index_ADT
        include/common.h
        include/index.h
        include/list.h
        include/map.h
        include/printing.h
        include/trie.h
        include/ui.h
        src/benchmark.c
        src/common.c
        src/hashmap.c
        src/index.c
        src/linkedlist.c
        src/main.c
        src/test.c
        src/timer.c
        src/trie.c
        src/ui.c)
