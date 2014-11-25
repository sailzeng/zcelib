
#ifndef ZCE_LIB_SQLITE_CONF_TABLE_H_
#define ZCE_LIB_SQLITE_CONF_TABLE_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3003000


/************************************************************************************************************
struct AI_IIJIMA_BINARY_DATA 用于将一些结构转换为二进制数据，存放于数据字段中
最近比较烦比你烦也比你烦，我梦见和饭岛爱一起晚餐；梦中的餐厅灯光太昏暗，我遍寻不著那蓝色的小药丸
最近也比较烦，写了一个很失败的PPT，我卖掉的是明明是白花油，却被别人非要当神油用了，这也怪我吗？不理解，去TMD！
最近有点事情多到了极点，但是也无聊到了极点，发现最近代码的名字乱七八糟的东西越来越多.
说了很多饭岛爱的废话。不如在代码里面纪念吧，一个充满矛盾人的，名字有爱，，被无数人无数幻想，却一生无爱，
被李宗盛传唱，但不被大众所知，结果在死后却上了sina网的首页，引发中国人民的对日本一个女孩怀念。
半推荐她的小说《柏拉图式性爱》，小说在一个侧面反映了黑暗的资本主义社会对一个小女孩身心的摧残，所以推荐大家读读，
最后要说的是，生命不易，希望大家珍惜。
***********************************************************************************************************/
struct AI_IIJIMA_BINARY_DATA
{
public:

    //动态编解码的数据区长度
    static const size_t MAX_LEN_OF_AI_IIJIMA_DATA = 32 * 1024;

public:

    //数据区长度
    unsigned int           ai_data_length_;
    //动态数据取
    char                   ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA + 1];

public:
    //构造和析构函数
    AI_IIJIMA_BINARY_DATA();
    ~AI_IIJIMA_BINARY_DATA();
    //
    void clear();

    //将一个结构进行编码
    template<class T>
    int AiIijimaDataEnCode(const T &info);
    //将一个结构进行解码
    template<class T>
    int AiIijimaDataDeCode(T &info);

};

template<class T>
int AI_IIJIMA_BINARY_DATA::AiIijimaDataDeCode(T &info)
{

    ACE_InputCDR incdr(ai_iijima_data_, ai_data_length_);
    //不能通过good_bit判断结果
    if (false == (incdr >> info))
    {
        ACE_DEBUG((LM_ERROR, "Class %s CDR Decode Fail.\n", typeid(info).name()));
        return -1;
    }

    return -1;
}

template<class T>
int AI_IIJIMA_BINARY_DATA::AiIijimaDataEnCode(const T &info)
{
    ACE_OutputCDR outcdr(ai_iijima_data_, MAX_LEN_OF_AI_IIJIMA_DATA);
    if (false == (outcdr << info))
    {
        //不知道这样写是否会影响客户端的使用?
        ACE_DEBUG((LM_ERROR, "Class %s CDR Encode Fail.\n", typeid(info).name()));
        return -1;
    }

    size_t szdatalen = outcdr.total_length();

#if defined DEBUG || defined _DEBUG
    ZCE_ASSERT(szdatalen <= MAX_LEN_OF_AI_IIJIMA_DATA);
#endif

    //数据太短
    if (szdatalen > MAX_LEN_OF_AI_IIJIMA_DATA)
    {
        return -1;
    }
    //调整FRAME的长度,
    ai_data_length_ = static_cast<unsigned int>(szdatalen);
    if (ai_data_length_ > MAX_LEN_OF_AI_IIJIMA_DATA / 2)
    {
        return -1;
    }

    return 0;
}


typedef std::vector <AI_IIJIMA_BINARY_DATA>   ARRARY_OF_AI_IIJIMA_BINARY;



/******************************************************************************************
struct BaobaoGeneralDBConf 一个很通用的从DB中间得到通用配置信息的方法
******************************************************************************************/
//一个很通用的从DB中间得到通用配置信息的结构


class General_SQLite_Config
{


public:

    //改写的SQL
    static void ReplaceSQLString(unsigned int table_id,
                                 unsigned int conf_id_1,
                                 unsigned int conf_id_2,
                                 const AI_IIJIMA_BINARY_DATA &conf_data_1,
                                 const AI_IIJIMA_BINARY_DATA &conf_data_2,
                                 unsigned int last_mod_time,
                                 char *sql_string,
                                 size_t &buflen);

    //得到选择一个确定数据的SQL
    static void SelectSQLString(unsigned int table_id,
                                unsigned int conf_id_1,
                                unsigned int conf_id_2,
                                char *sql_string,
                                size_t &buflen);

    //得到删除数据的SQL
    static void DeleteSQLString(unsigned int table_id,
                                unsigned int conf_id_1,
                                unsigned int conf_id_2,
                                char *sql_string,
                                size_t &buflen);


    //
    static void GetCountSQLString(unsigned int table_id,
                                  char *sql_string,
                                  size_t &buflen);

    //
    static void GetArraySQLString(unsigned int table_id,
                                  unsigned int startno,
                                  unsigned int numquery,
                                  char *sql_string,
                                  size_t &buflen);

public:





};


#endif //SQLITE_VERSION_NUMBER >= 3003000

#endif //ZCE_LIB_SQLITE_CONF_TABLE_H_
