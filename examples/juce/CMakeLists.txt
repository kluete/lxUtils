
include_directories(
    ${JUCE_DIR}
    JuceLibraryCode
    /usr/include/freetype2
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++14 -pthread")

add_definitions(
    -DJUCE_LINUX=1
    -DLX_JUCE=1
)

# Define the CXX sources
file(GLOB CXX_SRCS
    ${core_sources}
    ${JUCE_DIR}/modules/juce_core/juce_core.cpp
    ${JUCE_DIR}/modules/juce_data_structures/juce_data_structures.cpp
    ${JUCE_DIR}/modules/juce_events/juce_events.cpp
    ${JUCE_DIR}/modules/juce_graphics/juce_graphics.cpp
    ${JUCE_DIR}/modules/juce_gui_basics/juce_gui_basics.cpp
    ${JUCE_DIR}/modules/juce_gui_extra/juce_gui_extra.cpp
    main.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -Wall -Wfatal-errors -Wno-parentheses -Wshadow")

# Linker options
set(juce_libs X11 Xext Xinerama dl freetype pthread)
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L/usr/X11R6/lib/ ")

add_executable(juce_logger ${CXX_SRCS} ${C_SRCS})
target_link_libraries(juce_logger ${LINKER_OPTIONS} ${juce_libs} )

