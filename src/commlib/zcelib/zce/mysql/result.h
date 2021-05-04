/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/mysql/result.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月24日
* @brief      MYSQL数据库结果集的封装
*
* @details    设计思路如下，我不想搞太多类，其实准确说应该还有一层封装，列封装在OO
*             的角度看起会更加完美一些，
*             但是我不喜欢太麻烦了，一般人也会直接从结果集处理玩所有的结果，
*             而MYSQL的函数，（不知道5.0增加函数没有，我以4.0的CPI做的设计），是通过
*             一个mysql_fetch_row函数得到当前行，你必须一行行的黑醋栗
*             所以我设计成主动帮你记录当前行的，当前列，你通过fetch_row_next，将当前行+1，
*             然后你可以通过 >> 操作符号，取得个个字段值。
*             相对而言还是比较简单的，
*             早年在代码里面用了很多保护，但后来发现，这些保护的意义实在有限，不如让你崩溃
*
*             2013年1月，我从新回头整理一下这段代码的注释，我觉得我在提供API上有点偏执狂的
*             倾向，好吧，想8年前的懵懂少年致敬。
*
* @note       其中有不少用列字段名称处理的函数，但其实一方面，列名称的处理并不高效，
*             一方面其实很多结果的列字段名称很不标准，比如大小写，一些字段是组合，函
*             数运算得到的结果等，所以我不推荐使用，
*
*/

#ifndef ZCE_LIB_MYSQL_DB_RESULT_H_
#define ZCE_LIB_MYSQL_DB_RESULT_H_

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

#include "zce/util/non_copyable.h"
#include "zce/os_adapt/string.h"
#include "zce/logger/logging.h"
#include "zce/mysql/field.h"

/*!
* @brief      MYSQL的结果集封装
*             考虑一下，也让这个东东NO Copyable了，
*
*/
class ZCE_Mysql_Result: zce::NON_Copyable
{
public:

    ///一个空结构，由于二进制数据的的转换，用于>>操作区分char *和2进制数据，
    struct BINARY
    {
    };

public:
    ///构造函数
    ZCE_Mysql_Result();
    ///构造函数
    ZCE_Mysql_Result(MYSQL_RES *sqlresult);
    ///析构函数
    ~ZCE_Mysql_Result();

    ///结果集合是否为空
    inline bool is_null();

    /*!
    * @brief      设置结果集合
    * @return     void
    * @param      sqlresult 放入结果集合
    */
    void set_mysql_result(MYSQL_RES *sqlresult);

    ///根据Field ID返回表定义列域名,注意计算得到的列的名字也可能是空
    inline char *field_name(unsigned int fieldid) const;

    /*!
    * @brief      根据Field Name得到Field ID,列号
    * @return     inline int 0成功，-1失败
    * @param[in]  fname      列名称,
    * @param[out] fieldid    返回的列名称对应列ID
    */
    inline int field_index(const char *fname,
                           unsigned int &fieldid) const;

    /*!
    * @brief      返回结果集的行数目
    * @return     unsigned int 行的数量
    */
    inline unsigned int num_of_rows() const;

    /*!
    * @brief      返回结果集的列数目
    * @return     unsigned int 列的数量
    */
    inline unsigned int num_of_fields() const;

    /*!
    * @brief      根据列序号ID得到字段FIELD，[]操作符号函数不检查检查列ID,自己保证参数
    * @return     ZCE_MySQL_Field
    * @param[in]  fieldid          取的字段下标
    */
    ZCE_Mysql_Field operator[](unsigned int fieldid) const;

    /*!
    * @brief      通过列ID，查询当前行的字段，性能好，下标定位
    * @return     int      0成功，-1失败
    * @param[in]  fieldid  列ID，从0开始
    * @param[out] ffield   返回列的值
    */
    int get_field(unsigned int fieldid,ZCE_Mysql_Field &ffield) const;

    /*!
    * @brief      通过列名称查询当前行的某个字段的值，但性能并不好
    * @return     int     0成功，-1失败
    * @param      fname   列字段的名称，SQL中的名称，如果是复杂的SQL SELECT，列名字会比较怪异，
    * @param      ffield  返回参数，列数据的封装
    */
    int get_field(const char *,ZCE_Mysql_Field &ffield) const;

    /*!
    * @brief      在当前行，根据列序号ID得到字段值,将数据的指针作为作为返回值
    * @return     const char* 数据的指针，返回NULL表示取错误
    * @param      fieldid     下标
    * @note
    */
    inline const char *field_data(const unsigned int fieldid) const;

    /*!
    * @brief      在当前行，当前列，得到字段值,将数据的指针作为作为返回值
    * @return     const char* 数据的指针，返回NULL表示取错误
    * @param      fname       列（字段）名称
    */
    const char *field_data(const char *fname) const;

    /*!
    * @brief      根据列序号ID得到当前行的字段值,
    * @return     int       0成功，-1失败
    * @param      fieldid   列ID
    * @param      pfdata    列数据的指针
    */
    inline int field_data(unsigned int fieldid,char *pfdata) const;

    /*!
    * @brief      根据列名字得到字段值
    * @return     int      0成功，-1失败
    * @param      fname    列名称
    * @param      pfdata   列数据的指针
    */
    int field_data(const char *fname,char *pfdata) const;

    /*!
    * @brief      根据列序号得到字段的长度
    * @return     int
    * @param      fieldid
    * @param      flength
    * @note
    */
    inline int field_length(unsigned int fieldid,unsigned int &flength) const;

    /*!
    * @brief      根据列名字得到字段的长度
    * @return     int      0成功，-1失败
    * @param      fname    列名称，SELECT字段名称
    * @param      flength  列数据的长度
    */
    int field_length(const char *fname,unsigned int &flength) const;

    ///取得当前的字段的长度
    inline unsigned int get_cur_field_length();

    /*!
    * @brief      根据列序号ID得到字段的类型
    * @return     inline int   0成功，-1失败
    * @param      fieldid      列字段ID
    * @param      ftype        列数据的长度，要参考MYSQL CAPI 的enum_field_types
    */
    inline int field_type(unsigned int fieldid,enum_field_types &ftype) const;

    /*!
    * @brief      根据列名字得到字段的类型
    * @return     int    0成功，-1失败
    * @param      fname  列名称，SELECT字段名称
    * @param      ftype  列类型，要参考MYSQL CAPI 的enum_field_types
    */
    int field_type(const char *fname,enum_field_types &ftype) const;

    /*!
    * @brief      得到字段表结构定义的长度
    * @return     int
    * @param      fieldid 列字段ID
    * @param      flength 列定义的长度，
    */
    int field_define_size(unsigned int fieldid,unsigned int &flength) const;

    /*!
    * @brief      得到字段表结构定义的长度
    * @return     int
    * @param      name
    * @param      flength
    * @note
    */
    int field_define_size(const char *name,unsigned int &flength) const;

    /*!
    * @brief      将结果集处理的行，检索移动到某行
    * @return     int  0成功，-1失败
    * @param      row_id 行ID
    */
    int seek_row(unsigned int row_id);

    /*!
    * @brief      检索到下一行，返回true,其实有点类似Orale的光标处理，呵呵
    * @return     bool true还有结果集合，false没有结果集合了
    */
    bool fetch_row_next();

    /*!
    * @brief      如果已经有结果集, 释放原有的结果集,
    */
    void free_result();

    /// >> 操作符号,用于将结果输出到val中
    ///早年为了安全，>>操作前还做了各种防止溢出的检查，结果反而导致一个bug，
    ///所以后来改为还是由调用者包装边界安全把
    ZCE_Mysql_Result &operator >> (char &val);
    ZCE_Mysql_Result &operator >> (short &val);
    ZCE_Mysql_Result &operator >> (int &val);
    ZCE_Mysql_Result &operator >> (long &val);
    ZCE_Mysql_Result &operator >> (long long &val);

    ZCE_Mysql_Result &operator >> (unsigned char &val);
    ZCE_Mysql_Result &operator >> (unsigned short &val);
    ZCE_Mysql_Result &operator >> (unsigned int &val);
    ZCE_Mysql_Result &operator >> (unsigned long &val);
    ZCE_Mysql_Result &operator >> (unsigned long long &val);

    ZCE_Mysql_Result &operator >> (float &val);
    ZCE_Mysql_Result &operator >> (double &val);

    ZCE_Mysql_Result &operator >> (bool &val);

    ZCE_Mysql_Result &operator >> (char *val);
    ZCE_Mysql_Result &operator >> (unsigned char *val);
    ZCE_Mysql_Result &operator >> (std::string &val);

    ///二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
    ZCE_Mysql_Result &operator >> (BINARY *);

private:
    ///结果集合
    MYSQL_RES *mysql_result_;

    ///结果集合的当前行
    MYSQL_ROW        current_row_;
    ///当前列，
    unsigned int     current_field_;

    ///数组指针,指向结果集合的当前行的各个字段数据的长度
    unsigned long *fields_length_;

    ///结果集的行数
    unsigned int     num_result_row_;

    ///结果集的列数
    unsigned int     num_result_field_;

    ///MYSQL_FIELD数组指针,指向结果集合的所有Field说明.
    MYSQL_FIELD *mysql_fields_;
};

//Description     : 查询结果集合是否为空
inline bool ZCE_Mysql_Result::is_null()
{
    if (mysql_result_)
    {
        return false;
    }

    return true;
}

//根据列名得到列ID,从0开始排序
//循环比较,效率比较低
inline int ZCE_Mysql_Result::field_index(const char *fname,unsigned int &field_id) const
{
    //循环比较所有的列名,效率比较低下
    for (unsigned int i = 0; i < num_result_field_; ++i)
    {
        //MYSQL列名字是不区分大小写的
        if (!strcasecmp(fname,mysql_fields_[i].name))
        {
            field_id = i;
            return 0;
        }
    }

    return -1;
}

//根据列Field ID 返回表定义列域名,列域名字,可能为空
//计算得到的列的列名字也可能是空,
inline char *ZCE_Mysql_Result::field_name(unsigned int fieldid) const
{
    //检查结果集合为空,或者参数nfield错误
    if (mysql_result_ == NULL || fieldid >= num_result_field_)
    {
        return NULL;
    }

    //直接得到列域的名字
    return mysql_fields_[fieldid].name;
}

//返回结果集的行数目,num_result_row_ 结果在execute函数中也可以得到
inline unsigned int ZCE_Mysql_Result::num_of_rows() const
{
    return num_result_row_;
}

//返回结果集的列数目
inline unsigned int ZCE_Mysql_Result::num_of_fields() const
{
    return num_result_field_;
}

//根据字段列ID,得到字段值
const char *ZCE_Mysql_Result::field_data(const unsigned int fieldid) const
{
    if (current_row_ == NULL || fieldid >= num_result_field_)
    {
        return NULL;
    }

    return current_row_[fieldid];
}

//根据字段列ID,得到字段值的指针，长度你自己保证
inline int ZCE_Mysql_Result::field_data(unsigned int fieldid,char *pfdata) const
{
    //检查结果集合的当前行为空(可能没有fetch_row_next),或者参数fieldid错误
    if (current_row_ == NULL || fieldid >= num_result_field_ || pfdata == NULL)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    memcpy(pfdata,current_row_[fieldid],fields_length_[fieldid]);
    return 0;
}

//根据字段顺序ID,得到字段表结构定义的类型
inline int ZCE_Mysql_Result::field_type(unsigned int fieldid,enum_field_types &ftype) const
{
    //检查结果集合为空,或者参数nfield错误
    if (current_row_ == NULL || fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ftype = mysql_fields_[fieldid].type;
    return 0;
}

//根据Field ID 得到此列值的实际长度
inline int ZCE_Mysql_Result::field_length(unsigned int fieldid,unsigned int &flength) const
{
    //检查结果集合的当前行为空(可能没有fetch_row_next),或者参数fieldid错误
    if (current_row_ == NULL && fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    flength = fields_length_[fieldid];
    return 0;
}

inline unsigned int ZCE_Mysql_Result::get_cur_field_length()
{
    return static_cast<unsigned int>(fields_length_[current_field_]);
}

//根据字段的序列值得到字段值
inline int ZCE_Mysql_Result::get_field(unsigned int fieldid,ZCE_Mysql_Field &ffield) const
{
    //进行安全检查，如果错误返回
    if (current_row_ == NULL || fieldid >= num_result_field_)
    {
        ZCE_ASSERT(false);
        return -1;
    }

    ffield.set_field(current_row_[fieldid],fields_length_[fieldid],mysql_fields_[fieldid].type);
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2004年7月22日
Function        : ZCE_Mysql_Result::operator[]
Return          : ZCE_MySQL_Field
Parameter List  :
  Param1: unsigned int fieldid
Description     : 根据列序号ID得到字段FIELD，
Calls           :
Called By       :
Other           : []操作符号函数不检查检查列ID,自己保证参数
Modify Record   :
******************************************************************************************/
inline ZCE_Mysql_Field ZCE_Mysql_Result::operator[](unsigned int fieldid) const
{
    ZCE_Mysql_Field ffield(current_row_[fieldid],fields_length_[fieldid],mysql_fields_[fieldid].type);
    return ffield;
}

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_DB_RESULT_H_
