// ulog JUCE-implementation

#include <cassert>
#include <memory>
#include <algorithm>
#include <thread>
#include <future>
#include "JuceHeader.h"

#include "lx/ulog.h"
#include "lx/xutils.h"
#include "lx/color.h"

using namespace	std;
using namespace juce;
using namespace LX;

const int	BUTT_W = 100;
const int	BUTT_H = 32;
const int	MARGIN = 10;

// log level/ID definition (label, color) -------------------------------------

struct log_def
{
	log_def(const string &label, const RGB_COLOR &clr)
		: m_Label(label), m_Color(clr)
	{}
	
	log_def(const log_def&) = default;
	log_def& operator=(const log_def&) = default;
	
	const string	m_Label;
	const RGB_COLOR	m_Color;
	// could also store hash but is just as well recomputed
};

// log event (storage for batch processing)

struct log_evt
{
	log_evt(const timestamp_t stamp_ms, const LogLevel level, const string &msg, const int thread_index)
		: m_Stamp(stamp_ms), m_Lvl(level), m_Msg(msg), m_Thread(thread_index)
	{
	}
	
	const timestamp_t	m_Stamp;
	const LogLevel		m_Lvl;
	const string		m_Msg;
	const int		m_Thread;
};

//---- declare/hash log levels at compile time --------------------------------

#define CLIENT_LOG_MACRO(arg)	arg = #arg##_log

enum ImpLogLevel : LogLevel
{
	CLIENT_LOG_MACRO(APP_INIT),
	CLIENT_LOG_MACRO(UI_CMD),
	CLIENT_LOG_MACRO(USER1),
	CLIENT_LOG_MACRO(USER2),
	CLIENT_LOG_MACRO(USER3),
};	
#undef CLIENT_LOG_MACRO

//---- Log level definition table ---------------------------------------------

#define LOG_DEF_MACRO(t, clr)	{t, log_def(#t, RGB_COLOR::clr)}

static const
unordered_map<LogLevel, log_def>	s_LogLevelDefMap
{
	// core / built-in log levels
	LOG_DEF_MACRO(FATAL,		NIGHT_RED),
	LOG_DEF_MACRO(ERROR,		RED),
	LOG_DEF_MACRO(EXCEPTION,	BLUE),
	LOG_DEF_MACRO(WARNING,		ORANGE),
	LOG_DEF_MACRO(MSG,		BLACK),
	LOG_DEF_MACRO(DTOR,		BROWN),
	
	// client log levels
	LOG_DEF_MACRO(APP_INIT,		NIGHT_BLUE),
	LOG_DEF_MACRO(UI_CMD,		GREEN),
	LOG_DEF_MACRO(USER1,		PURPLE),
	LOG_DEF_MACRO(USER2,		BLUE),
	LOG_DEF_MACRO(USER3,		CYAN),
};
#undef LOG_DEF_MACRO

//---- Main Component ---------------------------------------------------------

class MainComponent : public Component, public ButtonListener, public LogSlot, private AsyncUpdater
{
public:
	MainComponent(rootLog &root_log)
		: Component("MainComponent"),
		m_Button1("user 1"), m_Button2("user 2"), m_Button3("user 3"), m_QuitButton("Quit")
	{
		m_TextCtrl.setMultiLine(true);
		m_TextCtrl.setReadOnly(true);
		m_TextCtrl.setCaretVisible(false);
		
		const string	monofont_s = Font::getDefaultMonospacedFontName().toStdString();
		
		m_TextCtrl.setFont(Font(monofont_s, 13, Font::plain));
		addAndMakeVisible(&m_TextCtrl);
		
		addAndMakeVisible(&m_Button1);
		m_Button1.addListener(this);

		addAndMakeVisible(&m_Button2);
		m_Button2.addListener(this);

		addAndMakeVisible(&m_Button3);
		m_Button3.addListener(this);

		addAndMakeVisible(&m_QuitButton);
		m_QuitButton.addListener(this);

		setSize(800, 600);
		setVisible(true);
		
		root_log.Connect(this);
	}

	void	buttonClicked(Button *clicked) override
	{
		if (clicked == &m_Button1)
		{
			uLog(USER1, "button1");
			return;
		}

		if (clicked == &m_Button2)
		{
			uLog(USER2, "button2");
			return;
		}

		if (clicked == &m_Button3)
		{
			uLog(USER3, "button3");
			return;
		}

		if (clicked == &m_QuitButton)
		{
			JUCEApplication::quit();
		}
	}
	
private:

	void	LogAtLevel(const timestamp_t stamp, const LogLevel level, const string &msg) override
	{
		unique_lock<mutex>	lock(m_QueueMutex);
		
		const auto	tid = this_thread::get_id();
		
		if (!m_ThreadIdMap.count(tid))	m_ThreadIdMap.emplace(tid, m_ThreadIdMap.size());
			
		const int	thread_index = m_ThreadIdMap.at(tid);
		
		// queue
		m_LogEvents.emplace_back(stamp, level, msg, thread_index);
		
		// trigger dequeue on main thread
		triggerAsyncUpdate();
	}
	
	void	handleAsyncUpdate(void) override
	{
		unique_lock<mutex>	lock(m_QueueMutex);
		
		m_TextCtrl.moveCaretToEnd();
		
		for (const auto &e : m_LogEvents)
		{
			const string	thread_s = (e.m_Thread > 0) ? xsprintf(" THR[%1d]", e.m_Thread) : "";
			const string	s = xsprintf("%s%s %s\n", xtimestamp_str(e.m_Stamp), thread_s, e.m_Msg);
			
			const RGB_COLOR	clr = s_LogLevelDefMap.count(e.m_Lvl) ? s_LogLevelDefMap.at(e.m_Lvl).m_Color : RGB_COLOR::BLACK;
			
			m_TextCtrl.setColour(TextEditor::textColourId, Color(clr).ToJuceColor());
			
			m_TextCtrl.insertTextAtCaret(s);
		}
		
		m_LogEvents.clear();
	}

	void	resized() override
	{
		const int	w = getWidth();
		const int	h = getHeight();
		
		const Rectangle<int>	r(0, 0, w, h - BUTT_H - MARGIN);
		
		m_TextCtrl.setBounds(r.reduced(MARGIN));
		
		const int	base_y = h - (BUTT_H + MARGIN);

		m_Button1.setBounds(MARGIN, base_y, BUTT_W, BUTT_H);
		m_Button2.setBounds(MARGIN + BUTT_W + MARGIN, base_y, BUTT_W, BUTT_H);
		m_Button3.setBounds(MARGIN + BUTT_W + MARGIN + BUTT_W + MARGIN, base_y, BUTT_W, BUTT_H);
		
		m_QuitButton.setBounds(w - (MARGIN + BUTT_W), base_y, BUTT_W, BUTT_H);
	}

	TextEditor	m_TextCtrl;
	TextButton	m_Button1, m_Button2, m_Button3, m_QuitButton;
	
	mutable mutex			m_QueueMutex;
	vector<log_evt>			m_LogEvents;
	unordered_map<thread::id, int>	m_ThreadIdMap;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//-----------------------------------------------------------------------------

class MainWindow : public DocumentWindow
{
public:
	MainWindow(rootLog &root_log)
		: DocumentWindow("freeform log", Colours::lightgrey, DocumentWindow::allButtons, true)
	{
		// create instance of main content component & add it to window
		setContentOwned(new MainComponent(root_log), true);
		centreWithSize(getWidth(), getHeight());
		setVisible(true);
		
		setResizable(true, true/*bottom-right resizer*/);
		
		setUsingNativeTitleBar(true);
	}

	void	closeButtonPressed() override
	{
		JUCEApplication::quit();
	}
};

//---- App IMP ----------------------------------------------------------------

class JUCEHelloWorldApplication : public JUCEApplication
{
public:
	JUCEHelloWorldApplication()
	{
		m_LogImp.EnableLevels({FATAL, ERROR, WARNING, MSG});
		m_FileLog.reset(LogSlot::Create(LOG_TYPE_T::STD_FILE, "freeform.log"));
		m_LogImp.Connect(m_FileLog.get());
		
		m_LogImp.EnableLevels({FATAL, EXCEPTION, ERROR, WARNING, MSG});
		// s_LogImp.EnableLevels({DTOR});
		
		m_LogImp.EnableLevels({UI_CMD});
		m_LogImp.EnableLevels({USER1, USER2, USER3});
	}

	void	initialise(const String &commandLine) override
	{
		m_MainWindow = new MainWindow(m_LogImp);
		
		m_MainWindow->toFront(true/*get focus*/);
	}
	
	void	shutdown() override
	{
		m_MainWindow = nullptr;
	}

	const String	getApplicationName() override
	{
		return "Hello JUCE!";
	}

	const String	getApplicationVersion() override
	{
		return "1.0.0";
	}

	bool	moreThanOneInstanceAllowed() override
	{
		return false;
	}

	void	anotherInstanceStarted(const String& commandLine) override
	{
	}

private:

	rootLog				m_LogImp;
	unique_ptr<LogSlot>		m_FileLog;
	ScopedPointer<MainWindow>	m_MainWindow;
};

START_JUCE_APPLICATION(JUCEHelloWorldApplication)

// nada mas
