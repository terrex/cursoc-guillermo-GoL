cmake_minimum_required(VERSION 2.6)
project(juego-vida)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_definitions(-std=c99 -D_BSD_SOURCE)

add_executable(juego-vida list.h world.c world.h game.c game.h world_normal.c world_normal.h world_toroidal.c world_toroidal.h main.c)
target_link_libraries(juego-vida)

add_executable(juego-vida-gui list.h world.c world.h game.c game.h world_normal.c world_normal.h world_toroidal.c world_toroidal.h gui.h gui.c main-gui.c)
target_link_libraries(juego-vida-gui ${GTK3_LIBRARIES})
configure_file(builder.ui builder.ui COPYONLY)
configure_file(icon-torus.png icon-torus.png COPYONLY)
configure_file(icon-torus.svg icon-torus.svg COPYONLY)

add_custom_target(check
        ${CMAKE_SOURCE_DIR}/checkpatch.pl -q --no-tree -f --fix-inplace ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/*.h)

add_custom_target(download
        COMMAND wget "http://www.mcs.anl.gov/~kazutomo/list/list.h" -O "${CMAKE_SOURCE_DIR}/list.h"
        COMMAND wget "https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl" -O "${CMAKE_SOURCE_DIR}/checkpatch.pl"
        COMMAND chmod +x "${CMAKE_SOURCE_DIR}/checkpatch.pl")

enable_testing()

add_test(NAME command_line_parsing_complex
        COMMAND juego-vida -f ${CMAKE_SOURCE_DIR}/juego-vida.args -d 45 -f ${CMAKE_SOURCE_DIR}/juego-vida.args -r 5 -c 2 -d 50 -f ${CMAKE_SOURCE_DIR}/juego-vida.args -d 50 -r 10 -c 50 -g 5)

add_test(NAME command_line_prevent_infinite_loop
        COMMAND juego-vida -f ${CMAKE_SOURCE_DIR}/test_infinite_loop.args)

add_test(NAME output_stats_for_gnuplot
        COMMAND juego-vida -g 15 -o juego-vida.plot)

add_test(NAME save_and_continue1
        COMMAND juego-vida -r 10 -c 20 -g 15 -w juego-vida.dat)

add_test(NAME save_and_continue2
        COMMAND juego-vida -l juego-vida.dat -g 20 -w juego-vida.dat -o juego-vida.plot)

add_test(NAME save_and_continue1-normal
        COMMAND juego-vida --normal -r 10 -c 20 -g 15 -w juego-vida-normal.dat)

add_test(NAME save_and_continue2-normal
        COMMAND juego-vida --normal -l juego-vida-normal.dat -g 20 -w juego-vida-normal.dat -o juego-vida.plot)

add_test(NAME save_and_continue1-toroidal
        COMMAND juego-vida --toroidal -r 10 -c 20 -g 15 -w juego-vida-toroidal.dat)

add_test(NAME save_and_continue2-toroidal
        COMMAND juego-vida --toroidal -l juego-vida-toroidal.dat -g 20 -w juego-vida-toroidal.dat -o juego-vida.plot)
