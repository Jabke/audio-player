cmake_minimum_required(VERSION 3.27)
project(audio_palyer)

set(CMAKE_CXX_STANDARD 17)

add_executable(audio_palyer main.cpp)

target_link_libraries(audio_palyer pulse-simple pulse)
