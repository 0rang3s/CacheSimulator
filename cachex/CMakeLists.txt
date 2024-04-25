cmake_minimum_required(VERSION 3.18)
project(cachex C)

set(CMAKE_C_STANDARD 99)

add_executable(cachex main.c
        cache.c
        cache.h)
