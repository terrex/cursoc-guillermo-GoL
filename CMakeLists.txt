cmake_minimum_required(VERSION 2.6)
project(juego-vida)

add_definitions(-std=c99)

add_executable(juego-vida list.h world.c world.h main.c)
target_link_libraries(juego-vida ssl crypto)

add_custom_target(check
        checkpatch.pl -q --no-tree -f --fix-inplace ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/*.h)
