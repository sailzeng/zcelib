
// illusion.h : main header file for the illusion application
//
#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CillusionApp:
// See illusion.cpp for the implementation of this class
//

class CillusionApp : public CWinAppEx
{
public:
    CillusionApp();


    // Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    // Implementation
protected:
    HMENU  m_hMDIMenu;
    HACCEL m_hMDIAccel;

public:
    UINT  m_nAppLook;
    BOOL  m_bHiColorIcons;

    virtual void PreLoadState();
    virtual void LoadCustomState();
    virtual void SaveCustomState();

    afx_msg void OnAppAbout();
    afx_msg void OnFileNew();
    DECLARE_MESSAGE_MAP()
};

extern CillusionApp theApp;
