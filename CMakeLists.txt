cmake_minimum_required(VERSION 2.8.11)

project(logger_wx)

include_directories(inc)

if (WITH_WXPATH)
    set(ENV{PATH} ${WITH_WXPATH}:$ENV{PATH})
endif()
unset(WITH_WXPATH CACHE)

set( LX_WX_CONFIG wx-config )

if (UNIX OR MINGW)
    execute_process(COMMAND which ${LX_WX_CONFIG} OUTPUT_VARIABLE WX_TOOL OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT WX_TOOL)
        message(FATAL_ERROR "\nNo functional wx_config script was found in your PATH.\nIs the wxWidgets development package installed?")
    else()
        execute_process(COMMAND sh ${WX_TOOL} --version OUTPUT_VARIABLE WX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(SUBSTRING "${WX_VERSION}" "0" "1" wxMAJOR_VERSION)
        string(SUBSTRING "${WX_VERSION}" "2" "1" wxMINOR_VERSION)
        string(SUBSTRING "${WX_VERSION}" "4" "1" wxRELEASE_NUMBER)
        if ( wxMAJOR_VERSION LESS 3 )
		message(FATAL_ERROR "\nYour wxWidgets version is too old, at least wxWidgets-3.0.0 is required")
        endif()
    endif()
    message("-- wx-config used is: ${WX_TOOL}")
    message("-- wxWidgets version is: ${WX_VERSION}")
    if (NOT APPLE AND NOT MINGW)
        # Is the wx we are using built on gtk2 or 3?
        execute_process(COMMAND ${WX_TOOL} --selected_config OUTPUT_VARIABLE WX_GTK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(SUBSTRING "${WX_GTK_VERSION}" "3" "1" GTK_VERSION)
        message("-- gtk version is: ${GTK_VERSION}")
    endif()
endif (UNIX OR MINGW)

# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.
find_package(wxWidgets COMPONENTS core base std REQUIRED)

# we need wxWidgets flags to be set only for the c++ files, so we do it like this
# by setting the CMAKE_CXX_FLAGS
if ( NOT MINGW )
    execute_process(COMMAND ${LX_WX_CONFIG} --cxxflags OUTPUT_VARIABLE WX_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
    execute_process(COMMAND sh ${LX_WX_CONFIG} --cxxflags OUTPUT_VARIABLE WX_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WX_CXX_FLAGS}")

add_definitions(
    -DLX_WX=1
)

# Linker options
execute_process(COMMAND ${LX_WX_CONFIG} --libs core base std OUTPUT_VARIABLE WX_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ${WX_LD_FLAGS}")

# Define the CXX sources
file(GLOB CXX_SRCS
    src/*.cpp
    examples/wx/main.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -Wall -Wfatal-errors -Wno-parentheses -Wshadow -g -O0 -std=c++14")

add_executable(logger_wx ${CXX_SRCS} ${C_SRCS})

if(GTK2_FOUND)
    target_link_libraries(logger_wx ${LINKER_OPTIONS} ${GTK2_LIBRARIES} ${wxWidgets_LIBRARIES} )
elseif (GTK3_FOUND)
    target_link_libraries(logger_wx ${LINKER_OPTIONS} ${GTK3_LIBRARIES} ${wxWidgets_LIBRARIES} )
else()
    target_link_libraries(logger_wx ${LINKER_OPTIONS} ${wxWidgets_LIBRARIES} )
endif()

