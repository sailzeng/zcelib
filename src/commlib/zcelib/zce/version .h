#pragma once

//服务器的版本信息，每个版本应该主动改写这个
//改写的途径应该是在Makefile为好

#ifndef ZCE_MAJOR_VER
#define ZCE_MAJOR_VER 1
#endif 

#ifndef ZCE_MINOR_VER
#define ZCE_MINOR_VER 0
#endif //#ifndef ZCE_MINOR_VER

// build num 由CI编译自动修改，手动修改无用
#ifndef ZCE_BUILD_VER
#define ZCE_BUILD_VER 0
#endif //#ifndef ZCE_BUILD_VER

// svn revision 由CI编译自动修改，手动修改无用
#ifndef ZCE_REVISION
#define ZCE_REVISION 0
#endif //#ifndef ZCE_REVISION

#ifndef _SOARING_LIB_VER_STRINGIZING
#define _SOARING_LIB_VER_STRINGIZING(v) #v
#endif

#ifndef _SOARING_LIB_VER_JOIN
#define _SOARING_LIB_VER_JOIN(major,minor,build,revision) _SOARING_LIB_VER_STRINGIZING(major.minor.build.revision)
#endif

#ifndef ZCE_JION_VER_STRING
#define ZCE_JION_VER_STRING  _SOARING_LIB_VER_JOIN(TSS_MAJOR_VER,TSS_MINOR_VER,TSS_BUILD_VER,TSS_REVISION)
#endif

#ifndef ZCE_BUILD_TYPE_STRING
#if defined (_DEBUG) || defined DEBUG
#define ZCE_BUILD_TYPE_STRING "This is compiled by debug version,DEBUG or _DEBUG is defined.\n"
#elif defined (NDEBUG)
#define ZCE_BUILD_TYPE_STRING "This is compiled by release version, NDEBUG is defined.\n"
#else
#define ZCE_BUILD_TYPE_STRING "This is compiled by neither release nor debug version, NDEBUG and DEBUG is not defined.\n"
#endif
#endif

#ifndef ZCE_VERSION_DECLARE
#define ZCE_VERSION_DECLARE(name)   "Server Version :" ZCE_JION_VER_STRING "  .\n" \
    ZCE_BUILD_TYPE_STRING
    "Compilation Timestamp :" __DATE__ " " __TIME__ " .\n" \
    "Copyright :(C) 2008-2012 . My Team (We are best!). All rights reserved. \n"  \
    "Author :sail and her firends.\n" \
    "Have fun. My brother.  You Can't Write Perfect Software !  Free as in Freedom .\n"
#endif



