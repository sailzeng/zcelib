
#ifndef ZCE_LIB_OS_ADAPT_GET_OPT_H_
#define ZCE_LIB_OS_ADAPT_GET_OPT_H_

#include "zce_os_adapt_predefine.h"

//��һ�β�����zce_os_adapt_predefine.h�����ԭ������Ϊ�еĴ���Ҳʵ�����������������Щȫ�ֱ���
//���Ի�������ͻ�����Գ�����ȷ��ʹ����Щ���������򲻱�©����
#if defined (ZCE_OS_WINDOWS)

//getopt_long����ʹ�õ�ѡ��
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

namespace zce
{

//optstring string˵��
//1.�����ַ�����ʾѡ�
//2.�����ַ����һ��ð��:,��ʾ��ѡ�������һ������������������ѡ�������Կո�������ò�����ָ�븳��optarg��
//3 �����ַ��������ð��::,��ʾ��ѡ�������һ���������������������ѡ������Կո�������ò�����ָ�븳��optarg��

//��Ҫע�����Ĭ�������getopt���������������в�����˳�����Ե�������в�����ѡ��������в������ŵ����

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
}; // end of namespace zce

#endif //ZCE_LIB_OS_ADAPT_GET_OPT_H_
