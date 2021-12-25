cmake_minimum_required(VERSION 3.21)
project(hpc_2 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_COMPILER "/usr/bin/mpicc")
include_directories(/usr/include/mpi/)
add_executable(hpc_2 main.c)
