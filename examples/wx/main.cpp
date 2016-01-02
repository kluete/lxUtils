
#include <cassert>
#include <memory>
#include <algorithm>
#include <thread>
#include <future>

#include "wx/wx.h"

#include "lx/ulog.h"
#include "lx/color.h"

#define	LOG_FROM_ASYNC		1

using namespace std;
using namespace	LX;

// UI constants
enum
{
	MENU_ID_QUIT = wxID_EXIT,
	
	BUTTON_ID_1,
	BUTTON_ID_2,
	BUTTON_ID_3,
	BUTTON_ID_QUIT,
};

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
	LOG_DEF_MACRO(APP_INIT,		NIGHT_BLUE),
	
	// client log levels
	LOG_DEF_MACRO(UI_CMD,		GREEN),
	LOG_DEF_MACRO(USER1,		PURPLE),
	LOG_DEF_MACRO(USER2,		BLUE),
	LOG_DEF_MACRO(USER3,		CYAN),
};
#undef LOG_DEF_MACRO

//---- wx Frame ---------------------------------------------------------------

class MyFrame : public wxFrame, public LogSlot
{
// log event (storage for batch processing)
struct log_evt
{
	log_evt(const timestamp_t stamp, const LogLevel level, const string &msg, const int thread_index)
		: m_Stamp(stamp), m_Lvl(level), m_Msg(msg), m_Thread(thread_index)
	{
	}
	
	const timestamp_t	m_Stamp;
	const LogLevel		m_Lvl;
	const string		m_Msg;
	const int		m_Thread;
};

public:
	MyFrame()
		: wxFrame(NULL, wxID_ANY, "freeform log", wxDefaultPosition, wxSize(800, 600)),
		m_TextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_NOHIDESEL | wxTE_DONTWRAP),
		m_CheckListBox(this, -1),
		m_Button1(this, BUTTON_ID_1, "user 1"),
		m_Button2(this, BUTTON_ID_2, "user 2"),
		m_Button3(this, BUTTON_ID_3, "user 3"),
		m_QuitButton(this, BUTTON_ID_QUIT, "Quit")
	{
		wxMenu *fileMenu = new wxMenu;
		fileMenu->Append(MENU_ID_QUIT, "Exit", "Quit this program");
		
		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "File");
		SetMenuBar(menuBar);
		
		wxFont	ft(wxFontInfo(9).Family(wxFONTFAMILY_MODERN).Encoding(wxFONTENCODING_DEFAULT));
		m_TextCtrl.SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, ft));
		
		auto	&root_log = rootLog::Get();
		
		vector<string>	labels;
		
		for (const auto &it : s_LogLevelDefMap)
		{
			const string	s = it.second.m_Label;
			
			labels.push_back(s);
		}
		
		std::sort(labels.begin(), labels.end());

		for (const string &s : labels)
		{
			const LogLevel	lvl = log_hash(s.c_str());
			
			const bool	f = root_log.IsLevelEnabled(lvl);
			const size_t	index = m_CheckListBox.GetCount();
			
			m_CheckListBox.Append(wxString{s});
			
			m_CheckListBox.Check(index, f);
		}
	
		wxBoxSizer	*text_n_check_h_sizer = new wxBoxSizer(wxHORIZONTAL);
		
		text_n_check_h_sizer->Add(&m_TextCtrl, wxSizerFlags(4).Border(wxALL, 4).Expand());
		text_n_check_h_sizer->Add(&m_CheckListBox, wxSizerFlags(1).Border(wxALL, 4).Expand());
		
		wxBoxSizer	*but_h_sizer = new wxBoxSizer(wxHORIZONTAL);
		
		but_h_sizer->Add(&m_Button1, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button2, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button3, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->AddStretchSpacer(1);
		but_h_sizer->Add(&m_QuitButton, wxSizerFlags(0).Border(wxALL, 4).Expand());
		
		wxBoxSizer	*top_v_sizer =  new wxBoxSizer(wxVERTICAL);
		
		top_v_sizer->Add(text_n_check_h_sizer, 1, wxALL | wxEXPAND, 1);
		top_v_sizer->Add(but_h_sizer, 0, wxALL | wxEXPAND, 1);
		
		SetSizer(top_v_sizer);
		
		Show();
		Centre();
		
		root_log.Connect(this);
		
		uMsg("vanilla log from ui thread");
	}
	
	void	OnClose(wxCloseEvent &e)
	{
		#if LOG_FROM_ASYNC
			// manually disconnect self (ui log receiver) so we can keep logging during class destruction
			// the file log will continue receiving events
			LogSlot::DisconnectSelf();
		#endif
		
		uLog(APP_INIT, "MyFrame::OnClose()");
		
		e.Skip();
	}
	
// user action
	void	OnQuit(wxCommandEvent &e)
	{
		uLog(UI_CMD, "MyFrame::OnQuit()");
		
		Close(true);
	}
	
	void	OnButton1(wxCommandEvent &e)
	{
		uLog(UI_CMD, "MyFrame::OnButton1()");
		
		uLog(USER1, "user1 id = %d", e.GetId());
	}
	
	void	OnButton2(wxCommandEvent &e)
	{
		uLog(UI_CMD, "MyFrame::OnButton2()");

		#if LOG_FROM_ASYNC
			m_Fut = async(std::launch::async, []{this_thread::sleep_for(200ms);uLog(USER2, "user2 in async task");});
		#else
			uLog(USER2, "user2 from default thread");
		#endif
	}
	
	void	OnButton3(wxCommandEvent &e)
	{
		uLog(UI_CMD, "MyFrame::OnButton3()");

		#if LOG_FROM_ASYNC
			m_Fut = async(std::launch::async, []{this_thread::sleep_for(3s);uLog(USER3, "user3 in async nap");});
		#else
			uLog(USER3, "user3 from vanilla");
		#endif
	}
	
	void	OnCheckbox(wxCommandEvent &e)
	{
		const string	s = e.GetString().ToStdString();
		const size_t	ind = e.GetInt();
		const bool	f = m_CheckListBox.IsChecked(ind);
		
		uLog(UI_CMD, "OnCheckbox(ind = %zu, %S = %c)", ind, s, f);
		
		const LogLevel	lvl = log_hash(s.c_str());
		
		// toggle log level on/off
		rootLog::Get().ToggleLevel(lvl, f);
		
		e.Skip();
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
		CallAfter(&MyFrame::DequeueLogs);
	}
	
	void	DequeueLogs(void)
	{
		unique_lock<mutex>	lock(m_QueueMutex);
		
		for (const auto &e : m_LogEvents)
		{
			const string	thread_s = (e.m_Thread > 0) ? xsprintf(" THR[%1d]", e.m_Thread) : "";
			const string	s = xsprintf("%s%s %s\n", e.m_Stamp.str(STAMP_FORMAT::MILLISEC), thread_s, e.m_Msg);
			
			const RGB_COLOR	clr = s_LogLevelDefMap.count(e.m_Lvl) ? s_LogLevelDefMap.at(e.m_Lvl).m_Color : RGB_COLOR::BLACK;
			m_TextCtrl.SetDefaultStyle(wxTextAttr(*Color(clr).ToWxColor()));
			
			m_TextCtrl.AppendText(s);
		}
		
		m_LogEvents.clear();
	}
	
	wxTextCtrl			m_TextCtrl;
	wxCheckListBox			m_CheckListBox;
	wxButton			m_Button1, m_Button2, m_Button3, m_QuitButton;
	
	#if LOG_FROM_ASYNC
		future<void>		m_Fut;
	#endif
	
	mutable mutex			m_QueueMutex;
	vector<log_evt>			m_LogEvents;
	unordered_map<thread::id, int>	m_ThreadIdMap;
	
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	
	EVT_CLOSE(					MyFrame::OnClose)
	
	EVT_MENU(		MENU_ID_QUIT,		MyFrame::OnQuit)
	
	EVT_BUTTON(		BUTTON_ID_1,		MyFrame::OnButton1)
	EVT_BUTTON(		BUTTON_ID_2,		MyFrame::OnButton2)
	EVT_BUTTON(		BUTTON_ID_3,		MyFrame::OnButton3)
	EVT_BUTTON(		BUTTON_ID_QUIT,		MyFrame::OnQuit)
	
	EVT_CHECKLISTBOX(	-1,			MyFrame::OnCheckbox)
	
END_EVENT_TABLE()

//-----------------------------------------------------------------------------

class MyApp : public wxApp
{
public:
	
	MyApp()
	{	
		uLog(APP_INIT, "MyApp::CTOR()");
	}
	
	virtual ~MyApp()
	{	
		uLog(DTOR, "MyApp::DTOR");
	}
	
	bool	OnInit() override
	{
		uLog(APP_INIT, "MyApp::OnInit()");
		
		if (!wxApp::OnInit())        return false;		// error
		
		new MyFrame();
		// (don't mem-manage wx resources)
		
		return true;
	}
	
	int	OnExit() override
	{
		uLog(APP_INIT, "MyApp::OnExit()");
		
		return wxApp::OnExit();
	}
	
	int	OnRun() override
	{
		uLog(APP_INIT, "MyApp::OnRun()");
		
		return wxApp::OnRun();
	}
};

//---- main -------------------------------------------------------------------

int	main(int argc, char* argv[])
{
	rootLog	logImp;
	
	logImp.EnableLevels({FATAL, EXCEPTION, ERROR, WARNING, MSG});
	
	// s_LogImp.EnableLevels({DTOR});
	logImp.EnableLevels({APP_INIT});
	logImp.EnableLevels({UI_CMD});
	logImp.EnableLevels({USER1, USER2, USER3});
		
	unique_ptr<LogSlot>	file_log(LogSlot::Create(LOG_TYPE_T::STD_FILE, "freeform.log"));
	assert(file_log);
	
	logImp.Connect(file_log.get());
		
	uLog(APP_INIT, "main() file log created, creating wx app");
	
	MyApp		*wx_app = new MyApp();
	(void)wx_app;					// (don't mem-manage wx resources)
	
	uLog(APP_INIT, "main() wx app created, starting wx event loop");
	
	// start wx event loop
	wxEntry(argc, argv);
	
	uLog(APP_INIT, "main(), about to exit main()");
}

// nada mas