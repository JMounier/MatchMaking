# cmake_minimum_required(VERSION <specify CMake version here>)
project(MatchMaking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXXFLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")

add_executable(MatchMaking src/main.cc src/match.cc src/match.hh, src/structures.hh)