/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_id_to_string.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004年8月2日
* @brief      定义几组IDToString的宏,主要用于错误定义等。
*             命令定义等，
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_ID_TO_STRING_H_
#define ZCE_LIB_ID_TO_STRING_H_

/********************************************************************************
下面这组宏用于定义一组IDToString的Symbol 和String,
DEFIDTOSTRING(ID_STR_001,0x01,"AAAA1111111111111");
使用LOADSTRINGBYID(ID_STR_001)得到定义的字符串
*********************************************************************************/
#ifndef DEFSYMBOLTOSTRING
#define DEFSYMBOLTOSTRING(symbol,smbid,tmpstr) static const unsigned int symbol = smbid;\
    static const char _tmp0_str_##symbol[]=tmpstr
#endif //DEFSYMBOLTOSTRING
#ifndef LOADSTRINGBYID
#define LOADSTRINGBYID(symbol) _tmp0_str_##symbol
#endif //LOADSTRINGBYID

/**********************************************************************************************
下面这组宏用于定义一组enum enum ID,和一组enum ID 对应的字符传.以及一个从中得到字符串的静态函数

BEGINDEFSYMBOL(STTID,)                       宏用于定义STRING TABLE类的名字,其中枚举值得名字
SYMBOLTOID(STR_ID_01,0x1)                    宏用于定义枚举名称和枚举数值
ENDDEFSYMBOL                                 结束类定义
下面这3个宏可以定义在.H中,但是部分编译器不支持,所以最好放入.cpp
BEGINDEFSTRINGTABLE(STT)                     宏定义类函数
IDTOSTRINGDEFINE(STR_ID_01,"0000000000")     宏定义函数的返回部分
ENDDEFSTRINGTABLE(STT)                       结束函数定义
使用函数 STTID::ZLoadString(STR_ID_01);得到字符.用ZLoadString的原因使用LoadString和Win32程序冲突
***********************************************************************************************/
#ifndef BEGINDEFSYMBOL
#define BEGINDEFSYMBOL(stringtable,idtable) class stringtable { \
    public: \
        enum idtable {
#endif //BEGINDEFSYMBOL

#ifndef DEFSYMBOLTOID
#define DEFSYMBOLTOID(symbol,strid)   symbol = strid,
#endif //DEFSYMBOLTOID

#ifndef ENDDEFSYMBOL
#define ENDDEFSYMBOL  }; \
    static void ZLoadString(int _tmp_string_id_,char *return_str); \
    };
#endif //ENDDEFSYMBOL

#ifndef BEGINDEFSTRTABLE
#define BEGINDEFSTRTABLE(stringtable) void stringtable::ZLoadString(int _tmp_string_id_,char* _return_str_) \
    { \
        switch(_tmp_string_id_) \
        {
#endif //BEGINDEFSTRTABLE

#ifndef DEFIDTOSTRING
#define DEFIDTOSTRING(symbol,tmpstr)  case symbol: \
{\
    static const char __tmp_string_##symbol[]=tmpstr; \
    strcpy(_return_str_,__tmp_string_##symbol); \
    break;\
}
#endif //DEFIDTOSTRING

#ifndef ENDDEFSTRTABLE
#define ENDDEFSTRTABLE default : \
{ \
    _return_str_[0] = '\0'; \
    break; \
} \
return; \
    } \
    };
#endif //ENDDEFSTRTABLE

/********************************************************************************
这组宏和上面这组功能几乎一样.
而且不是所有的编译器都支持const char *在.h定义, 算了封杀把.

下面这组宏用于定义一组IDToString的String Table. String Table存放在一个名字空间内.
DEFSTRINGTABLE   宏用于定义名字空间的名字
DEFSYMBOLTOID    宏用于定义标号和ID值的对应关系
DEFINEIDTOSTRING 宏用于定义标号和STRING的对应关系
例子:
DEFSTRINGTABLE(STT)
DEFSYMBOLTOID(STR_001,0x1)
BEGINSTRINGTABLE
DEFINEIDTOSTRING(STR_001,"1111111111")
ENDSTRINGTABLE
使用STT::ZLoadString(ID) 函数得到字符串,STT为定义的namespace

struct STRINGIDRECORD
{
    int       strID;
    const char *pstrRes;
};

#ifndef DEFSTRINGTABLE
#define DEFSTRINGTABLE(stringtable) namespace stringtable { \
enum __symboltable_00_ {
#endif //DEFSTRINGTABLE

#ifndef DEFINESYMBOLTOID
#define DEFINESYMBOLTOID(symbol,strid)   symbol = strid,
#endif //DEFINESYMBOLTOID

#ifndef BEGINSTRINGTABLE
#define BEGINSTRINGTABLE }; \
static const STRINGIDRECORD __strtable_00_[]={
#endif //BEGINSTRINGTABLE

#ifndef DEFINEIDTOSTRING
#define DEFINEIDTOSTRING(symbol,tmpstr)  {symbol,tmpstr},
#endif //DEFINEIDTOSTRING

#ifndef ENDSTRINGTABLE
#define ENDSTRINGTABLE      };\
static const char * ZLoadString( int strid)    \
{ \
unsigned int numstrtable = (sizeof(__strtable_00_) / sizeof(STRINGIDRECORD)); \
for(unsigned int i =0; i< numstrtable; i++)   \
{ \
if(strid == __strtable_00_[i].strID)  \
return __strtable_00_[i].pstrRes;  \
} \
return NULL; \
} \
};
#endif //ENDSTRINGTABLE

*********************************************************************************/

#endif //ZCE_LIB_ID_TO_STRING_H_

