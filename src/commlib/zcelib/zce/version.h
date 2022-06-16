/*
   《回答》 北岛
　　卑鄙是卑鄙者的通行证，
　　高尚是高尚者的墓志铭，
　　看吧，在那镀金的天空中，
　　飘满了死者弯曲的倒影。
　　冰川纪过去了，
　　为什么到处都是冰凌？
　　好望角发现了，
　　为什么死海里千帆相竞？
　　我来到这个世界上，
　　只带着纸、绳索和身影，
　　为了在审判前，
　　宣读那些被判决的声音。
　　告诉你吧，世界
　　我——不——相——信！
　　纵使你脚下有一千名挑战者，
　　那就把我算作第一千零一名。
　　我不相信天是蓝的，
　　我不相信雷的回声，
　　我不相信梦是假的，
　　我不相信死无报应。
　　如果海洋注定要决堤，
　　就让所有的苦水都注入我心中，
　　如果陆地注定要上升，
　　就让人类重新选择生存的峰顶。
　　新的转机和闪闪星斗，
　　正在缀满没有遮拦的天空。
　　那是五千年的象形文字，
　　那是未来人们凝视的眼睛。
*/

#pragma once

//服务器的版本信息，每个版本应该主动改写这个
//改写的途径应该是在Makefile为好

#ifndef ZCE_MAJOR_VER
#define ZCE_MAJOR_VER "2"
#endif 

#ifndef ZCE_MINOR_VER
#define ZCE_MINOR_VER "0"
#endif 

// build num 由CI编译自动修改，手动修改无用
#ifndef ZCE_BUILD_VER
#define ZCE_BUILD_VER "0"
#endif

// svn revision 或者git commit id 由CI编译自动修改，手动修改无用
// svn info --show-item revision
// git rev-parse --short HEAD
#ifndef ZCE_REVISION
#define ZCE_REVISION "0"
#endif

#ifndef ___STRINGIZING
#define ___STRINGIZING(v) #v
#endif

#ifndef _ZCE_LIB_VERNUM_JOIN
#define _ZCE_LIB_VERNUM_JOIN(major,minor,build,revision) ___STRINGIZING(major.minor.build.revision)
#endif

#ifndef _ZCE_LIB_VERSTR_JOIN
#define _ZCE_LIB_VERSTR_JOIN(major,minor,build,revision)  major"."minor"."build"."revision
#endif


#ifndef ZCE_JION_VER_STRING
#define ZCE_JION_VER_STRING  _ZCE_LIB_VERSTR_JOIN(ZCE_MAJOR_VER,ZCE_MINOR_VER,ZCE_BUILD_VER,ZCE_REVISION)
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
#define ZCE_VERSION_DECLARE(name)   #name"Server Version :" ZCE_JION_VER_STRING "  .\n" \
    ZCE_BUILD_TYPE_STRING \
"Compilation Timestamp :" __DATE__ " " __TIME__ " .\n" \
"Copyright :(C) 2008-2012 . My Team (We are best!). All rights reserved. \n"  \
"Author :sail and her firends.\n" \
"Have fun. My brother.  You Can't Write Perfect Software !  Free as in Freedom .\n"
#endif



