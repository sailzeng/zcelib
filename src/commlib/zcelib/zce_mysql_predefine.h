/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MYSQL类的一些基础定义，最开始，我希望同时保有断言，返回值，和异常3种模式，
*             结果多年以来，一直还是使用返回值的方法，异常与我无爱。
* 
* @details    这居然是刚来腾讯时写的代码，好像那时候大家推荐用的数据库还是MYSQL 3.23，
*             时间过的飞快，你的执行效率远远赶不上时间的流逝的速度。
* 
* @note
* 
*/

#ifndef ZCE_LIB_MYSQL_DB_DEFINE_H_
#define ZCE_LIB_MYSQL_DB_DEFINE_H_

//默认的选择还是没有选择异常和断言，而是选择了返回错误

//
#define ER_BOUND_OVER_FLOW     3001
#define ER_DATA_CONVERT_ERROR  3002
#define ER_FIELD_TYPE_ERROR    3003

const char STR_BOUND_OVER_FLOW[]    =  "Result set is NULL , or access overflow bound data,or parameter error.";
const char STR_DATA_CONVERT_ERROR[] =  "Field data is NULL, or Convert fail,or parameter error.";
const char STR_FIELD_TYPE_ERROR[]   =  "Could exist field convert error. Please Check.";

//INITBUFSIZE =10K
#define INITBUFSIZE 1024*64

// OK
enum ZCELIB_EXPORT MYSQLRETURN
{
    MYSQL_RETURN_OK   = 0,
    MYSQL_RETURN_FAIL = -1,
};

class ZCELIB_EXPORT MySQLDBException
{
public:
    const unsigned int  m_ErrorID;
    const char         *m_ErrorStr;
public:

    MySQLDBException(unsigned int erid, const char *erstr):
        m_ErrorID(erid),
        m_ErrorStr(erstr)
    {
    };
    ~MySQLDBException()
    {
    };
    MySQLDBException &operator =(MySQLDBException &others)
    {
        if (&others != this)
        {
            memcpy(this, &others, sizeof(MySQLDBException));
        }

        return *this;
    }
};

//MYSQLCONNECTCHECK,MYSQLCOMMANDCHECK,MYSQLRESULTCHECK,FIELDCONVERTCHECK 用于检查错误,
//根据外部宏的定义，可以用于抛出异常，或者返回错误 MYSQL_RETURN_FAIL

//MYSQLCONNECTCHECK 用在检查 ZCE_Mysql_Connect 类中的错误,

//MYSQLCOMMANDCHECK 用在检查 ZCE_Mysql_Command 类中的错误,

//MYSQLRESULTCHECK检查错误,返回MYSQL_RETURN_FAIL,或者抛出异常,或者什么也不做
//MYSQLRESULTCHECK 用在检查 ZCE_Mysql_Result 类中的错误,

//FIELDCONVERTCHECK 用在检查数据转换(sscanf)错误

//检查发生错误时在代码中使用异常
#if defined(MYSQLUSEEXCEPTION)

#define MYSQLCONNECTCHECK(f)  if(!(f)) \
    {\
        MySQLDBException sqlexp(get_error_no(),get_error_message()); \
        throw sqlexp;\
    }

#define MYSQLCOMMANDCHECK(f)  if(!(f)) \
    {\
        MySQLDBException sqlexp(get_error_no(),get_error_message());\
        throw sqlexp; \
    }

#define MYSQLSTMTCMDCHECK(f)  if(!(f)) \
    {\
        MySQLDBException sqlexp(get_error_no(),get_error_message());\
        throw sqlexp; \
    }

#define MYSQLRESULTCHECK(f)   if(!(f))\
    {\
        MySQLDBException sqlexp(ER_BOUND_OVER_FLOW,STR_FIELD_TYPE_ERROR);\
        throw sqlexp; \
    }

#define FIELDCONVERTCHECK(f,g,h)   if((f))\
    {\
        if(sscanf(f,g,h) <= 0) \
        {\
            MySQLDBException sqlexp1(ER_DATA_CONVERT_ERROR,STR_DATA_CONVERT_ERROR);\
            throw sqlexp1; \
        }\
    }

#define CHECKFIELDTYPE(f)  if(!(f))\
    {\
        MySQLDBException sqlexp(ER_FIELD_TYPE_ERROR,STR_BOUND_OVER_FLOW);\
        throw sqlexp; \
    }

//检查发生错误时在代码中使用断言
#elif defined(MYSQLUSEASSERT) && defined(DEBUG)

#define MYSQLCONNECTCHECK(f)  if(!(f)) ZCE_ASSERT(false)

#define MYSQLCOMMANDCHECK(f)  if(!(f)) ZCE_ASSERT(false)

#define MYSQLSTMTCMDCHECK(f)  if(!(f)) ZCE_ASSERT(false)

#define MYSQLRESULTCHECK(f)   if(!(f)) ZCE_ASSERT(false)

#define FIELDCONVERTCHECK(f,g,h)   if((f))\
    {\
        if(sscanf(f,g,h) <= 0) \
        {\
            ZCE_ASSERT(false); \
        }\
    }

#define CHECKFIELDTYPE(f)  if(!(f)) ZCE_ASSERT(false)

//在代码中不使用异常和断言,默认情况下，
#else //defined(USEMYSQLEXCEPTION)

#define MYSQLCONNECTCHECK(f)  if(!(f)) \
    { \
        return MYSQL_RETURN_FAIL; \
    }

#define MYSQLCOMMANDCHECK(f)  if(!(f)) \
    { \
        return MYSQL_RETURN_FAIL;  \
    }

#define MYSQLSTMTCMDCHECK(f)  if(!(f)) \
    { \
        return MYSQL_RETURN_FAIL; \
    }

#define FIELDCONVERTCHECK(f,g,h)   if((f)) \
    { \
        sscanf(f,g,h); \
    }

#define CHECKFIELDTYPE(f)         ((void)0)

//如果在非DEBUG版本,你对速度有疯狂的爱好,并且你自己肯定保证
//确认空值和边界检查,你用这个定义这个宏 MYSQLNOCHECK

#if defined(MYSQLNOCHECK) && !defined(DEBUG)

#define MYSQLRESULTCHECK(f)       ((void)0)

#else //defined(NO_MYSQLCHECK) && !defined(DEBUG)

#define MYSQLRESULTCHECK(f)   if(!(f)) \
    { \
        return MYSQL_RETURN_FAIL; \
    }

#endif //defined(NOMYSQLCHECK) && !defined(DEBUG)

#endif //defined(USEMYSQLEXCEPTION)

#endif //ZCE_LIB_MYSQL_DB_DEFINE_H_

