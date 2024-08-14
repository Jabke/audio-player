cmake_minimum_required(VERSION 3.27)
project(audio_palyer)

set(CMAKE_CXX_STANDARD 17)

#-----------------------------------------------------------------------------------------------------------------------
# LIBRARIES
#-----------------------------------------------------------------------------------------------------------------------

find_package(spdlog REQUIRED)

#-----------------------------------------------------------------------------------------------------------------------
# EXECUTABLE
#-----------------------------------------------------------------------------------------------------------------------

set(SOURCES src/audio_player/audio_player.hpp)

add_executable(audio_palyer ${SOURCES} main.cpp)
add_executable(test test.cpp)
target_link_libraries(audio_palyer pulse-simple pulse spdlog::spdlog)
target_link_libraries(test pulse-simple pulse)
