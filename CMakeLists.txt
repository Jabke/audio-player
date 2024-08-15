cmake_minimum_required(VERSION 3.27)
project(audio_palyer)

set(CMAKE_CXX_STANDARD 17)

#-----------------------------------------------------------------------------------------------------------------------
# LIBRARIES
#-----------------------------------------------------------------------------------------------------------------------

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
#-----------------------------------------------------------------------------------------------------------------------
# EXECUTABLE
#-----------------------------------------------------------------------------------------------------------------------

set(SOURCES src/audio_player/audio_player.hpp
        src/pulse_client/pulse_client.hpp
        src/pulse_client/pulse_audio.cpp
        src/pulse_client/exceptions.h)

add_executable(audio_palyer ${SOURCES} main.cpp)
target_link_libraries(audio_palyer pulse-simple pulse spdlog::spdlog fmt::fmt)
