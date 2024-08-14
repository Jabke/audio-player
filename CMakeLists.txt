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
target_link_libraries(audio_palyer pulse-simple pulse spdlog::spdlog)
