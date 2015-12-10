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

#define	LOG_FROM_ASYNC		1

using namespace	std;
using namespace juce;
using namespace LX;

const int	TAB_MARGIN = 4;

const int	BUTT_W = 100;
const int	BUTT_H = 32;
const int	BUTT_MARGIN = 10;

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
	LOG_DEF_MACRO(UI_CMD,		MID_GREEN),
	LOG_DEF_MACRO(USER1,		PURPLE),
	LOG_DEF_MACRO(USER2,		BLUE),
	LOG_DEF_MACRO(USER3,		CYAN),
};
#undef LOG_DEF_MACRO

//---- Main Component ---------------------------------------------------------

class MainComponent : public Component, public ButtonListener, public LogSlot, public ListBoxModel, private AsyncUpdater
{
public:
	MainComponent()
		: m_Button1("user 1"), m_Button2("user 2"), m_Button3("user 3"), m_QuitButton("Quit")
	{
		uLog(APP_INIT, "MainComponent::ctor");
		
		m_TextCtrl.setMultiLine(true);
		m_TextCtrl.setReadOnly(true);
		m_TextCtrl.setCaretVisible(false);
		
		for (const auto &it : s_LogLevelDefMap)
		{
			const string	s = it.second.m_Label;
			
			m_Labels.push_back(s);
		}
		
		std::sort(m_Labels.begin(), m_Labels.end());

		const string	monofont_s = Font::getDefaultMonospacedFontName().toStdString();
		
		m_TextCtrl.setFont(Font(monofont_s, 15, Font::plain));
		addAndMakeVisible(&m_TextCtrl);
		
		m_Checklist.setModel(this);
		m_Checklist.setRowHeight(16);
		addAndMakeVisible(&m_Checklist);
		
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
		
		rootLog::Get().Connect(this);
		
		uMsg("vanilla log from ui thread");
	}
	
	~MainComponent()
	{
		uLog(DTOR, "MainComponent::DTOR");
	}

	void	buttonClicked(Button *clicked) override
	{
		uLog(UI_CMD, "buttonClicked()");
		
		if (clicked == &m_Button1)
		{
			uLog(USER1, "user1");
			return;
		}

		if (clicked == &m_Button2)
		{
			#if LOG_FROM_ASYNC
				m_Fut = async(std::launch::async, []{this_thread::sleep_for(200ms);uLog(USER2, "user2 in async task");});
			#else
				uLog(USER2, "user2 from default thread");
			#endif
			return;
		}

		if (clicked == &m_Button3)
		{
			#if LOG_FROM_ASYNC
				m_Fut = async(std::launch::async, []{this_thread::sleep_for(3s);uLog(USER3, "user3 in async nap");});
			#else
				uLog(USER3, "user3 from vanilla");
			#endif
		
			return;
		}

		if (clicked == &m_QuitButton)
		{
			#if LOG_FROM_ASYNC
				// manually disconnect self (ui log receiver) so we can keep logging during class destruction
				// the file log will continue receiving events
				LogSlot::DisconnectSelf();
			#endif
		
			JUCEApplication::quit();
		}
	}
	
	int	getNumRows(void) override
	{
		return m_Labels.size();
	}
	
	void	paintListBoxItem(int row, Graphics &g, int w, int h, bool selected_f) override
	{
		assert(row < m_Labels.size());
		
		g.fillAll(Colours::white);
		
		const string	level_s = m_Labels[row];
		
		const LogLevel	lvl = log_hash(level_s.c_str());
			
		const bool	enabled_f = rootLog::Get().IsLevelEnabled(lvl);
		
		g.setColour(Color(enabled_f ? RGB_COLOR::SOFT_BLACK : RGB_COLOR::BRIGHT_GREY));
		// g.setFont(m_Font);
		const Rectangle<int>	r(0, 0, w, h);
		g.drawText(String(level_s), r.reduced(4, 2), Justification::left, true/*ellipsize*/);
	}
	
	void	listBoxItemClicked(int row, const MouseEvent &e) override
	{
		uLog(UI_CMD, "listBoxItemClicked(%d)", row);
		
		assert(row < m_Labels.size());
		
		const LogLevel	lvl = log_hash(m_Labels[row].c_str());
		
		auto	&rlog = rootLog::Get();
		
		const bool	enabled_f = rlog.IsLevelEnabled(lvl);
		
		rlog.ToggleLevel(lvl, !enabled_f);
		
		m_Checklist.updateContent();
		m_Checklist.repaint();
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
		
		const int	wL = w * 0.8;
		const int	wR = w - wL;
		
		const Rectangle<int>	rL(0, 0, wL, h - BUTT_H - BUTT_MARGIN);
		const Rectangle<int>	rR(wL, 0, wR, h - BUTT_H - BUTT_MARGIN);
		
		m_TextCtrl.setBounds(rL.reduced(TAB_MARGIN));
		
		m_Checklist.setBounds(rR.reduced(TAB_MARGIN));
		
		const int	base_y = h - (BUTT_H + BUTT_MARGIN);

		m_Button1.setBounds(BUTT_MARGIN, base_y, BUTT_W, BUTT_H);
		m_Button2.setBounds(BUTT_MARGIN + BUTT_W + BUTT_MARGIN, base_y, BUTT_W, BUTT_H);
		m_Button3.setBounds(BUTT_MARGIN + BUTT_W + BUTT_MARGIN + BUTT_W + BUTT_MARGIN, base_y, BUTT_W, BUTT_H);
		
		m_QuitButton.setBounds(w - (BUTT_MARGIN + BUTT_W), base_y, BUTT_W, BUTT_H);
	}
	
	void	paint(Graphics &g) override
	{
		g.fillAll(Color(RGB_COLOR::GTK_GREY));
	}

	TextEditor			m_TextCtrl;
	ListBox				m_Checklist;
	TextButton			m_Button1, m_Button2, m_Button3, m_QuitButton;
	
	vector<string>			m_Labels;
	
	#if LOG_FROM_ASYNC
		future<void>		m_Fut;
	#endif
	
	mutable mutex			m_QueueMutex;
	vector<log_evt>			m_LogEvents;
	unordered_map<thread::id, int>	m_ThreadIdMap;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//-----------------------------------------------------------------------------

class MainWindow : public DocumentWindow
{
public:
	MainWindow()
		: DocumentWindow("freeform log", Colours::lightgrey, DocumentWindow::allButtons, true)
	{
		uLog(APP_INIT, "MainWindow::ctor");
		
		// create instance of main content component & add it to window
		setContentOwned(new MainComponent(), true);
		centreWithSize(getWidth(), getHeight());
		setVisible(true);
		
		setResizable(true, true/*bottom-right resizer*/);
		
		setUsingNativeTitleBar(true);
	}

	void	closeButtonPressed() override
	{
		uLog(APP_INIT, "MainWindow::closeButtonPressed");
		
		JUCEApplication::quit();
	}
};

//---- App IMP ----------------------------------------------------------------

class jApp : public JUCEApplication
{
public:
	jApp()
	{
		m_RootLog.EnableLevels({DTOR});
		m_RootLog.EnableLevels({APP_INIT});
		m_RootLog.EnableLevels({UI_CMD});
		m_RootLog.EnableLevels({USER1, USER2, USER3});

		m_FileLog.reset(LogSlot::Create(LOG_TYPE_T::STD_FILE, "freeform.log"));
		m_RootLog.Connect(m_FileLog.get());
	}

	void	initialise(const String &commandLine) override
	{
		uLog(APP_INIT, "jApp::initialise");
		
		m_MainWindow = new MainWindow();
	}
	
	void	shutdown() override
	{
		uLog(APP_INIT, "jApp::shutdown");
		
		m_MainWindow = nullptr;
	}

	const String	getApplicationName() override
	{
		return "freeform log on JUCE";
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

	rootLog				m_RootLog;
	unique_ptr<LogSlot>		m_FileLog;
	ScopedPointer<MainWindow>	m_MainWindow;
};

START_JUCE_APPLICATION(jApp)

// nada mas
