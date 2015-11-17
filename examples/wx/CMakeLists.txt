if (WXCONF_PATH)
    set(ENV{PATH} ${WXCONF_PATH}:$ENV{PATH})
endif()
unset(WXCONF_PATH CACHE)

set( LX_WX_CONFIG wx-config )

if (UNIX)
    execute_process(COMMAND which ${LX_WX_CONFIG} OUTPUT_VARIABLE WX_TOOL OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT WX_TOOL)
        message(FATAL_ERROR "\nNo functional wx_config script was found in your PATH.\nIs the wxWidgets development package installed?")
    endif()
    message("-- wx-config used is: ${WX_TOOL}")
    if (NOT APPLE)
        # Is the wx we are using built on gtk2 or 3?
        execute_process(COMMAND ${WX_TOOL} --selected_config OUTPUT_VARIABLE WX_GTK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(SUBSTRING "${WX_GTK_VERSION}" "3" "1" GTK_VERSION)
        message("-- gtk version is: ${GTK_VERSION}")
    endif()
endif (UNIX)

# when using MinGW gcc it is essential that 'core' is mentioned before 'base'
find_package(wxWidgets COMPONENTS core base std REQUIRED)

execute_process(COMMAND ${LX_WX_CONFIG} --cxxflags OUTPUT_VARIABLE WX_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WX_CXX_FLAGS}")

# Linker options
execute_process(COMMAND ${LX_WX_CONFIG} --libs core base std OUTPUT_VARIABLE WX_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} ${WX_LD_FLAGS}")

add_definitions(
    -DLX_WX=1
)

# Define the CXX sources
set(CXX_SRCS ${core_sources} main.cpp)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -Wall -Wfatal-errors -Wno-parentheses -Wshadow -g -O0 -std=c++14")

add_executable(logger ${CXX_SRCS} )

if (GTK2_FOUND)
    target_link_libraries(logger ${LINKER_OPTIONS} ${GTK2_LIBRARIES} ${wxWidgets_LIBRARIES} )
elseif (GTK3_FOUND)
    target_link_libraries(logger ${LINKER_OPTIONS} ${GTK3_LIBRARIES} ${wxWidgets_LIBRARIES} )
else()
    target_link_libraries(logger ${LINKER_OPTIONS} ${wxWidgets_LIBRARIES} )
endif()

