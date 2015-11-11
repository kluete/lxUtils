
#include <cassert>
#include <memory>
#include <tuple>
#include <utility>
#include <functional>
#include <sstream>
#include <thread>
#include <deque>

#include "wx/wx.h"

#include "lx/ulog.h"

#include "lx/xcolor.h"

using namespace std;
using namespace	LX;

#define LOG_MACRO(arg)	arg = #arg##_log

enum ImpLogLevel : LogLevel
{
	LOG_MACRO(APP_INIT),
	LOG_MACRO(USER_CMD),
};	

#undef LOG_MACRO


static const
unordered_map<LogLevel, RGB_COLOR>	s_LogLevelToColorLUT
{
	// common
	{FATAL,			RGB_COLOR::RED},
	{ERROR,			RGB_COLOR::RED},
	{LX_EXCEPTION,		RGB_COLOR::BLUE},
	{WARNING,		RGB_COLOR::ORANGE},
	{MSG,			RGB_COLOR::BLACK},
	{DTOR,			RGB_COLOR::BROWN},
	
	{APP_INIT,		RGB_COLOR::NIGHT_BLUE},
	{USER_CMD,		RGB_COLOR::GREEN},
};

enum
{
	Minimal_Quit = wxID_EXIT,
	
	Butt_1,
	Butt_2,
	Butt_3,
	
	Butt_quit,
};

struct log_struct
{
	log_struct(const timestamp_t stamp_ms, const LogLevel level, const string &msg, const int thread_index)
		: m_Stamp(stamp_ms), m_Lvl(level), m_Msg(msg), m_ThreadIndex(thread_index)
	{
	}
	
	const timestamp_t	m_Stamp;
	const LogLevel		m_Lvl;
	const string		m_Msg;
	const int		m_ThreadIndex;
};

//---- wx Frame ---------------------------------------------------------------

class MyFrame : public wxFrame, public LogSlot
{
public:
	MyFrame(const wxString &title)
		: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
		m_ThreadID(this_thread::get_id()),
		m_TextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_NOHIDESEL | wxTE_DONTWRAP),
		m_Button1(this, Butt_1, "button1", wxDefaultPosition, wxDefaultSize),
		m_Button2(this, Butt_2, "button2", wxDefaultPosition, wxDefaultSize),
		m_Button3(this, Butt_3, "button3", wxDefaultPosition, wxDefaultSize),
		m_QuitButton(this, Butt_quit, "Quit", wxDefaultPosition, wxDefaultSize)
	{
		
		CreateStatusBar(1);
		
		InitMenus();
		
		wxFont	ft(wxFontInfo(9).Family(wxFONTFAMILY_MODERN).Encoding(wxFONTENCODING_DEFAULT));
		m_TextCtrl.SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, ft));
	
		wxBoxSizer	*but_h_sizer =  new wxBoxSizer(wxHORIZONTAL);
		
		but_h_sizer->Add(&m_Button1, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button2, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button3, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->AddStretchSpacer(1);
		but_h_sizer->Add(&m_QuitButton, wxSizerFlags(0).Border(wxALL, 4).Expand());
		
		wxBoxSizer	*top_v_sizer =  new wxBoxSizer(wxVERTICAL);
		
		top_v_sizer->Add(&m_TextCtrl, 1, wxALL | wxEXPAND, 1);
		top_v_sizer->Add(but_h_sizer, 0, wxALL | wxEXPAND, 1);
		
		SetSizer(top_v_sizer);
		
		SetStatusText("Welcome to uLog example!");
		
		Show();
	}
	
	virtual ~MyFrame()
	{
		uLog(DTOR, "MyFrame::DTOR");
	}
	
	void	OnClose(wxCloseEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnClose()");
		
		e.Skip();
	}
	
// user action
	void	OnQuit(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnQuit()");
		
		Close(true);
	}
	
	void	OnButton1(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton1()");
	}
	
	void	OnButton2(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton2()");
	}
	
	void	OnButton3(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton3()");
	}
	
private:
	
	void	InitMenus(void)
	{
		wxMenu *fileMenu = new wxMenu;
		fileMenu->Append(Minimal_Quit, "Exit", "Quit this program");
		
		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "File");
		SetMenuBar(menuBar);
	}
	
	void	LogAtLevel(const timestamp_t stamp, const LogLevel level, const string &msg) override
	{
		unique_lock<mutex>	lock(m_ThreadMutex);
		
		const auto	tid = this_thread::get_id();
		if (tid != m_ThreadID)
		{
			// don't log from other thread or wx BOMBS!
			if (!m_ThreadIndexMap.count(tid))	m_ThreadIndexMap.emplace(tid, m_ThreadIndexMap.size() + 1);
			
			const int	thread_index = m_ThreadIndexMap.at(tid);
			
			m_ThreadLogs.emplace_back(stamp, level, msg, thread_index);
			
			CallAfter(&MyFrame::DequeueLogs);
			
			return;
		}
		
		DequeueLogs();
		
		const wxString	s(xsprintf("%s %s\n", xtimestamp_str(stamp), msg));
		
		SetLevelColor(level);
		
		m_TextCtrl.AppendText(wxString{s});
		
		// (don't log from inside logger!)
	}
	
	void	DequeueLogs(void)
	{
		for (const auto &e : m_ThreadLogs)
		{
			SetLevelColor(e.m_Lvl);
			
			const string	s = xsprintf("%s THR[%1d] %s\n", xtimestamp_str(e.m_Stamp), e.m_ThreadIndex, e.m_Msg);
			
			m_TextCtrl.AppendText(s);
		}
		
		m_ThreadLogs.clear();
	}
	
	void	SetLevelColor(const LogLevel lvl)
	{
		const RGB_COLOR	clr = s_LogLevelToColorLUT.count(lvl) ? s_LogLevelToColorLUT.at(lvl) : RGB_COLOR::BLACK;
	
		const wxColor	wx_clr = Color(clr).ToWxColor();
	
		m_TextCtrl.SetDefaultStyle(wxTextAttr(wx_clr));
	}
	
	const thread::id		m_ThreadID;
	wxTextCtrl			m_TextCtrl;
	wxButton			m_Button1, m_Button2, m_Button3, m_QuitButton;
	
	mutable mutex			m_ThreadMutex;
	vector<log_struct>		m_ThreadLogs;
	unordered_map<thread::id, int>	m_ThreadIndexMap;
	
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	
	EVT_CLOSE(				MyFrame::OnClose)
	
	EVT_MENU(	Minimal_Quit,		MyFrame::OnQuit)
	
	EVT_BUTTON(	Butt_1,			MyFrame::OnButton1)
	EVT_BUTTON(	Butt_2,			MyFrame::OnButton2)
	EVT_BUTTON(	Butt_3,			MyFrame::OnButton3)
	EVT_BUTTON(	Butt_quit,		MyFrame::OnQuit)
	
END_EVENT_TABLE()

//-----------------------------------------------------------------------------

class MyApp : public wxApp
{
public:
	
	MyApp(rootLog &rlog)
		: m_RootLog(rlog), m_TopFrame(nil)
	{	
		uLog(MSG, "MyApp::CTOR()");
	}
	
	virtual ~MyApp()
	{	
		uLog(DTOR, "MyApp::DTOR");
	}
	
	bool	OnInit() override
	{
		uLog(MSG, "MyApp::OnInit()");
		
		if (!wxApp::OnInit())        return false;
		
		MyFrame	*tf = nil;
		
		try
		{
			tf = new MyFrame("logger wx");
		}
		catch (...)
		{
			assert(0);
		}
		
		m_RootLog.Connect(tf);
		
		// tf->SetSize(r.x(), r.y(), r.w(), r.h());
		
		tf->Show(true);
		
		m_TopFrame = tf;
		
		return true;
	}
	
	int	OnExit() override
	{
		uLog(DTOR, "MyApp::OnExit()");
		
		return wxApp::OnExit();
	}
	
	int	OnRun() override
	{
		uLog("APP_INIT", "MyApp::OnRun()");
		
		return wxApp::OnRun();
	}
		
private:
	
	rootLog		&m_RootLog;
	MyFrame		*m_TopFrame;		// do NOT mem-manage wxFrame or will be deleted twice
};

//---- main -------------------------------------------------------------------

int	main(int argc, char* argv[])
{
	static rootLog		s_LogImp;
	
	s_LogImp.EnableLevels({FATAL, LX_EXCEPTION, ERROR, WARNING, MSG});
	
	s_LogImp.EnableLevels({"APP_INIT"_log});
	// s_LogImp.EnableLevels({DTOR});
	s_LogImp.EnableLevels({USER_CMD});
		
	/*unique_ptr<ddtLog>*/auto	file_log(rootLog::MakeLogType(LOG_TYPE_T::STD_FILE, "gitp4.log"));
	s_LogImp.Connect(file_log);
		
	uLog("APP_INIT", "main() file log created, creating wx app");
	
	MyApp		*wx_app = new MyApp(s_LogImp);
	(void)wx_app;
	
	uLog("APP_INIT", "main() wx app created, starting wx event loop");
	
	// start wx event loop
	wxEntry(argc, argv);
	
	uLog("APP_INIT", "main(), about to exit main()");
}

// nada mas