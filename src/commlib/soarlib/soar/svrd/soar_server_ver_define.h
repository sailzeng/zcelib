

#ifndef SOARING_LIB_SERVER_VERSION_DEFINE_H_
#define SOARING_LIB_SERVER_VERSION_DEFINE_H_

//服务器的版本信息，每个版本应该主动改写这个
//改写的途径应该是在Makefile为好

#ifndef TSS_MAJOR_VER
#define TSS_MAJOR_VER 1
#endif //#ifndef TSS_MAJOR_VER

#ifndef TSS_MINOR_VER
#define TSS_MINOR_VER 0
#endif //#ifndef TSS_MINOR_VER

// build num 由CI编译自动修改，手动修改无用
#ifndef TSS_BUILD_VER
#define TSS_BUILD_VER 0
#endif //#ifndef TSS_BUILD_VER

// svn revision 由CI编译自动修改，手动修改无用
#ifndef TSS_REVISION
#define TSS_REVISION 0
#endif //#ifndef TSS_REVISION

#ifndef _SOARING_LIB_VER_STRINGIZING
#define _SOARING_LIB_VER_STRINGIZING(v) #v
#endif

#ifndef _SOARING_LIB_VER_JOIN
#define _SOARING_LIB_VER_JOIN(major, minor,build, revision) _SOARING_LIB_VER_STRINGIZING(major.minor.build.revision)
#endif

#ifndef TSS_JION_VER_STRING
#define TSS_JION_VER_STRING  _SOARING_LIB_VER_JOIN(TSS_MAJOR_VER,TSS_MINOR_VER,TSS_BUILD_VER,TSS_REVISION)
#endif

#ifndef TSS_COMPILE_VERSION_STRING
#if defined (_DEBUG) || defined DEBUG
#define TSS_COMPILE_VERSION_STRING "Tss is compiled by debug version,DEBUG or _DEBUG is defined.\n"
#elif defined (NDEBUG)
#define TSS_COMPILE_VERSION_STRING "Tss is compiled by release version, NDEBUG is defined.\n"
#else
#define TSS_COMPILE_VERSION_STRING "Tss is compiled by neither release nor debug version, NDEBUG and DEBUG is not defined.\n"
#endif
#endif

#ifndef TSS_SERVER_VER_DECLARE
#define TSS_SERVER_VER_DECLARE   "Server Version :" TSS_JION_VER_STRING "  .\n" \
    TSS_COMPILE_VERSION_STRING \
    "Compilation Timestamp :" __DATE__ " " __TIME__ " .\n" \
    "Copyright :(C) 2008-2012 . Fullsail Team (We are best!). All rights reserved. \n"  \
    "Author :sailzeng and her firends.\n" \
    "Have fun. My brother.  You Can't Write Perfect Software !  Free as in Freedom .\n"
#endif

#endif //#ifndef SOARING_LIB_SERVER_VERSION_DEFINE_H_

