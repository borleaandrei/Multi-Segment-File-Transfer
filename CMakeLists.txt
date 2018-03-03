cmake_minimum_required(VERSION 3.7)
enable_language(C)
find_package(Threads)

project(Multi-Segment-File-Transfer C)

set(GCC_PTHREAD_FLAGS "-pthread")
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "${GCC_PTHREAD_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${GCC_PTHREAD_FLAGS}")

set(SOURCE_FILES server.c server.h netio.h netio.c file_utilities.c file_utilities.h)
add_executable(Multi-Segment-File-Transfer ${SOURCE_FILES})

