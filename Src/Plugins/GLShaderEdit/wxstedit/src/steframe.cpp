///////////////////////////////////////////////////////////////////////////////
// Name:        steframe.cpp
// Purpose:     wxSTEditorFrame
// Author:      John Labenski, parts taken from wxGuide by Otto Wyss
// Modified by:
// Created:     11/05/2002
// RCS-ID:
// Copyright:   (c) John Labenski, Otto Wyss
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "steframe.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/stedit/stedit.h"

#include "wx/file.h"
#include "wx/filename.h"  // wxFileName
#include "wx/tokenzr.h"

#include "wx/config.h"    // wxConfigBase
#include "wx/docview.h"   // wxFileHistory

#define STE_MM wxSTEditorMenuManager

#include "../art/pencil16.xpm"
#include "../art/pencil32.xpm"

#ifdef GLI_CHANGES
#include "GLIShaderData.h"
#include "GLIShaders.h"
#include "GLIShaderDebug.h"
#include "ShaderLang.h"

#include "../art/GLIcon16.xpm"

#endif// GLI_CHANGES


//-----------------------------------------------------------------------------
// wxSTEditorFrame
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSTEditorFrame, wxFrame)

BEGIN_EVENT_TABLE(wxSTEditorFrame, wxFrame)
    EVT_MENU_OPEN             (wxSTEditorFrame::OnMenuOpen)
    EVT_MENU                  (wxID_ANY, wxSTEditorFrame::OnMenu)

    //EVT_STE_CREATED           (wxID_ANY, wxSTEditorFrame::OnSTECreatedEvent)
    EVT_STE_STATE_CHANGED     (wxID_ANY, wxSTEditorFrame::OnSTEState)
    EVT_STC_UPDATEUI          (wxID_ANY, wxSTEditorFrame::OnSTCUpdateUI)
    EVT_STE_POPUPMENU         (wxID_ANY, wxSTEditorFrame::OnSTEPopupMenu)

    EVT_STN_PAGE_CHANGED      (wxID_ANY, wxSTEditorFrame::OnNotebookPageChanged)
    EVT_LISTBOX               (ID_STF_FILELISTBOX, wxSTEditorFrame::OnFileListBox)

#ifdef GLI_CHANGES

    EVT_MOVE                  (wxSTEditorFrame::OnMove) 

#endif //GLI_CHANGES

    EVT_CLOSE                 (wxSTEditorFrame::OnClose)
END_EVENT_TABLE()

void wxSTEditorFrame::Init()
{
    m_steNotebook      = NULL;
    m_steSplitter      = NULL;
    m_mainSplitter     = NULL;
    m_sideSplitter     = NULL;
    m_sideNotebook     = NULL;
    m_fileListBox      = NULL;
    m_sideSplitterWin1 = NULL;
    m_sideSplitterWin2 = NULL;

#ifdef GLI_CHANGES

    m_outputTextCtrl   = NULL;
    gliShadersDialog   = NULL;
    glslValidate       = true;

#endif //GLI_CHANGES

}

bool wxSTEditorFrame::Create(wxWindow *parent, wxWindowID id,
                             const wxString& title,
                             const wxPoint& pos, const wxSize& size,
                             long  style,
                             const wxString& name)
{
    m_titleBase = title;

    if (!wxFrame::Create(parent, id, title, pos, size, style, name))
        return false;

#ifdef GLI_CHANGES

    // Set the frame's icons
    wxBitmap bmp(GLIcon16_xpm);
    wxIcon icon;
    icon.CopyFromBitmap(bmp);
    wxIconBundle iconBundle(icon);

   
#else //GLI_CHANGES

    // Set the frame's icons
    wxBitmap bmp(pencil16_xpm);
    wxIcon icon;
    icon.CopyFromBitmap(bmp);
    wxIconBundle iconBundle(icon);
    bmp = wxBitmap(pencil32_xpm);
    icon.CopyFromBitmap(bmp);
    iconBundle.AddIcon(icon);

#endif //GLI_CHANGES    
    
    SetIcons(iconBundle);

    return true;
}

wxSTEditorFrame::~wxSTEditorFrame()
{
    SetSendSTEEvents(false);
    if (GetToolBar() && (GetToolBar() == GetOptions().GetToolBar())) // remove for safety
        GetOptions().SetToolBar(NULL);
    if (GetMenuBar() && (GetMenuBar() == GetOptions().GetMenuBar())) // remove for file history
        GetOptions().SetMenuBar(NULL);
    if (GetStatusBar() && (GetStatusBar() == GetOptions().GetStatusBar()))
        GetOptions().SetStatusBar(NULL);

    // This stuff always gets saved when the frame closes
    wxConfigBase *config = wxConfigBase::Get(false);
    if (config && GetOptions().HasConfigOption(STE_CONFIG_FILEHISTORY))
        GetOptions().SaveFileConfig(*config);

    if (config && GetOptions().HasConfigOption(STE_CONFIG_FINDREPLACE) &&
        GetOptions().GetFindReplaceData())
        GetOptions().GetFindReplaceData()->SaveConfig(*config,
                      GetOptions().GetConfigPath(STE_OPTION_CFGPATH_FINDREPLACE));


#ifdef GLI_CHANGES

    //Shutdown the shader data interface
    GLIShaderData::Shutdown();

#endif// GLI_CHANGES

}
bool wxSTEditorFrame::Destroy()
{
    SetSendSTEEvents(false);
    if (GetToolBar() && (GetToolBar() == GetOptions().GetToolBar())) // remove for safety
        GetOptions().SetToolBar(NULL);
    if (GetMenuBar() && (GetMenuBar() == GetOptions().GetMenuBar())) // remove for file history
        GetOptions().SetMenuBar(NULL);
    if (GetStatusBar() && (GetStatusBar() == GetOptions().GetStatusBar()))
        GetOptions().SetStatusBar(NULL);

    return wxFrame::Destroy();
}
void wxSTEditorFrame::SetSendSTEEvents(bool send)
{
    if (GetEditorNotebook())
        GetEditorNotebook()->SetSendSTEEvents(send);
    else if (GetEditorSplitter())
        GetEditorSplitter()->SetSendSTEEvents(send);
    else if (GetEditor())
        GetEditor()->SetSendSTEEvents(send);
}


#ifdef GLI_CHANGES
    
void wxSTEditorFrame::CreateOptions(const wxSTEditorOptions& options, void* gliWindowHandle)

#else //GLI_CHANGES

void wxSTEditorFrame::CreateOptions( const wxSTEditorOptions& options )

#endif //GLI_CHANGES

{
    m_options = options;
    wxConfigBase *config = wxConfigBase::Get(false);
    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();

    if (steMM && GetOptions().HasFrameOption(STF_CREATE_MENUBAR))
    {
        wxMenuBar *menuBar = GetMenuBar() != NULL ? GetMenuBar() : new wxMenuBar(wxMB_DOCKABLE);
        steMM->CreateMenuBar(menuBar, true);

        if (menuBar)
        {
            SetMenuBar(menuBar);

            if (GetOptions().HasFrameOption(STF_CREATE_FILEHISTORY) && !GetOptions().GetFileHistory())
            {
                // if has file open then we can use wxFileHistory to save them
                wxMenu *menu = NULL;
                if (menuBar->FindItem(wxID_OPEN, &menu))
                {
                    GetOptions().SetFileHistory(new wxFileHistory(9), false);
                    GetOptions().GetFileHistory()->UseMenu(menu);
                    if (config)
                        GetOptions().LoadFileConfig(*config);
                }
            }

            GetOptions().SetMenuBar(menuBar);
        }
    }
    if (steMM && GetOptions().HasFrameOption(STF_CREATE_TOOLBAR))
    {
        wxToolBar* toolBar = (GetToolBar() != NULL) ? GetToolBar() : CreateToolBar();
        steMM->CreateToolBar(toolBar);
        GetOptions().SetToolBar(toolBar);
    }
    if ((GetStatusBar() == NULL) && GetOptions().HasFrameOption(STF_CREATE_STATUSBAR))
    {
        CreateStatusBar(1);
        GetOptions().SetStatusBar(GetStatusBar());
    }
    if (steMM)
    {
        if (GetOptions().HasEditorOption(STE_CREATE_POPUPMENU))
            GetOptions().SetEditorPopupMenu(steMM->CreateEditorPopupMenu(), false);
        if (GetOptions().HasSplitterOption(STS_CREATE_POPUPMENU))
            GetOptions().SetSplitterPopupMenu(steMM->CreateSplitterPopupMenu(), false);
        if (GetOptions().HasNotebookOption(STN_CREATE_POPUPMENU))
            GetOptions().SetNotebookPopupMenu(steMM->CreateNotebookPopupMenu(), false);

#ifdef GLI_CHANGES

        //Do the initial updates to disable all menu items
        steMM->EnableEditorItems(false, NULL, GetMenuBar(), GetToolBar());

#endif //GLI_CHANGES
    }

    if (!m_sideSplitter && GetOptions().HasFrameOption(STF_CREATE_SIDEBAR))
    {
        m_sideSplitter = new wxSplitterWindow(this, ID_STF_SIDE_SPLITTER );
        m_sideSplitter->SetMinimumPaneSize(10);

#ifdef GLI_CHANGES

        m_sideNotebook = new wxNotebook(m_sideSplitter, ID_STF_SIDE_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);

        // Set the splitter to only resize the top window when the main window resizes
        m_sideSplitter->SetSashGravity(1.0);

        //Create a output log
        m_outputTextCtrl = new wxStyledTextCtrl(m_sideNotebook, ID_STF_OUTPUT_WINDOW); 
        m_outputTextCtrl->SetReadOnly(true);
        m_outputTextCtrl->SetMarginWidth(1,0);

        m_sideNotebook->AddPage(m_outputTextCtrl, wxT("Output"));
        
        m_sideSplitterWin1 = m_sideNotebook;

#else //GLI_CHANGES

        m_sideNotebook = new wxNotebook(m_sideSplitter, ID_STF_SIDE_NOTEBOOK);
        m_fileListBox = new wxListBox(m_sideNotebook, ID_STF_FILELISTBOX,
                                wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
        m_sideNotebook->AddPage(m_fileListBox, wxT("Files"));
        m_sideSplitterWin1 = m_sideNotebook;

#endif //GLI_CHANGES

    }

    if (!m_steNotebook && GetOptions().HasFrameOption(STF_CREATE_NOTEBOOK))
    {
        m_mainSplitter = new wxSplitterWindow(m_sideSplitter ? (wxWindow*)m_sideSplitter : (wxWindow*)this, ID_STF_MAIN_SPLITTER);
        m_mainSplitter->SetMinimumPaneSize(10);

        m_steNotebook = new wxSTEditorNotebook(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                               wxCLIP_CHILDREN);
        m_steNotebook->CreateOptions(m_options);

#ifndef GLI_CHANGES        
        (void)m_steNotebook->InsertEditorSplitter(-1, wxID_ANY, GetOptions().GetDefaultFileName(), true);
#endif //GLI_CHANGES

        // update after adding a single page
        m_steNotebook->UpdateAllItems();
        m_mainSplitter->Initialize(m_steNotebook);
        m_sideSplitterWin2 = m_mainSplitter;
    }
    else if (!m_steSplitter && GetOptions().HasFrameOption(STF_CREATE_SINGLEPAGE))
    {
        m_mainSplitter = new wxSplitterWindow(m_sideSplitter ? (wxWindow*)m_sideSplitter : (wxWindow*)this, ID_STF_MAIN_SPLITTER);
        m_mainSplitter->SetMinimumPaneSize(10);

        m_steSplitter = new wxSTEditorSplitter(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
        m_steSplitter->CreateOptions(m_options);
        m_mainSplitter->Initialize(m_steSplitter);
    }
    //else user will set up the rest

    if (GetOptions().HasFrameOption(STF_CREATE_SIDEBAR) && GetSideSplitter() && m_sideSplitterWin1 && m_sideSplitterWin2)
    {
#ifdef GLI_CHANGES
        GetSideSplitter()->SplitHorizontally(m_sideSplitterWin2, m_sideSplitterWin1, GetSideSplitter()->GetClientSize().GetHeight() - 210);        
#else //GLI_CHANGES
        GetSideSplitter()->SplitVertically(m_sideSplitterWin1, m_sideSplitterWin2, 100);
#endif// GLI_CHANGES

    }

#if wxUSE_DRAG_AND_DROP
    SetDropTarget(new wxSTEditorFrameFileDropTarget(this));
#endif //wxUSE_DRAG_AND_DROP

    if (GetOptions().HasConfigOption(STE_CONFIG_FINDREPLACE) && config)
    {
        if (GetOptions().GetFindReplaceData() &&
            !GetOptions().GetFindReplaceData()->HasLoadedConfig())
            GetOptions().GetFindReplaceData()->LoadConfig(*config);
    }

    if (config)
        LoadConfig(*config);

    UpdateAllItems();

    // if we've got an editor let it update gui
    wxSTEditor *editor = GetEditor();
    if (editor)
        editor->UpdateAllItems();

#ifdef GLI_CHANGES

    //Create the dialog
    gliShadersDialog = new GLIShaders(this);

    //Create the debug window
    gliDebugDialog = new GLIShaderDebug(this);

    //Initialize the shader data interface
    if(GLIShaderData::Init(this, gliWindowHandle))
    {
      //Refresh the shader list to set the initial list
      GLIShaderData::GetInstance()->RefreshShaderList();

      //Show the window
      gliShadersDialog->UpdateWindowPosition();
      gliShadersDialog->Show(true);
    }

    //gliDebugDialog->Show(true);

#endif// GLI_CHANGES

}

wxSTEditor *wxSTEditorFrame::GetEditor(int page) const
{
    wxSTEditorSplitter *splitter = GetEditorSplitter(page);
    return splitter ? splitter->GetEditor() : (wxSTEditor*)NULL;
}

wxSTEditorSplitter *wxSTEditorFrame::GetEditorSplitter(int page) const
{
    return GetEditorNotebook() ? GetEditorNotebook()->GetEditorSplitter(page) : m_steSplitter;
}

void wxSTEditorFrame::ShowAboutDialog()
{
    wxString msg;
    msg.Printf( _T("Welcome to ") STE_VERSION_STRING _T(".\n")
                _T("Using the Scintilla editor, http://www.scintilla.org\n")
                _T("and the wxWidgets library, http://www.wxwidgets.org.\n")
                _T("Written by John Labenski, Otto Wyss.\n\n")
                _T("Compiled with %s."), wxVERSION_STRING);

    // FIXME - or test wxFileConfig doesn't have ClassInfo is this safe?
    //if ((wxFileConfig*)wxConfigBase::Get(false))
    //    msg += wxT("\nConfig file: ")+((wxFileConfig*)wxConfigBase::Get(false))->m_strLocalFile;

    wxMessageBox(msg, _T("About editor"), wxOK|wxICON_INFORMATION, this);
}

void wxSTEditorFrame::UpdateAllItems()
{
    UpdateItems(GetOptions().GetEditorPopupMenu(), GetOptions().GetMenuBar(),
                                                   GetOptions().GetToolBar());
    UpdateItems(GetOptions().GetNotebookPopupMenu());
    UpdateItems(GetOptions().GetSplitterPopupMenu());
}
void wxSTEditorFrame::UpdateItems(wxMenu *menu, wxMenuBar *menuBar, wxToolBar *toolBar)
{
    if (!menu && !menuBar && !toolBar) return;

    STE_MM::DoEnableItem(menu, menuBar, toolBar, ID_STF_SHOW_SIDEBAR, GetSideSplitter() != NULL);
    STE_MM::DoCheckItem(menu, menuBar, toolBar, ID_STF_SHOW_SIDEBAR, (GetSideSplitter() != NULL) && GetSideSplitter()->IsSplit());
}

void wxSTEditorFrame::LoadConfig(wxConfigBase &config, const wxString &configPath_)
{
    wxString configPath = wxSTEditorOptions::FixConfigPath(configPath_, false);

    if (GetMenuBar() && GetMenuBar()->FindItem(ID_STF_SHOW_SIDEBAR))
    {
        long val = 0;
        if (config.Read(configPath + wxT("/ShowSidebar"), &val))
        {
            wxSTEditorMenuManager::DoCheckItem(NULL, GetMenuBar(), NULL,
                                               ID_STF_SHOW_SIDEBAR, val != 0);
            // send fake event to HandleEvent
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_STF_SHOW_SIDEBAR);
            evt.SetInt(int(val));
            HandleMenuEvent(evt);
        }
    }

    wxString str;
    if (config.Read(configPath + wxT("/FrameSize"), &str))
    {
        wxRect rect = GetRect();
        long lrect[4] = { rect.x, rect.y, rect.width, rect.height };
        wxArrayString arrStr = wxStringTokenize(str, wxT(","));
        if (arrStr.GetCount() == 4u)
        {
            for (size_t n = 0; n < 4; n++)
                arrStr[n].ToLong(&lrect[n]);

            wxRect cfgRect((int)lrect[0], (int)lrect[1], (int)lrect[2], (int)lrect[3]);
            cfgRect = cfgRect.Intersect(wxGetClientDisplayRect());

            if ((cfgRect != rect) && (cfgRect.width>=100) && (cfgRect.height>=100))
                SetSize(cfgRect);
        }
    }
}
void wxSTEditorFrame::SaveConfig(wxConfigBase &config, const wxString &configPath_)
{
    wxString configPath = wxSTEditorOptions::FixConfigPath(configPath_, false);
    if (GetMenuBar() && GetMenuBar()->FindItem(ID_STF_SHOW_SIDEBAR))
    {
        wxString val = GetMenuBar()->IsChecked(ID_STF_SHOW_SIDEBAR) ? wxT("1") : wxT("0");
        config.Write(configPath + wxT("/ShowSidebar"), val);
    }

    wxRect rect = GetRect();
    if ((rect.x>=0) && (rect.y>=0) && (rect.width>=100) && (rect.height>=100))
       config.Write(configPath + wxT("/FrameSize"), wxString::Format(wxT("%d,%d,%d,%d"), rect.x, rect.y, rect.width, rect.height));
}

void wxSTEditorFrame::OnNotebookPageChanged(wxNotebookEvent &WXUNUSED(event))
{
    wxSTEditor *editor = GetEditor();
    wxString title = m_titleBase;
    wxSTEditorMenuManager *steMM = GetOptions().GetMenuManager();

    if (editor)
    {
        if ( steMM && !steMM->HasEnabledEditorItems())
            steMM->EnableEditorItems(true, NULL, GetMenuBar(), GetToolBar());

        wxString modified = editor->IsModified() ? wxT("*") : wxT("");
        title += wxT(" - ") + modified + editor->GetFileName();
    }
    else
    {
        if (steMM && steMM->HasEnabledEditorItems())
            steMM->EnableEditorItems(false, NULL, GetMenuBar(), GetToolBar());
    }

    UpdateFileListBox();
    SetTitle(title);
}

void wxSTEditorFrame::OnFileListBox(wxCommandEvent &event)
{
    if (GetEditorNotebook())
        GetEditorNotebook()->SetSelection(event.GetInt());
}

void wxSTEditorFrame::UpdateFileListBox()
{
    wxNotebook *noteBook = GetEditorNotebook();
    wxListBox *listBox = GetFileListBox();
    if (!listBox || !noteBook)
        return;

    int listCount = listBox->GetCount();
    int pageCount = noteBook->GetPageCount();

    while (listCount > pageCount)
    {
        listBox->Delete(listCount-1);
        listCount = listBox->GetCount();
    }
    while (listCount < pageCount)
    {
        listBox->Append(wxEmptyString);
        listCount = listBox->GetCount();
    }

    wxString value;

    for (int n = 0; n < listCount; n++)
    {
        value = noteBook->GetPageText(n);
        if (value != listBox->GetString(n))
            listBox->SetString(n, value);
    }

    if (noteBook->GetSelection() >= 0)
        listBox->Select(noteBook->GetSelection());
}

void wxSTEditorFrame::OnSTECreated(wxCommandEvent &event)
{
    event.Skip();
    UpdateFileListBox();
}

void wxSTEditorFrame::OnSTEPopupMenu(wxSTEditorEvent &event)
{
    event.Skip();
    wxSTEditor *editor = event.GetEditor();
    UpdateItems(editor->GetOptions().GetEditorPopupMenu());
}

void wxSTEditorFrame::OnSTEState(wxSTEditorEvent &event)
{
    event.Skip();
    wxSTEditor *editor = event.GetEditor();

    if ( event.HasStateChange(STE_FILENAME | STE_MODIFIED) )
    {
        wxString modified = editor->IsModified() ? wxT("*") : wxT("");
        SetTitle(m_titleBase + wxT(" - ") + modified + event.GetString());

        UpdateFileListBox();
        if (event.HasStateChange(STE_FILENAME) && GetOptions().GetFileHistory())
        {
            if (wxFileExists(event.GetString()))
                GetOptions().GetFileHistory()->AddFileToHistory( event.GetString() );
        }
    }
}

void wxSTEditorFrame::OnSTCUpdateUI(wxStyledTextEvent &event)
{
    event.Skip();
    if (!GetStatusBar()) // nothing to do
        return;

    wxStyledTextCtrl* editor = (wxStyledTextCtrl*)event.GetEventObject();
    int pos   = editor->GetCurrentPos();
    int line  = editor->GetCurrentLine() + 1; // start at 1
    int lines = editor->GetLineCount();
    int col   = editor->GetColumn(pos) + 1;   // start at 1
    int chars = editor->GetLength();

    wxString txt = wxString::Format(wxT("Line %6d of %6d, Col %4d, Chars %6d  "), line, lines, col, chars);
    txt += editor->GetOvertype() ? wxT("[OVR]") : wxT("[INS]");

    if (txt != GetStatusBar()->GetStatusText()) // minor flicker reduction
        SetStatusText(txt, 0);
}

void wxSTEditorFrame::OnMenuOpen(wxMenuEvent &WXUNUSED(event))
{
    // might need to update the preferences, the rest should be ok though
    wxSTEditor *editor = GetEditor();
    if (editor && GetMenuBar())
        editor->UpdateItems(NULL, GetMenuBar());
}

void wxSTEditorFrame::OnMenu(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_OnMenu);
    if (guard.IsInside()) return;

    if (!HandleMenuEvent(event))
        event.Skip();
}

bool wxSTEditorFrame::HandleMenuEvent(wxCommandEvent &event)
{
    wxSTERecursionGuard guard(m_rGuard_HandleMenuEvent);
    if (guard.IsInside()) return false;

    int win_id  = event.GetId();
    wxSTEditor *editor = GetEditor();

    // menu items that the frame handles before children
    switch (win_id)
    {
        case ID_STE_SAVE_PREFERENCES :
        {
            // we save everything the children do and more
            wxConfigBase *config = wxConfigBase::Get(false);
            if (config)
            {
                SaveConfig(*config, GetOptions().GetConfigPath(STE_OPTION_CFGPATH_FRAME));
                GetOptions().SaveConfig(*config);
            }

            return true;
        }
    }


#ifdef GLI_CHANGES

    //Allow messages even when there is no editor
    if (GetEditorNotebook() && GetEditorNotebook()->HandleMenuEvent(event))
    {
      return true;
    }

#endif //GLI_CHANGES    

    // Try the children to see if they'll handle the event first
    if (editor)
    {

#ifndef GLI_CHANGES

        if (GetEditorNotebook() && GetEditorNotebook()->HandleMenuEvent(event))
            return true;

#endif //GLI_CHANGES

        if (wxDynamicCast(editor->GetParent(), wxSTEditorSplitter) &&
            wxDynamicCast(editor->GetParent(), wxSTEditorSplitter)->HandleMenuEvent(event))
            return true;
        if (editor->HandleMenuEvent(event))
            return true;
    }

    if ((win_id >= wxID_FILE1) && (win_id <= wxID_FILE9))
    {
        if (GetOptions().GetFileHistory())
        {
            wxString fileName = GetOptions().GetFileHistory()->GetHistoryFile(win_id-wxID_FILE1);

            if (GetEditorNotebook())
                GetEditorNotebook()->LoadFile(fileName);
            else if (editor)
                editor->LoadFile(fileName);
        }

        return true;
    }

    switch (win_id)
    {
        case ID_STE_SHOW_FULLSCREEN :
        {
            long style = wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOCAPTION;
            ShowFullScreen(event.IsChecked(), style);
            return true;
        }
        case ID_STF_SHOW_SIDEBAR :
        {
            if (GetSideSplitter() && m_sideSplitterWin1 && m_sideSplitterWin2)
            {
                if (event.IsChecked())
                {
                    if (!GetSideSplitter()->IsSplit())
                    {

#ifdef GLI_CHANGES
                        GetSideSplitter()->SplitHorizontally(m_sideSplitterWin2, m_sideSplitterWin1, GetSideSplitter()->GetClientSize().GetHeight() - 210);
#else  //GLI_CHANGES
                        GetSideSplitter()->SplitVertically(m_sideSplitterWin1, m_sideSplitterWin2, 100);
                        GetSideNotebook()->Show(true);
#endif //GLI_CHANGES
                    }
                }
                else if (GetSideSplitter()->IsSplit())
                    GetSideSplitter()->Unsplit(m_sideSplitterWin1);
            }

            UpdateAllItems();
            return true;
        }
        case wxID_EXIT :
        {
            if (GetEditorNotebook())
            {
                if (!GetEditorNotebook()->QuerySaveIfModified())
                    return true;
            }
            else if (editor && (editor->QuerySaveIfModified(true) == wxCANCEL))
                return true;

            Destroy();
            return true;
        }

#ifdef GLI_CHANGES

        //Compile the current shader
        case ID_STN_COMPILE_SHADER :  
        {
          //If there is an editor
          if(editor)
          {
            // Focus the output window
            ShowSideNotebookWindow(m_outputTextCtrl);

            //Clear the output window
            OutputClear();

            //Get the filename
            wxFileName fullFileName(editor->GetFileName());

            //Compile with GLSL validate if necessary
            if(glslValidate && 
               fullFileName.GetExt().CmpNoCase(wxT("glsl")) == 0) 
            {
              //Validate the shader
              GLSLValidate(editor->GetText()); 
            }

            //Send to GLIntercept if there is a connection
            if(GLIShaderData::GetInstance())
            {
              //Pass to GLIntercept for compiling
              GLIShaderData::GetInstance()->CompileShader(fullFileName.GetFullName(), editor->GetText());
            }
            else
            {
              OutputAppendString("==No active GLIntercept interface==\n");
            }
          }

          return true;
        }


        //Set the GLSL validate flag
        case ID_STN_VALIDATE_SHADER :  
        {
          glslValidate = event.IsChecked();
          return true;
        }

#endif //GLI_CHANGES


        case wxID_ABOUT :
        {
            ShowAboutDialog();
            return true;
        }
        default : break;
    }

    return false;
}

void wxSTEditorFrame::OnClose( wxCloseEvent &event )
{
    int style = event.CanVeto() ? wxYES_NO|wxCANCEL : wxYES_NO;

    if (GetEditorNotebook())
    {
        if (!GetEditorNotebook()->QuerySaveIfModified(style))
        {
            event.Veto(true);
            return;
        }
    }
    else if (GetEditor() && (GetEditor()->QuerySaveIfModified(true, style) == wxCANCEL))
    {
        event.Veto(true);
        return;
    }

    // **** Shutdown so that the focus event doesn't crash the editors ****
    SetSendSTEEvents(false);
    event.Skip();
}

//-----------------------------------------------------------------------------
// wxSTEditorFileDropTarget
//-----------------------------------------------------------------------------
#if wxUSE_DRAG_AND_DROP

bool wxSTEditorFrameFileDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                                const wxArrayString& filenames)
{
    wxCHECK_MSG(m_owner, false, wxT("Invalid drop target"));
    const size_t count = filenames.GetCount();
    if (count == 0)
        return false;

    // see if it has a notebook and use it to load the files
    if (m_owner->GetEditorNotebook())
    {
        wxArrayString files = filenames;
        m_owner->GetEditorNotebook()->LoadFiles(&files);
    }
    else if (m_owner->GetEditor())
        m_owner->GetEditor()->LoadFile(filenames[0]);

    return true;
}

#endif //wxUSE_DRAG_AND_DROP


#ifdef GLI_CHANGES

///////////////////////////////////////////////////////////////////////////////
//
void wxSTEditorFrame::OutputClear()
{
  //Return now if there is no control
  if(!m_outputTextCtrl)
  {
    return;
  }

  //Clear the output log
  m_outputTextCtrl->SetReadOnly(false);
  m_outputTextCtrl->ClearAll();
  m_outputTextCtrl->SetReadOnly(true);

}

///////////////////////////////////////////////////////////////////////////////
//
void wxSTEditorFrame::OutputAppendString(const wxString &logString)
{
  //Return now if there is no control
  if(!m_outputTextCtrl)
  {
    return;
  }

  //Append the output string
  m_outputTextCtrl->SetReadOnly(false);
  m_outputTextCtrl->AddText(logString);
  
  //Scroll to the last line added
  m_outputTextCtrl->ScrollToLine(m_outputTextCtrl->GetLineCount());
  m_outputTextCtrl->SetReadOnly(true);
}


///////////////////////////////////////////////////////////////////////////////
//
bool wxSTEditorFrame::GLSLValidate(const wxString &src) 
{
  //Only init once
  static bool InitSh = false;
  if(!InitSh)
  {
    ShInitialize();
    InitSh = true;
  }

  //Assign some high limits for compiling
  TBuiltInResource resourceLimits;
  resourceLimits.maxLights        = 32;
  resourceLimits.maxClipPlanes    = 64;
  resourceLimits.maxTextureUnits  = 128;
  resourceLimits.maxTextureCoords = 128;
  resourceLimits.maxVertexAttribs             = 16000;
  resourceLimits.maxVertexUniformComponents   = 16000;
  resourceLimits.maxVaryingFloats             = 16000;
  resourceLimits.maxVertexTextureImageUnits   = 16000;
  resourceLimits.maxCombinedTextureImageUnits = 16000;
  resourceLimits.maxTextureImageUnits         = 16000;
  resourceLimits.maxFragmentUniformComponents = 16000;
  resourceLimits.maxDrawBuffers               = 16000;

  bool shaderTagsFound = false;

  //Flag that the GLSL validate is in use
  OutputAppendString("Validating...\n");

  //Extract the vertex shader part of the string
  wxString vertexShaderSource;
  if(ExtractShaderString("[Vertex Shader]", src, vertexShaderSource))
  {
    //Flag that a tag was found
    shaderTagsFound = true;

    //Compile the shader
    const char *src = vertexShaderSource.c_str();
	  ShHandle vertexCompiler = ShConstructCompiler(EShLangVertex, 0);
    
    OutputAppendString("  [Vertex Shader]...");

    if(!ShCompile(vertexCompiler, &src, 1, EShOptNone, &resourceLimits, 0))
    {
      OutputAppendString(" Error\n");
    }
    else
    {
      OutputAppendString(" Success\n");
    }

    //Output any append log
    const char* info = ShGetInfoLog(vertexCompiler);
    if(info && strlen(info) > 0)
    {
      OutputAppendString(info);
      OutputAppendString("\n");
    }

    ShDestruct(vertexCompiler);
  }

  //Extract the fragment shader part of the string
  wxString fragmentShaderSource;
  if(ExtractShaderString("[Fragment Shader]", src, fragmentShaderSource))
  {
    //Flag that a tag was found
    shaderTagsFound = true;

    //Compile the shader
    const char *src = fragmentShaderSource.c_str();
	  ShHandle fragmentCompiler = ShConstructCompiler(EShLangFragment, 0);

    OutputAppendString("  [Fragment Shader]...");

    if(!ShCompile(fragmentCompiler, &src, 1, EShOptNone, &resourceLimits, 0))
    {
      OutputAppendString(" Error\n");
    }
    else
    {
      OutputAppendString(" Success\n");
    }

    //Output any append log
    const char* info = ShGetInfoLog(fragmentCompiler);
    if(info && strlen(info) > 0)
    {
      OutputAppendString(info);
      OutputAppendString("\n");
    }

    ShDestruct(fragmentCompiler);
  }

  //Tell the user if no tags were found
  if(!shaderTagsFound)
  {
    OutputAppendString("No [Vertex Shader] or [Fragment Shader] tags found.\n");
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
bool wxSTEditorFrame::ExtractShaderString(const wxString &shaderToken, const wxString &shaderSource, wxString &retSource) const
{
  uint lineStart = 0;
  uint lineCount = 1;
  bool startTokenFound = false;
  uint shaderStringStart =0;
  uint shaderStartLine   =1;
 

  //Loop for all characters in the string
  uint i = 0;
  for( ; i<shaderSource.length(); i++)
  {
    //Check if the starting character is a open bracket
    if(shaderSource[i] == '[' && lineStart == i)
    {
      if(!startTokenFound)
      {
        //Test for the start token
        if(shaderSource.find(shaderToken.c_str(), i) == (int)i)
        {
          shaderStringStart = i + shaderToken.length();
          startTokenFound = true;

          //Set what line the shader was found on
          shaderStartLine = lineCount;
        }
      }
      else
      {
        //Break where the end token was found
        break;
      }
    }

    //If the character is equal to the new line character,
    // The next character must be on the new line
    if(shaderSource[i] == '\r' || shaderSource[i] == '\n')
    {
      lineStart = i+1;
    }

    //Count the new lines
    if(shaderSource[i] == '\n')
    {
      lineCount++;
    }
  }

  //If the string was not found, return false
  if(!startTokenFound || shaderStringStart >= i)
  {
    return false;
  }

  //Assign the return string
  retSource = shaderSource.substr(shaderStringStart, i - shaderStringStart);

  //Add the line directive to the shader source
  wxString lineDirective;
  lineDirective.Printf("#line %u ",shaderStartLine);

  lineDirective += retSource;
  retSource = lineDirective;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//
void wxSTEditorFrame::OnMove(wxMoveEvent &event)
{
  // If the shaders dialog is up, move with the main window
  if(gliShadersDialog)
  {
    gliShadersDialog->UpdateWindowPosition();
  }

  event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
//
void wxSTEditorFrame::ShowSideNotebookWindow(wxWindow * showWindow)
{
  // If no side notebook, return now
  if(!m_sideNotebook)
  {
    return;
  }

  // Loop for all pages
  for(uint i=0; i<m_sideNotebook->GetPageCount(); i++)
  {
    // Check if the page is the window to be shown
    if(m_sideNotebook->GetPage(i) == showWindow)
    {
      m_sideNotebook->SetSelection(i); 
      return;
    }
  }

  // Window was not in the notebook
}



#endif //GLI_CHANGES


