
#ifndef ZCE_LIB_OS_ADAPT_GET_OPT_H_
#define ZCE_LIB_OS_ADAPT_GET_OPT_H_

#include "zce_os_adapt_predefine.h"

//这一段不放在zce_os_adapt_predefine.h里面的原因是因为有的代码也实现了这个函数，和这些全局变量
//所以会引发冲突，所以除非明确你使用这些函数，否则不暴漏他们
#if defined (ZCE_OS_WINDOWS)

//getopt_long函数使用的选项
struct option
{
    //
    const char  *name;
    //
    int          has_arg;
    //
    int         *flag;
    //
    int          val;
};

/* if error message should be printed */
extern int     opterr;
/* index into parent argv vector */
extern int     optind;
/* character checked for validity */
extern int     optopt;
/* argument associated with option */
extern const char   *optarg;
/* reset getopt */
extern int     optreset;

#define no_argument            0
#define required_argument      1
#define optional_argument      2

#endif // #if defined (ZCE_OS_WINDOWS)

namespace ZCE_LIB
{

//optstring string说明
//1.单个字符，表示选项，
//2.单个字符后接一个冒号:,表示该选项后必须跟一个参数。参数紧跟在选项后或者以空格隔开。该参数的指针赋给optarg。
//3 单个字符后跟两个冒号::,表示该选项后必须跟一个参数。参数必须紧跟在选项后不能以空格隔开。该参数的指针赋给optarg。

//还要注意的是默认情况下getopt会重新排列命令行参数的顺序，所以到最后所有不包含选项的命令行参数都排到最后。

//
int getopt(int argc,
           char *const argv[],
           const char *optstring);

//
int getopt_long(int argc,
                char *const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);

//
int getopt_long_only(int argc,
                     char *const argv[],
                     const char *optstring,
                     const struct option *longopts,
                     int *longindex);
}; // end of namespace ZCE_LIB

#endif //ZCE_LIB_OS_ADAPT_GET_OPT_H_
