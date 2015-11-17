cmake_minimum_required(VERSION 2.8.11)

project(logger)

include_directories(inc)

file(GLOB core_sources
    src/*.cpp
)

if (LX_WX)
    ADD_SUBDIRECTORY(examples/wx)
endif()

if (LX_JUCE)
    ADD_SUBDIRECTORY(examples/juce)
endif()
