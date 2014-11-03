
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars



#ifdef WIN32
#pragma warning(disable : 4702)
#endif //#ifdef WIN32

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <vector>
#include <string>
using namespace std;

#ifdef WIN32
#pragma warning(default : 4702)
#endif //#ifdef WIN32

#ifdef WIN32
#pragma warning(disable : 4003)
#pragma warning(disable : 4616)
#pragma warning(disable : 4298)
#pragma warning(disable : 4192)
#pragma warning(disable : 4278)
#endif //#ifdef WIN32




//用了一个不应该用的
#include <CRange.h>
#include <CWorkbook.h>
#include <CWorkbooks.h>
#include <CWorksheet.h>
#include <CWorksheets.h>
#include <CApplication.h>

//如果出现 error C3505: cannot load type library '{2DF8D04C-5BFA-101B-BDE5-00AA0044DE52}
//请确认一下注册表中的项目
//HKEY_CLASSES_ROOT\TypeLib\{2DF8D04C - 5BFA - 101B - BDE5 - 00AA0044DE52}
//有时候安装过多个版本的EXCEL会有这个问题。

// mso.dll
#import "libid:{2df8d04c-5bfa-101b-bde5-00aa0044de52}"  auto_rename
// vb6ext.olb
#import "libid:{0002e157-0000-0000-c000-000000000046}"  auto_rename
// excel.exe
#import "libid:{00020813-0000-0000-c000-000000000046}"  auto_rename



#ifdef WIN32
#pragma warning(default : 4003)
#pragma warning(default : 4616)
#pragma warning(default : 4298)
#pragma warning(default : 4192)
#pragma warning(default : 4278)
#endif //#ifdef WIN32





#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


