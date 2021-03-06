/////////////////////////////////////////////////////////////////////////////
// Name:        spinbutt.cpp
// Purpose:     wxSpinCtrl
// Author:      Robert
// Modified by: Mark Newsam (Based on GTK file)
// RCS-ID:      $Id$
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "spinctrl.h"
#endif

#include "wx/wxprec.h"

#if wxUSE_SPINCTRL

#include "wx/spinbutt.h"
#include "wx/spinctrl.h"
#include "wx/textctrl.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the focus rect around a text may have 4 pixels in each direction
// we handle these problems right now in an extended vis region of a window
static const wxCoord TEXTBORDER = 4 ;
// the margin between the text control and the spin
static const wxCoord MARGIN = 8 - TEXTBORDER;

// ----------------------------------------------------------------------------
// wxSpinCtrlText: text control used by spin control
// ----------------------------------------------------------------------------

class wxSpinCtrlText : public wxTextCtrl
{
public:
    wxSpinCtrlText(wxSpinCtrl *spin, const wxString& value)
        : wxTextCtrl(spin , -1, value, wxDefaultPosition, wxSize(40, -1))
    {
        m_spin = spin;
        
        // remove the default minsize, the spinctrl will have one instead
        SetSizeHints(-1,-1);
    }

protected:
    void OnTextChange(wxCommandEvent& event)
    {
        int val;
        if ( m_spin->GetTextValue(&val) )
        {
            m_spin->GetSpinButton()->SetValue(val);

            // Send event that the text was manually changed
            wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, m_spin->GetId());
            event.SetEventObject(m_spin);
            event.SetString(m_spin->GetText()->GetValue());
            event.SetInt(val);

            m_spin->GetEventHandler()->ProcessEvent(event);
        }

        event.Skip();
    }

    bool ProcessEvent(wxEvent &event)
    {
        // Hand button down events to wxSpinCtrl. Doesn't work.
        if (event.GetEventType() == wxEVT_LEFT_DOWN && m_spin->ProcessEvent( event ))
            return TRUE;

        return wxTextCtrl::ProcessEvent( event );
    }

private:
    wxSpinCtrl *m_spin;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxSpinCtrlText, wxTextCtrl)
    EVT_TEXT(-1, wxSpinCtrlText::OnTextChange)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// wxSpinCtrlButton: spin button used by spin control
// ----------------------------------------------------------------------------

class wxSpinCtrlButton : public wxSpinButton
{
public:
    wxSpinCtrlButton(wxSpinCtrl *spin, int style)
        : wxSpinButton(spin )
    {
        m_spin = spin;
        SetWindowStyle(style | wxSP_VERTICAL);

        // TODO: The spin button gets truncated a little bit due to size
        // differences so change it's default size a bit.  SMALL still gets a
        // bit truncated, but MINI seems to be too small...  Readdress this
        // when the textctrl issues are all sorted out.
        //SetWindowVariant(wxWINDOW_VARIANT_SMALL);

        // remove the default minsize, the spinctrl will have one instead
        SetSizeHints(-1,-1);
    }

protected:
    void OnSpinButton(wxSpinEvent& eventSpin)
    {
      m_spin->SetTextValue(eventSpin.GetPosition());

      wxCommandEvent event(wxEVT_COMMAND_SPINCTRL_UPDATED, m_spin->GetId());
      event.SetEventObject(m_spin);
      event.SetInt(eventSpin.GetPosition());

      m_spin->GetEventHandler()->ProcessEvent(event);
    }

private:
    wxSpinCtrl *m_spin;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxSpinCtrlButton, wxSpinButton)
    EVT_SPIN(-1, wxSpinCtrlButton::OnSpinButton)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxSpinCtrl, wxControl)
    
// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxSpinCtrl creation
// ----------------------------------------------------------------------------

void wxSpinCtrl::Init()
{
    m_text = NULL;
    m_btn = NULL;
}

bool wxSpinCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        int min,
                        int max,
                        int initial,
                        const wxString& name)
{
    m_macIsUserPane = true;
    if ( !wxControl::Create(parent, id, pos, size, style,
                            wxDefaultValidator, name) )
    {
        return FALSE;
    }

    // the string value overrides the numeric one (for backwards compatibility
    // reasons and also because it is simpler to satisfy the string value which
    // comes much sooner in the list of arguments and leave the initial
    // parameter unspecified)
    if ( !value.empty() )
    {
        long l;
        if ( value.ToLong(&l) )
            initial = l;
    }

    wxSize csize = size ;
    m_text = new wxSpinCtrlText(this, value);
    m_btn = new wxSpinCtrlButton(this, style);
    
    m_btn->SetRange(min, max);
    m_btn->SetValue(initial);

    if ( size.x == -1 ){
        csize.x = m_text->GetSize().x + MARGIN + m_btn->GetSize().x ;
    }

    if ( size.y == -1 ) {
	    csize.y = m_text->GetSize().y + 2 * TEXTBORDER ; //allow for text border highlights
	    if ( m_btn->GetSize().y > csize.y )
	        csize.y = m_btn->GetSize().y ;
    }
    
    //SetSize(csize);
    
    //MacPostControlCreate(pos, csize);
    SetInitialBestSize(csize);

    return TRUE;
}

wxSpinCtrl::~wxSpinCtrl()
{
    // delete the controls now, don't leave them alive even though they would
    // still be eventually deleted by our parent - but it will be too late, the
    // user code expects them to be gone now
    delete m_text;
    m_text = NULL ;
    delete m_btn;
    m_btn = NULL ;
}

// ----------------------------------------------------------------------------
// geometry
// ----------------------------------------------------------------------------

wxSize wxSpinCtrl::DoGetBestSize() const
{
    if (!m_btn || !m_text)
        return GetSize();
    
    wxSize sizeBtn = m_btn->GetBestSize(),
           sizeText = m_text->GetBestSize();

    sizeText.y += 2 * TEXTBORDER ;
    sizeText.x += 2 * TEXTBORDER ;

    int height;
    if (sizeText.y > sizeBtn.y)
        height = sizeText.y;
    else
        height = sizeBtn.y;
        
    return wxSize(sizeBtn.x + sizeText.x + MARGIN, height );
}

void wxSpinCtrl::DoMoveWindow(int x, int y, int width, int height)
{
    // position the subcontrols inside the client area
    wxSize sizeBtn = m_btn->GetSize();
    wxSize sizeText = m_text->GetSize();
    
    wxControl::DoMoveWindow(x, y, width, height);

    wxCoord wText = width - sizeBtn.x - MARGIN - 2 * TEXTBORDER;
    
    m_text->SetSize(TEXTBORDER, (height - sizeText.y) / 2, wText, -1);
    m_btn->SetSize(0 + wText + MARGIN + 2 * TEXTBORDER , (height - sizeBtn.y) / 2 , -1, -1 );
}

// ----------------------------------------------------------------------------
// operations forwarded to the subcontrols
// ----------------------------------------------------------------------------

bool wxSpinCtrl::Enable(bool enable)
{
    if ( !wxControl::Enable(enable) )
        return FALSE;
    return TRUE;
}

bool wxSpinCtrl::Show(bool show)
{
    if ( !wxControl::Show(show) )
        return FALSE;
    return TRUE;
}

void wxSpinCtrl::SetFocus()
{
    if ( m_text != NULL) {
        m_text->SetFocus();
    }
}

// ----------------------------------------------------------------------------
// value and range access
// ----------------------------------------------------------------------------

bool wxSpinCtrl::GetTextValue(int *val) const
{
    long l;
    if ( !m_text->GetValue().ToLong(&l) )
    {
        // not a number at all
        return FALSE;
    }

    if ( l < GetMin() || l > GetMax() )
    {
        // out of range
        return FALSE;
    }

    *val = l;

    return TRUE;
}

int wxSpinCtrl::GetValue() const
{
    return m_btn ? m_btn->GetValue() : 0;
}

int wxSpinCtrl::GetMin() const
{
    return m_btn ? m_btn->GetMin() : 0;
}

int wxSpinCtrl::GetMax() const
{
    return m_btn ? m_btn->GetMax() : 0;
}

// ----------------------------------------------------------------------------
// changing value and range
// ----------------------------------------------------------------------------

void wxSpinCtrl::SetTextValue(int val)
{
    wxCHECK_RET( m_text, _T("invalid call to wxSpinCtrl::SetTextValue") );

    m_text->SetValue(wxString::Format(_T("%d"), val));

    // select all text
    m_text->SetSelection(0, -1);

    // and give focus to the control!
    // m_text->SetFocus();    Why???? TODO.
}

void wxSpinCtrl::SetValue(int val)
{
    wxCHECK_RET( m_btn, _T("invalid call to wxSpinCtrl::SetValue") );

    SetTextValue(val);

    m_btn->SetValue(val);
}

void wxSpinCtrl::SetValue(const wxString& text)
{
    wxCHECK_RET( m_text, _T("invalid call to wxSpinCtrl::SetValue") );

    long val;
    if ( text.ToLong(&val) && ((val > INT_MIN) && (val < INT_MAX)) )
    {
        SetValue((int)val);
    }
    else // not a number at all or out of range
    {
        m_text->SetValue(text);
        m_text->SetSelection(0, -1);
    }
}

void wxSpinCtrl::SetRange(int min, int max)
{
    wxCHECK_RET( m_btn, _T("invalid call to wxSpinCtrl::SetRange") );

    m_btn->SetRange(min, max);
}

void wxSpinCtrl::SetSelection(long from, long to)
{
    // if from and to are both -1, it means (in wxWidgets) that all text should
    // be selected
    if ( (from == -1) && (to == -1) )
    {
        from = 0;     
    }
    m_text->SetSelection(from, to);
}     

#endif // wxUSE_SPINCTRL
