About lxUtils
=============

These files contain C++14 utilities for:

* template-based sprintf() formatting [1]
* free-form logging with compile-time string hashing from  [2]
* thread-aware logging with normalized timestamps

The logger is somewhat similar to Boost::format except that log levels don't need to be hierarchical; they're a set of freeform TAGS that can be ANDed, ORed, etc., like in std::unordered_set. Log tags are computed at compile-time in constexpr functions, so don't need to be pre-declared in an enum, say, and can be used in any source file / translation unit, without worrying about initialization order (static-, thread-local- or otherwise).

I started writing these for a language-teaching software called "Linguamix", which is where the "lx"-prefix came from.

[1] Bjarne Stroustrup "The C++ Programming Language", 4th edition, section 28.6.1: "A Type-Safe printf" (page 809)
    [Stroustrup variadic templates](http://www.stroustrup.com/C++11FAQ.html#variadic-templates)
[2] Daniel Berstein's "djb2" string hasher
    [djb2 hash](http://www.cse.yorku.ca/~oz/hash.html)


## Headers
* [color.h](inc/lx/color.h) - RGB color definitions for the UI
* [ulog.h](inc/lx/ulog.h) - logger header
* [xstring.h](inc/lx/xstring.h) - sprintf-formatter header
* [xutils.h](inc/lx/xutils.h) - time & timestamp header


## Example

The file ./examples/main.cpp implements a wxWidgets-based user interface to generate logs from different threads, display them in color, and dynamically toggle filtering. A secondary file log target is created upstream. Binaries build for Clang/libc++ and g++ 4.9.1 with libstdc++. The project was created with the CodeLite IDE, which generates a Makefile.


## Compiler Defines

to bridge with JUCE
  #define LX_JUCE 1

to bridge with wxWidgets
  #define LX_WX 1

to enable off-thread log generation
  #define LOG_FROM_ASYNC 1
  
