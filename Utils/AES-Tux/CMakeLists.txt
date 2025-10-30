cmake_minimum_required(VERSION 3.10)

project(AesEcbPenguin CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(penguin_encrypt main.cpp aes.cpp)

message(STATUS "Project configured. Use 'make' to build.")