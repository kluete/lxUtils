#pragma once

#define JUCE_MODULE_AVAILABLE_juce_core                 1
#define JUCE_MODULE_AVAILABLE_juce_data_structures      1
#define JUCE_MODULE_AVAILABLE_juce_events               1
#define JUCE_MODULE_AVAILABLE_juce_graphics             1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics           1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra            1

#ifndef JUCE_STANDALONE_APPLICATION
	#define	JUCE_STANDALONE_APPLICATION		0			// when mixed with WX is NOT stdalone app!
#endif

#define JUCE_CHECK_MEMORY_LEAKS				1			// fails on (opaque) CoreAudio reader???

#define JUCE_WEB_BROWSER				0
#define JUCE_USE_DIRECTWRITE				0

#ifndef    JUCE_INCLUDE_ZLIB_CODE
 //#define JUCE_INCLUDE_ZLIB_CODE
#endif

// juce_graphics flags:
#ifndef    JUCE_USE_COREIMAGE_LOADER
 //#define JUCE_USE_COREIMAGE_LOADER
#endif

#ifndef    JUCE_USE_XSHM
 //#define JUCE_USE_XSHM
#endif
#ifndef    JUCE_USE_XRENDER
 //#define JUCE_USE_XRENDER
#endif
#ifndef    JUCE_USE_XCURSOR
 //#define JUCE_USE_XCURSOR
#endif

// nada mas
