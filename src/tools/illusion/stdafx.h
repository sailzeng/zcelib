
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

// 在WINDOWS下和POSIX标准兼容的宏，VS2003以下版本如何，我没有测试，2003以后，Windows对于很多代码宏会使用"_"前缀，
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_WARNINGS  1

// 用于告诉VC++，我不强迫使用_s类型的API，_CRT_SECURE_NO_DEPRECATE是_CRT_SECURE_NO_WARNINGS的老版本
#define _CRT_SECURE_NO_WARNINGS   1
#define _CRT_SECURE_NO_DEPRECATE  1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

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



#include "zce_predefine.h"
#include "zce_trace_log_msg.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_string.h"
#include "zce_sqlite_db_handler.h"
#include "zce_sqlite_conf_table.h"



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


