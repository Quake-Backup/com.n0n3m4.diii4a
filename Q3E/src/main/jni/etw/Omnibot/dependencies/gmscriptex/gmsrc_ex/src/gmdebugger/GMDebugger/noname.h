///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  1 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __noname__
#define __noname__

// Define WX_GCH in order to support precompiled headers with GCC compiler.
// You have to create the header "wx_pch.h" and include all files needed
// for compile your gui inside it.
// Then, compile it and place the file "wx_pch.h.gch" into the same
// directory that "wx_pch.h".
#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#include <wx/menu.h>
#include <wx/wxscintilla.h>
#ifdef __VISUALC__
#include <wx/link_additions.h>
#endif //__VISUALC__
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/button.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class GMDebuggerFrame
///////////////////////////////////////////////////////////////////////////////
class GMDebuggerFrame : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnDebugTextInput( wxCommandEvent& event ){ OnDebugTextInput( event ); }
		
	
	protected:
		enum
		{
			ID_DEFAULT = wxID_ANY, // Default
			ID_DEBUGGERFRAME = 1000,
			ID_DEBUGGERMENUBAR,
			ID_FILEOPEN,
			ID_FILESAVE,
			ID_FILESAVEAS,
			ID_CONNECT,
			ID_CLOSE,
			ID_DEBUGGERSTATUSBAR,
			ID_DEBUGGERTOOLBAR,
			ID_STEPINTO,
			ID_STEPOVER,
			ID_STEPOUT,
			ID_RESUMECURRENT,
			ID_STOPDEBUGGING,
			ID_TOGGLEBREAKPOINT,
			ID_BREAKALL,
			ID_RESUMEALL,
			ID_BREAKCURRENT,
			ID_CODEVIEW,
			ID_DEBUGTEXTOUT,
			ID_DEBUGTEXTIN,
			ID_GLOBALS,
			ID_GLOBALSSVARIABLELIST,
			ID_AUTOPANEL,
			ID_AUTOSVARIABLELIST,
			ID_THREADLIST,
			ID_CALLSTACK,
		};
		
		wxMenuBar* DebuggerMenuBar;
		wxStatusBar* m_DebuggerStatusBar;
		wxToolBar* m_DebuggerToolbar;
		wxScintilla* m_CodeView;
		wxTextCtrl* m_DebugTextOut;
		wxTextCtrl* m_DebugTextInput;
		wxNotebook* m_VarNotebook;
		wxScrolledWindow* m_GlobalsPanelWindow;
		wxPropertyGrid* m_GlobalsPropGrid;
		wxScrolledWindow* m_AutoPanelWindow;
		wxPropertyGrid* m_AutosPropGrid;
		wxListCtrl* m_ThreadList;
		wxListCtrl* m_Callstack;
		wxPanel* m_panel3;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnDebugTextInput( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		GMDebuggerFrame( wxWindow* parent, int id = ID_DEBUGGERFRAME, wxString title = wxT("Game Monkey Debugger 1.0"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 800,700 ), int style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE|wxMAXIMIZE_BOX|wxMINIMIZE|wxMINIMIZE_BOX|wxSYSTEM_MENU|wxCLIP_CHILDREN|wxTAB_TRAVERSAL );
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class AboutDialog
///////////////////////////////////////////////////////////////////////////////
class AboutDialog : public wxDialog 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnAboutOk( wxCommandEvent& event ){ OnAboutOk( event ); }
		
	
	protected:
		enum
		{
			ID_DEFAULT = wxID_ANY, // Default
			ID_ABOUT_DIALOG = 2000,
			
		};

		wxTextCtrl* m_textCtrl3;
		wxButton* m_AboutBtnOk;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnAboutOk( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		AboutDialog( wxWindow* parent, int id = ID_ABOUT_DIALOG, wxString title = wxT("About"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 400,200 ), int style = 0 );
	
};

#endif //__noname__
