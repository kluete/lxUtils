cmake_minimum_required(VERSION 2.8.11)

project(logger)

include_directories(
    inc
    /home/plaufenb/development/build/wx_dbg_libcxx/lib/wx/include/gtk2-unicode-static-3.1
    /home/plaufenb/development/git/wxTrunk/include

)

add_definitions(
    -DLX_WX=1
    -D_FILE_OFFSET_BITS=64
    -D__WXGTK__
)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  $(shell $(WXCONF_PATH) --debug=yes --libs std)")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} ")

# Define the CXX sources
set ( CXX_SRCS
    src/color.cpp
    src/ulog.cpp
    src/xstring.cpp
    src/xutils.cpp
    examples/wx/main.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -Wall -Wfatal-errors -Wno-parentheses -Wshadow -pthread -g -O0 -std=c++14 -pthread")

add_executable(logger ${CXX_SRCS} ${C_SRCS})

