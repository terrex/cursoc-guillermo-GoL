cmake_minimum_required(VERSION 2.6)
project(juego-vida)

add_executable(juego-vida world.c main.c)
target_link_libraries(juego-vida ssl crypto)

add_custom_target(check
        checkpatch.pl -q --no-tree -f --fix-inplace *.c)
