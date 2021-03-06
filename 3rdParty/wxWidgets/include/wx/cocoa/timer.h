/////////////////////////////////////////////////////////////////////////////
// Name:        timer.h
// Purpose:     Cocoa wxTimer class
// Author:      Ryan Norton
// Id:          $Id$
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __WX_TIMER_H__
#define __WX_TIMER_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "timer.h"
#endif

#include "wx/cocoa/ObjcRef.h"

//-----------------------------------------------------------------------------
// wxTimer
//-----------------------------------------------------------------------------

DECLARE_WXCOCOA_OBJC_CLASS(NSTimer);

class WXDLLEXPORT wxTimer : public wxTimerBase
{
public:
    wxTimer() { Init(); }
    wxTimer(wxEvtHandler *owner, int timerid = -1) : wxTimerBase(owner, timerid)
        { Init(); }
    ~wxTimer();

    virtual bool Start(int millisecs = -1, bool oneShot = false);
    virtual void Stop();

    virtual bool IsRunning() const;

    inline WX_NSTimer GetNSTimer()
    {   return m_cocoaNSTimer; }

protected:
    void Init();

private:
    WX_NSTimer m_cocoaNSTimer;
    static const wxObjcAutoRefFromAlloc<struct objc_object *> sm_cocoaDelegate;

    DECLARE_ABSTRACT_CLASS(wxTimer)
};

#endif // __WX_TIMER_H__
