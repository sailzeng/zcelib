/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_id_to_string.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004��8��2��
* @brief      ���弸��IDToString�ĺ�,��Ҫ���ڴ�����ȡ�
*             �����ȣ�
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_ID_TO_STRING_H_
#define ZCE_LIB_ID_TO_STRING_H_

/********************************************************************************
������������ڶ���һ��IDToString��Symbol ��String,
DEFIDTOSTRING(ID_STR_001,0x01,"AAAA1111111111111");
ʹ��LOADSTRINGBYID(ID_STR_001)�õ�������ַ���
*********************************************************************************/
#ifndef DEFSYMBOLTOSTRING
#define DEFSYMBOLTOSTRING(symbol,smbid,tmpstr) static const unsigned int symbol = smbid;\
    static const char _tmp0_str_##symbol[]=tmpstr
#endif //DEFSYMBOLTOSTRING
#ifndef LOADSTRINGBYID
#define LOADSTRINGBYID(symbol) _tmp0_str_##symbol
#endif //LOADSTRINGBYID

/**********************************************************************************************
������������ڶ���һ��enum enum ID,��һ��enum ID ��Ӧ���ַ���.�Լ�һ�����еõ��ַ����ľ�̬����

BEGINDEFSYMBOL(STTID,)                       �����ڶ���STRING TABLE�������,����ö��ֵ������
SYMBOLTOID(STR_ID_01,0x1)                    �����ڶ���ö�����ƺ�ö����ֵ
ENDDEFSYMBOL                                 �����ඨ��
������3������Զ�����.H��,���ǲ��ֱ�������֧��,������÷���.cpp
BEGINDEFSTRINGTABLE(STT)                     �궨���ຯ��
IDTOSTRINGDEFINE(STR_ID_01,"0000000000")     �궨�庯���ķ��ز���
ENDDEFSTRINGTABLE(STT)                       ������������
ʹ�ú��� STTID::ZLoadString(STR_ID_01);�õ��ַ�.��ZLoadString��ԭ��ʹ��LoadString��Win32�����ͻ
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
�������������鹦�ܼ���һ��.
���Ҳ������еı�������֧��const char *��.h����, ���˷�ɱ��.

������������ڶ���һ��IDToString��String Table. String Table�����һ�����ֿռ���.
DEFSTRINGTABLE   �����ڶ������ֿռ������
DEFSYMBOLTOID    �����ڶ����ź�IDֵ�Ķ�Ӧ��ϵ
DEFINEIDTOSTRING �����ڶ����ź�STRING�Ķ�Ӧ��ϵ
����:
DEFSTRINGTABLE(STT)
DEFSYMBOLTOID(STR_001,0x1)
BEGINSTRINGTABLE
DEFINEIDTOSTRING(STR_001,"1111111111")
ENDSTRINGTABLE
ʹ��STT::ZLoadString(ID) �����õ��ַ���,STTΪ�����namespace

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

