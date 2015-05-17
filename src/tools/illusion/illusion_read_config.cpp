#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_coding_convert.h"
#include "illusion_read_config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//处理的单子实例
Illusion_Read_Config *Illusion_Read_Config::instance_ = NULL;

//
Illusion_Read_Config::Illusion_Read_Config()
{
}


Illusion_Read_Config::~Illusion_Read_Config()
{
}


Illusion_Read_Config *Illusion_Read_Config::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Illusion_Read_Config();
    }
    return instance_;
}

void Illusion_Read_Config::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}



//
bool Illusion_Read_Config::initialize(bool need_open_excel,
                                      const std::string &config_path)
{
    config_path_ = config_path;

    //db3的路径没有可以创建
    sqlitedb_pah_ = config_path_;
    ZCE_LIB::path_string_cat(sqlitedb_pah_, "db3");
    if (false == ZCE_LIB::is_directory(sqlitedb_pah_.c_str()))
    {
        ZCE_LIB::mkdir_recurse(sqlitedb_pah_.c_str());
    }

    //log的路径没有可以创建
    outlog_dir_path_ = config_path_;
    ZCE_LIB::path_string_cat(outlog_dir_path_, "log");
    if (false == ZCE_LIB::is_directory(outlog_dir_path_.c_str()))
    {
        ZCE_LIB::mkdir_recurse(outlog_dir_path_.c_str());
    }

    need_open_excel_ = need_open_excel;
    if (need_open_excel_)
    {
        BOOL bret = ils_excel_file_.init_excel();
        if (FALSE == bret)
        {
            return false;
        }
    }
    return true;
}

//
void Illusion_Read_Config::finalize()
{
    clear();

    if (need_open_excel_)
    {
        ils_excel_file_.close_excelfile(FALSE);
        Illusion_ExcelFile::release_excel();
    }

    return;
}

//清理所有的读取数据
void Illusion_Read_Config::clear()
{
    file_cfg_map_.clear();
}


int Illusion_Read_Config::read_excel_byucname(const CString &open_file)
{
    clear();

    BOOL bret = ils_excel_file_.open_excelfile(open_file);
    //Excel文件打开失败
    if (bret != TRUE)
    {
        return -1;
    }
    //
    TRACE("Dream excecl file have sheet num[%d].\n", ils_excel_file_.sheets_count());

    //表格错误
    if (ils_excel_file_.load_sheet(_T("TABLE_CONFIG"), FALSE) == FALSE ||
        ils_excel_file_.load_sheet(_T("ENUM_CONFIG"), FALSE) == FALSE)
    {
        //
        ::AfxMessageBox(_T("你选择的配置EXCEL不是能读取的配置表，请重现检查后打开。!"));
        return -1;
    }

    //file_cfg_map_[open_file] = excel_data;
    EXCEL_FILE_DATA excel_data;
    auto result = file_cfg_map_.insert(make_pair(open_file, excel_data));
    if (!result.second)
    {
        return -1;
    }

    //
    EXCEL_FILE_DATA &xls_data = (*result.first).second;
    int ret = read_table_enum(xls_data);
    if (0 != ret)
    {
        ::AfxMessageBox(_T("你选择的配置EXCEL文件中的ENUM_CONFIG表不正确，请重现检查后打开。!"));
        return ret;
    }

    //
    ret = read_table_config(xls_data);
    if (0 != ret)
    {
        ::AfxMessageBox(_T("你选择的配置EXCEL文件中的TABLE_CONFIG表不正确，请重现检查后打开。!"));
        return ret;
    }

    ///
    ARRARY_OF_AI_IIJIMA_BINARY  fandaoai_ary;

    auto iter_tmp = xls_data.xls_table_cfg_.begin();
    for (; iter_tmp != xls_data.xls_table_cfg_.end(); ++iter_tmp)
    {

        ret = read_table_cfgdata(iter_tmp->second, &fandaoai_ary);
        if (0 != ret)
        {
            return ret;
        }

        ret = save_to_sqlitedb(iter_tmp->second, &fandaoai_ary);
        if (0 != ret)
        {
            return ret;
        }
    }

    return 0;
}

///
int Illusion_Read_Config::read_excel(const std::string &mbcs_name)
{
#if defined UNICODE || defined _UNICODE
    DWORD ret = 0;
    const size_t UTF16_NAME_BUFFER = 16 * 1024;
    wchar_t utf16_name[UTF16_NAME_BUFFER];
    ret = ::MultiByteToWideChar(CP_ACP,
                                0,
                                mbcs_name.c_str(),
                                mbcs_name.length() + 1,
                                utf16_name,
                                UTF16_NAME_BUFFER);
    if (ret == 0)
    {
        return -1;
    }

    return read_excel_byucname(utf16_name);
#else
    return read_excel_byucname(mbcs_name.c_str());
#endif
}


//
void Illusion_Read_Config::map_proto_path(const std::string &path_name)
{
    ils_proto_reflect_.map_path(path_name);
}

///
int Illusion_Read_Config::read_proto(const std::string &proto_fname)
{

    int ret = ils_proto_reflect_.import_file(proto_fname);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}


//读取所有的枚举值
int Illusion_Read_Config::read_table_enum(EXCEL_FILE_DATA &file_cfg_data)
{
    //前面检查过了
    BOOL bret =  ils_excel_file_.load_sheet(_T("ENUM_CONFIG"), TRUE);
    if (bret == FALSE)
    {
        return -1;
    }

    //答应行列
    long row_count = ils_excel_file_.row_count();
    long col_count = ils_excel_file_.column_count();
    TRACE("ENUM_CONFIG table have col_count = %u row_count =%u\n", col_count, row_count);

    //注意行列的下标都是从1开始。
    const long COL_ENUM_KEY = 1;
    const long COL_ENUM_VALUE = 2;
    const TCHAR ENUM_FIRST_CHAR = _T('[');

    size_t read_enum = 0;
    //读取所有的行
    for (long i = 1; i <= row_count; ++i)
    {

        long   row_no = i;
        //字符串
        CString enum_key = ils_excel_file_.get_cell_cstring(row_no, COL_ENUM_KEY);

        //如果第一个字符是[
        if (enum_key[0] == ENUM_FIRST_CHAR )
        {
            CString enum_vlaue = ils_excel_file_.get_cell_cstring(row_no, COL_ENUM_VALUE);
            file_cfg_data.xls_enum_[enum_key] = enum_vlaue;

            ++read_enum;
        }
    }

    return 0;
}


//读取表格配置
int Illusion_Read_Config::read_table_config(EXCEL_FILE_DATA &file_cfg_data)
{
    //前面检查过了
    BOOL bret = ils_excel_file_.load_sheet(_T("TABLE_CONFIG"), TRUE);
    if (bret == FALSE)
    {
        return -1;
    }

    long row_count = ils_excel_file_.row_count();
    long col_count = ils_excel_file_.column_count();
    TRACE("TABLE_CONFIG table have col_count = %u row_count =%u\n", col_count, row_count);

    //注意行列的下标都是从1开始。
    const long COL_TC_KEY = 1;
    const long COL_TC_VALUE = 2;

    for (long row_no = 1; row_no <= row_count; ++row_no)
    {

        CString tc_key = ils_excel_file_.get_cell_cstring(row_no, COL_TC_KEY);

        CString temp_value;
        TABLE_CONFIG tc_data;

        if (tc_key == _T("表格名称"))
        {

            tc_data.excel_table_name_ = ils_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (tc_data.excel_table_name_.IsEmpty())
            {
                continue;
            }

            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }
            //table id
            tc_data.table_id_ = ils_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.table_id_ <= 0)
            {
                return -1;
            }
            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }
            tc_data.read_data_start_ = ils_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.read_data_start_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }

            temp_value  = ils_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (temp_value.IsEmpty())
            {
                return -1;
            }
            Coding_Convert::instance()->default_to_utf8(temp_value, tc_data.pb_msg_name_);
            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }
            tc_data.pb_fieldname_line_ = ils_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.pb_fieldname_line_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }
            temp_value = ils_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (temp_value.IsEmpty())
            {
                return -1;
            }
            Coding_Convert::instance()->default_to_utf8(temp_value, tc_data.sqlite3_db_name_);

            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }

            tc_data.index1_column_ = ils_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.index1_column_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_count)
            {
                return -1;
            }
            tc_data.index2_column_ = ils_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            //INDEX 2可以为0
            //if (tc_data.index2_column_ <= 0)

            auto result = file_cfg_data.xls_table_cfg_.insert(std::make_pair(tc_data.excel_table_name_, tc_data));
            if (false == result.second)
            {
                return -2;
            }
        }
        else
        {
            continue;
        }
    }

    return 0;
}


//读取表格数据read_table_data
int Illusion_Read_Config::read_table_cfgdata(TABLE_CONFIG &tc_data,
                                             ARRARY_OF_AI_IIJIMA_BINARY *aiiijma_ary)
{
    int ret = 0;
    //检查EXCEL文件中是否有这个表格
    if (ils_excel_file_.load_sheet(tc_data.excel_table_name_, TRUE) == FALSE)
    {
        return -3;
    }

    google::protobuf::Message *tmp_msg = NULL;
    ret = ils_proto_reflect_.new_mesage(tc_data.pb_msg_name_, tmp_msg);
    if (ret != 0)
    {
        return ret;
    }
    std::shared_ptr <google::protobuf::Message> new_msg(tmp_msg);

    long line_count = ils_excel_file_.row_count();
    long col_count = ils_excel_file_.column_count();
    TRACE("%s table have col_count = %u row_count =%u\n", tc_data.excel_table_name_, col_count, line_count);

    CString field_name_cstring;
    std::string field_name_stdstr;

    if (tc_data.pb_fieldname_line_ > line_count || tc_data.read_data_start_ > line_count )
    {
        return -4;
    }

    for (long col_no = 1; col_no <= col_count; ++col_no)
    {
        field_name_cstring = ils_excel_file_.get_cell_cstring(tc_data.pb_fieldname_line_, col_no);

        ret = Coding_Convert::instance()->default_to_mbcs(field_name_cstring, field_name_stdstr);

        tc_data.proto_field_ary_.push_back(field_name_stdstr);


        size_t find_pos = tc_data.proto_field_ary_[col_no - 1].find_last_of('.');
        if (find_pos != std::string::npos)
        {
            if (0 == strcmp(tc_data.firstshow_field_.c_str(),
                            field_name_stdstr.c_str()))
            {
                tc_data.item_msg_firstshow_.push_back(true);
            }
            else
            {
                if (tc_data.firstshow_msg_.length() > 0 &&
                    0 == strncmp(tc_data.firstshow_msg_.c_str(),
                                 field_name_stdstr.c_str(),
                                 tc_data.firstshow_msg_.length()))
                {
                    tc_data.item_msg_firstshow_.push_back(false);
                }
                else
                {
                    tc_data.firstshow_field_ = field_name_stdstr;
                    tc_data.firstshow_msg_.assign(field_name_stdstr, 0, find_pos + 1);
                    tc_data.item_msg_firstshow_.push_back(true);
                }
            }
        }
        else
        {
            tc_data.item_msg_firstshow_.push_back(false);
        }

    }

    std::vector<google::protobuf::Message *> field_msg_ary;
    std::vector<const google::protobuf::FieldDescriptor *> field_desc_ary;

    google::protobuf::Message *field_msg = NULL;
    const google::protobuf::FieldDescriptor *field_desc = NULL;
    for (long col_no = 1; col_no <= col_count; ++col_no)
    {
        //如果为空表示不需要关注这列
        if (tc_data.proto_field_ary_[col_no - 1].length() == 0)
        {
            field_msg_ary.push_back(NULL);
            field_desc_ary.push_back(NULL);
            continue;
        }

        //取得字段的描述
        ret = Illusion_Protobuf_Reflect::get_fielddesc(new_msg.get(),
                                                       tc_data.proto_field_ary_[col_no - 1],
                                                       tc_data.item_msg_firstshow_[col_no - 1] == 1 ? true : false,
                                                       field_msg,
                                                       field_desc);
        if (0 != ret)
        {
            ZCE_LOG(RS_ERROR, "Message [%s] don't find field_desc [%s] field_desc name define in Line/Column[%d/%d(%s)]",
                    tc_data.pb_msg_name_.c_str(),
                    tc_data.proto_field_ary_[col_no - 1].c_str(),
                    tc_data.pb_fieldname_line_,
                    col_no,
                    Illusion_ExcelFile::column_mbcs_name(col_no)
                   );
            return ret;
        }
        field_msg_ary.push_back(field_msg);
        field_desc_ary.push_back(field_desc);
    }

    //吧啦吧啦吧啦吧啦吧啦吧啦吧啦，这段啰嗦的代码只是为了搞个日志的名字,EXCEFILENAE_TABLENAME.log
    std::string xlsfile_stdstring;
    Coding_Convert::instance()->default_to_mbcs(ils_excel_file_.open_filename(), xlsfile_stdstring);
    char file_basename[MAX_PATH];
    ZCE_LIB::basename(xlsfile_stdstring.c_str(), file_basename, MAX_PATH);
    std::string log_file_name = file_basename;
    log_file_name += "_";

    std::string tablename_stdstring;
    Coding_Convert::instance()->default_to_mbcs(tc_data.excel_table_name_, tablename_stdstring);
    log_file_name += tablename_stdstring;
    log_file_name += ".log";
    std::string out_log_file = outlog_dir_path_;
    ZCE_LIB::path_string_cat(out_log_file, log_file_name.c_str());

    std::ofstream read_table_log;
    read_table_log.open(out_log_file.c_str());
    if (!read_table_log.good())
    {
        ZCE_LOG(RS_ERROR, "Read excel file data log file [%s] open fail.", out_log_file.c_str());
        return -1;
    }
    std::stringstream sstr_stream;

    //什么？为啥不用google pb 的debugstring直接输出？为啥，自己考虑
    sstr_stream << "Read excel file:" << xlsfile_stdstring.c_str() << " line count" << line_count
                << "column count " << col_count << std::endl;
    sstr_stream << "Read table:" << tablename_stdstring.c_str() << std::endl;

    ZCE_LOG(RS_INFO, "Read excel file:%s table :%s start. line count %u column %u.",
            xlsfile_stdstring.c_str(),
            tablename_stdstring.c_str(),
            line_count,
            col_count);

    int index_1 = 0, index_2 = 0;
    CString read_data;
    std::string set_data, show_data;

    //读取每一行的数据 ,+1是因为read_data_start_也要读取
    aiiijma_ary->resize(line_count - tc_data.read_data_start_ + 1);
    for (long line_no = tc_data.read_data_start_; line_no <= line_count; ++line_no)
    {
        //new_msg->Clear();

        ZCE_LOG(RS_ERROR, "Read line [%d] ", line_no);
        sstr_stream << "Read line:" << line_no << std::endl << "{" << std::endl;

        for (long col_no = 1; col_no <= col_count; ++col_no)
        {
            //如果为空表示不需要关注这列
            if (tc_data.proto_field_ary_[col_no - 1].length() ==  0)
            {
                continue;
            }

            //读出EXCEL数据，注意这个地方是根据MFC的编码决定CString数据的编码
            read_data = ils_excel_file_.get_cell_cstring(line_no, col_no);

            //取得字段的描述
            field_msg = field_msg_ary[ col_no - 1 ];
            field_desc = field_desc_ary[col_no - 1];

            //如果是string 类型，Google PB之支持UTF8
            if (field_desc->type() == google::protobuf::FieldDescriptor::Type::TYPE_STRING )
            {
                ret = Coding_Convert::instance()->default_to_utf8(read_data, set_data);
                //
                ret = Coding_Convert::instance()->default_to_mbcs(read_data, show_data);
            }
            //对于BYTES，
            else if (field_desc->type() == google::protobuf::FieldDescriptor::Type::TYPE_BYTES)
            {
                ret = Coding_Convert::instance()->default_to_bytescoding(read_data, set_data);
                show_data = set_data;
            }
            //其他字段类型统一转换为UTF8的编码
            else
            {
                ret = Coding_Convert::instance()->default_to_utf8(read_data, set_data);
                show_data = set_data;
            }
            //根据描述，设置字段的数据
            ret = Illusion_Protobuf_Reflect::set_fielddata(field_msg, field_desc, set_data);
            if (0 != ret)
            {
                ZCE_LOG(RS_ERROR, "Message [%s] field_desc [%s] type [%d][%s] set_fielddata fail. Line,Colmn[%d|%d(%s)]",
                        tc_data.pb_msg_name_.c_str(),
                        field_desc->full_name().c_str(),
                        field_desc->type(),
                        field_desc->type_name(),
                        line_no,
                        col_no,
                        Illusion_ExcelFile::column_mbcs_name(col_no)
                       );
                return ret;
            }

            //读取索引
            if (col_no == tc_data.index1_column_)
            {
                index_1 = std::stol(set_data, 0, 10 );
            }
            if (tc_data.index2_column_ != 0 && col_no == tc_data.index2_column_)
            {
                index_2 = std::stol(set_data, 0, 10);
            }

            sstr_stream << "\t" << tc_data.proto_field_ary_[col_no - 1].c_str() << ":" << show_data.c_str()
                        << std::endl;
        }

        //如果没有初始化
        if (!new_msg->IsInitialized())
        {
            ZCE_LOG(RS_ERROR, "Read line [%d] message [%s] is not IsInitialized, please check your excel or proto file.",
                    line_no,
                    tc_data.pb_msg_name_.c_str());

            ZCE_LOG(RS_ERROR, "Read line [%d] message [%s] InitializationErrorString :%s;",
                    line_no,
                    tc_data.pb_msg_name_.c_str(),
                    new_msg->InitializationErrorString().c_str());
            return -1;
        }

        sstr_stream << "} index_1 :" << index_1 << "index_2" << index_2 << std::endl;

        ret = (*aiiijma_ary)[line_no - tc_data.read_data_start_].protobuf_encode(
                  index_1, index_2, new_msg.get());
        if (ret != 0)
        {
            return -1;
        }
        std::cout << new_msg->DebugString() << std::endl;
    }

    std::string out_string;
    out_string.reserve(64 * 1024 * 1024);
    out_string = sstr_stream.str();

    ZCE_LOG(RS_INFO, "\n%s", out_string.c_str());
    read_table_log << out_string;

    ZCE_LOG(RS_INFO, "Read excel file:%s table :%s end.", xlsfile_stdstring.c_str(),
            tablename_stdstring.c_str());

    return 0;
}


int Illusion_Read_Config::save_to_sqlitedb(const TABLE_CONFIG &table_cfg,
                                           const ARRARY_OF_AI_IIJIMA_BINARY *aiiijma_ary)
{
    int ret = 0;
    std::string db3_file = sqlitedb_pah_;
    ZCE_LIB::path_string_cat(db3_file, table_cfg.sqlite3_db_name_);

    ZCE_General_Config_Table sqlite_config;
    ret = sqlite_config.open_dbfile(db3_file.c_str(), false, true);
    if (ret != 0)
    {
        return ret;
    }

    ret = sqlite_config.create_table(table_cfg.table_id_);
    if (ret != 0)
    {
        return ret;
    }

    //更新数据库
    ret = sqlite_config.replace_array(table_cfg.table_id_, aiiijma_ary);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}




///从DB3文件里面读取某个配置表的配置
int Illusion_Read_Config::read_db3_conftable(const std::string &db3_fname,
                                             const std::string &conf_message_name,
                                             unsigned int table_id,
                                             unsigned int index_1,
                                             unsigned int index_2)
{
    //
    ZCE_LOG(RS_INFO, "Read sqlite db3 file:%s message :%s table id :%u index 1:%u index2: %u start.",
            db3_fname.c_str(),
            conf_message_name.c_str(),
            table_id,
            index_1,
            index_2);

    int ret = 0;
    std::string db3_file = sqlitedb_pah_;
    ZCE_LIB::path_string_cat(db3_file, db3_fname);

    ///
    ZCE_General_Config_Table sqlite_config;
    ret = sqlite_config.open_dbfile(db3_file.c_str(), true, false);
    if (ret != 0)
    {
        return ret;
    }

    std::string log_file_name = db3_fname;
    log_file_name += "_";

    char table_id_buf[32];
    snprintf(table_id_buf, 31 , "%u", table_id);
    log_file_name += table_id_buf;
    log_file_name += ".log";
    std::string out_log_file = outlog_dir_path_;
    ZCE_LIB::path_string_cat(out_log_file, log_file_name.c_str());

    std::ofstream read_db3_log;
    read_db3_log.open(out_log_file.c_str());
    if (!read_db3_log.good())
    {
        ZCE_LOG(RS_ERROR, "Read excel file data log file [%s] open fail.", out_log_file.c_str());
        return -1;
    }
    google::protobuf::Message *temp_msg = NULL;
    ret = ils_proto_reflect_.new_mesage(conf_message_name, temp_msg);
    std::shared_ptr<google::protobuf::Message> new_msg(temp_msg);
    if (ret != 0)
    {
        return ret;
    }

    std::stringstream sstr_stream;

    //不制定查询对象，查询所有的列表
    if (index_1 == 0 && index_2 == 0)
    {

        ARRARY_OF_AI_IIJIMA_BINARY aiiijma_ary;
        //更新数据库
        ret = sqlite_config.select_array(table_id, 0, 0, &aiiijma_ary);
        if (ret != 0)
        {
            return ret;
        }

        for (size_t i = 0; i < aiiijma_ary.size(); ++i)
        {
            sstr_stream << "index1:" << aiiijma_ary[i].index_1_ << " index2:"
                        << aiiijma_ary[i].index_2_ << " " << std::endl << "{" << std::endl;
            new_msg->Clear();
            ret = aiiijma_ary[i].protobuf_decode(&index_1, &index_2, new_msg.get());
            if (ret != 0)
            {
                return ret;
            }

            Illusion_Protobuf_Reflect::protobuf_output(new_msg.get(), &sstr_stream);
            sstr_stream << "}" << std::endl;
        }
    }
    else
    {
        AI_IIJIMA_BINARY_DATA aiiijma_data;
        aiiijma_data.index_1_ = index_1;
        aiiijma_data.index_2_ = index_2;
        ret = sqlite_config.select_one(table_id, &aiiijma_data);
        if (ret != 0)
        {
            return ret;
        }

        new_msg->Clear();

        sstr_stream << "index 1:" << aiiijma_data.index_1_ << " index 2:"
                    << aiiijma_data.index_2_ << " " << std::endl << "{" << std::endl;

        ret = aiiijma_data.protobuf_decode(&index_1, &index_2, new_msg.get());
        if (ret != 0)
        {
            return ret;
        }
        Illusion_Protobuf_Reflect::protobuf_output(new_msg.get(), &sstr_stream);

        sstr_stream << "}" << std::endl;
    }

    std::string out_string;
    out_string.reserve(64 * 1024 * 1024);
    out_string = sstr_stream.str();

    //打印日志，屏幕输出，
    ZCE_LOG(RS_INFO, "\n%s", out_string.c_str());
    read_db3_log << out_string;

    ZCE_LOG(RS_INFO, "Read sqlite db3 file:%s message :%s table id :%u index1:%u index2: %u end.",
            db3_fname.c_str(),
            conf_message_name.c_str(),
            table_id,
            index_1,
            index_2);

    return 0;
}
