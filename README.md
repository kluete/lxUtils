# About lxUtils

These files contain C++14 utilities for:

* template-based sprintf() formatting à la [Bjarne Stroustrup "A Type-Safe printf"][1], see "The C++ Programming Language", 4th edition, section 28.6.1 (page 809)
* free-form log _tags_ (instead of levels) with compile-time string hashing from [Dan Bernstein][2]
* thread-aware logic with normalized timestamps

The logger is somewhat similar to [Boost::format](http://www.boost.org/doc/libs/1_59_0/libs/format/doc/format.html) but uses the traditional printf() format flags and log levels don't need to be hierarchical; they're a set of freeform _tags_ that can be ANDed, ORed, etc. (like in std::unordered_set).  

Log tags are computed at compile-time in constexpr functions, so don't need to be pre-declared (in an enum, say), can be used in any source file or translation unit, without worrying about initialization order (static-, thread_local- or otherwise).

[1]: http://www.stroustrup.com/C++11FAQ.html#variadic-templates
[2]: http://www.cse.yorku.ca/~oz/hash.html


## Overview

```c++
#include "lx/ulog.h"

using namespace LX;

void	resized() override
{
	uLog("UI"_log, "resized %s to %d x %d", getName(), getWidth(), getHeight());
}

constexpr auto	LIFETIME = "APP_DTOR"_log;

void	shutdown() override
{
	uLog(LIFETIME, "App::shutdown()");
}

	JUCEApplicationImp::~JUCEApplicationImp()
{
	uLog(DTOR, "App::DTOR");
}

```

## Headers

* [ulog.h](inc/lx/ulog.h) - logger interfaces
* [xstring.h](inc/lx/xstring.h) - sprintf-formatter
* [xutils.h](inc/lx/xutils.h) - timestamps & misc
* [color.h](inc/lx/color.h) - RGB color definitions for the UI (optional)

Within these headers, declarations happen within their own namespace _LX_. Any local synonyms to STL types are \#used individually (not in bulk) within the LX namespace, i.e. without polluting the global namespace (see Stroustrup "The C++ Programming Language", 4th ed, Section 14.2.2: "\#using declarations"). 


## Examples

There are UI integration examples for [JUCE](http://www.juce.com) and [wxWidgets](http://www.wxwidgets.org), respectively. The single-source app generates logs from different threads, displays them in color, and dynamically toggles filtering. A file log target is also created upstream.  

Binaries build for Clang/libc++ and g++ 4.9.1 with libstdc++.  

Workspaces were created with the [CodeLite](http://www.codelite.org) IDE, which generates a Makefile. There'll be a CMake recipe shortly.


## Build Configuration

### Environment Variables

* path to JUCE source code (e.g. /usr/local/JUCE)  

    $(JUCE_DIR)

* path to wxWidgets configuration script (e.g. /usr/local/wxWidgets/lib/config/gtk2-unicode-static-3.1)  

    $(WXCONF_PATH)


### Compiler Defines

* to bridge with JUCE  

    \#define LX_JUCE 1

* to bridge with wxWidgets  

    \#define LX_WX 1

* to enable off-thread log generation  

    \#define LOG_FROM_ASYNC 1


## Formatting

* I use 8-char tabs, not spaces. So there.


## Factoids

* I started writing these for a language-teaching software called "Linguamix", which is where the "lx"-prefix came from.
* source files inevitably end wih the comment  
    // nada mas  
  ever since I used a macro-assembler that wouldn't flush the disk cache correctly, so on crash my source files would be missing a sector's worth of data.
* swearwords usually come more naturally to me in French.
