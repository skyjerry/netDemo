cmake_minimum_required(VERSION 3.15)
project(netDemo C)

set(CMAKE_C_STANDARD 99)

add_executable(netDemo main.c ae.h ae_select.c anet.c)