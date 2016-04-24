cmake_minimum_required(VERSION 2.6)
project(juego-vida)

add_definitions(-std=c99)

add_executable(juego-vida list.h world.c world.h game.c game.h main.c)
target_link_libraries(juego-vida ssl crypto)

add_custom_target(check
        checkpatch.pl -q --no-tree -f --fix-inplace ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/*.h)

add_custom_target(download
        wget "http://www.mcs.anl.gov/~kazutomo/list/list.h" -O "${CMAKE_SOURCE_DIR}/list.h")

enable_testing()

add_test(NAME command_line_parsing_complex
        COMMAND ${CMAKE_BINARY_DIR}/juego-vida -f ${CMAKE_SOURCE_DIR}/juego-vida.args -d 45 -f ${CMAKE_SOURCE_DIR}/juego-vida.args -r 5 -c 2 -d 50 -f ${CMAKE_SOURCE_DIR}/juego-vida.args -d 50 -r 10 -c 50 -g 5)
