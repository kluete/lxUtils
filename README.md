# About lxUtils

These files contain C++14 utilities for:

* template-based sprintf() formatting à la [Bjarne Stroustrup "A Type-Safe printf"][1], see "The C++ Programming Language", 4th edition, section 28.6.1 (page 809)
* free-form log _tags_ (instead of levels) with compile-time string hashing from [Dan Bernstein][2]
* thread-aware logging with normalized timestamps

The logger is somewhat similar to Boost::format except that log levels don't need to be hierarchical; they're a set of freeform _tags_ that can be ANDed, ORed, etc., like in std::unordered_set.  

Log tags are computed at compile-time in constexpr functions, so don't need to be pre-declared (in an enum, say), can be used in any source file or translation unit, without worrying about initialization order (static-, thread_local- or otherwise).

[1]: http://www.stroustrup.com/C++11FAQ.html#variadic-templates
[2]: http://www.cse.yorku.ca/~oz/hash.html

## Headers
* [color.h](inc/lx/color.h) - RGB color definitions for the UI
* [ulog.h](inc/lx/ulog.h) - logger interfaces
* [xstring.h](inc/lx/xstring.h) - sprintf-formatter
* [xutils.h](inc/lx/xutils.h) - timestamps & misc


## Example

The file ./examples/main.cpp implements a wxWidgets-based user interface to generate logs from different threads, display them in color, and dynamically toggle filtering. A secondary file log target is created upstream.  

Binaries build for Clang/libc++ and g++ 4.9.1 with libstdc++.  

The project was created with the [CodeLite](http://www.codelite.org) IDE, which generates a Makefile. There'll be a CMake receipe shortly.


## Compiler Defines

* to bridge with JUCE  

    \#define LX_JUCE 1

* to bridge with wxWidgets  

    \#define LX_WX 1

* to enable off-thread log generation

    \#define LOG_FROM_ASYNC 1


## Formatting

* I use 8-char tabs, not spaces, so there.


## Factoids

* I started writing these for a language-teaching software called "Linguamix", which is where the "lx"-prefix came from.
* source files inevitably end wih the comment
    // nada mas  
  ever since I used a macro-assembler that wouldn't flush the disk cache before executing the build, so on crash my source files would be missing a sector's worth of data (back then I barely spoke Spanish).
* swearwords usually come more naturally to me in French.
