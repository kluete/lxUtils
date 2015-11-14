#pragma once

#define JUCE_MODULE_AVAILABLE_juce_core                 1
#define JUCE_MODULE_AVAILABLE_juce_data_structures      1
#define JUCE_MODULE_AVAILABLE_juce_events               1
#define JUCE_MODULE_AVAILABLE_juce_graphics             1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics           1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra            1

#define JUCE_MODULE_AVAILABLE_juce_audio_basics         1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices        1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats        1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors     1
#define JUCE_MODULE_AVAILABLE_juce_audio_utils          1

#ifndef JUCE_STANDALONE_APPLICATION
	#define	JUCE_STANDALONE_APPLICATION		0			// when mixed with WX is NOT stdalone app!
#endif

// #define	JUCE_CATCH_DEPRECATED_CODE_MISUSE		1
#define JUCE_CHECK_MEMORY_LEAKS				1			// fails on (opaque) CoreAudio reader???

#define JUCE_WEB_BROWSER				0
#define JUCE_USE_DIRECTWRITE				0
#define	JUCE_USE_FLAC					1

#ifdef JUCE_LINUX
	#define	JUCE_ALSA				1
	// #define	JUCE_ALSA_LOGGING			1
	// #define JUCE_JACK				1
	#define JUCE_USE_MP3AUDIOFORMAT			1
	#define	JUCE_MODAL_LOOPS_PERMITTED		1
	// #define	JUCE_USE_XRENDER			1
#endif

#ifdef JUCE_WINDOWS
	// #define JUCE_ASIO			1
	#define	JUCE_WASAPI_LOGGING		0
	#define JUCE_WASAPI			0
	#define JUCE_WASAPI_EXCLUSIVE		0
	#define JUCE_DIRECTSOUND		1
	#define	JUCE_USE_WINDOWS_MEDIA_FORMAT	1		// Windows Media Format SDK codecs
	// #define	JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES		1		// (only for MSVC "import" directives)
#endif

#ifdef JUCE_MAC
	// #define	JUCE_QUICKTIME			1
	
	// #define	JUCE_USE_VDSP_FRAMEWORK		0
	// #define	JUCE_PLUGINHOST_AU		0
	// #define	JUCE_PLUGINHOST_VST		0
	// #define	JUCE_PLUGINHOST_VST3		0
#endif


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
