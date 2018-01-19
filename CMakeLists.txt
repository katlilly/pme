cmake_minimum_required(VERSION 2.8)

project(pme)

add_executable(pme pme.c)

install(TARGETS pme DESTINATION bin)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug)

