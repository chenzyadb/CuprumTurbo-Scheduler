cmake_minimum_required (VERSION 3.13)
project (CuprumTurbo-Scheduler)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

add_executable(CuDaemon ${SRC})