锘?include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_coding_convert.h"
#include "illusion_read_config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//澶勭悊镄勫崟瀛愬疄渚?Illusion_Read_Config *Illusion_Read_Config::instance_ = NULL;

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

    //db3镄勮矾寰勬病链夊彲浠ュ垱寤?    sqlitedb_pah_ = config_path_;
    ZCE_LIB::path_string_cat(sqlitedb_pah_, "db3");
    if (false == ZCE_LIB::is_directory(sqlitedb_pah_.c_str()))
    {
        ZCE_LIB::mkdir_recurse(sqlitedb_pah_.c_str());
    }

    //log镄勮矾寰勬病链夊彲浠ュ垱寤?    outlog_dir_path_ = config_path_;
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

//娓呯悊镓