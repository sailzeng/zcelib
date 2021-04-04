
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_dirent.h"

//Ϊʲô��������ACE�������ޣ��������ޣ��������������������������ޣ���������������

//��һ��Ŀ¼�����ڶ�ȡ
DIR *zce::opendir (const char *dir_name)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD fileAttribute = ::GetFileAttributesA (dir_name);

    if (fileAttribute == INVALID_FILE_ATTRIBUTES
        || !(fileAttribute & FILE_ATTRIBUTE_DIRECTORY))
    {
        return NULL;
    }

    DIR *dir_handle = new DIR();

    //ǰ���Ѿ���֤����Ŀ¼������������
    ::strncpy (dir_handle->directory_name_, dir_name, PATH_MAX);
    dir_handle->directory_name_[PATH_MAX] = '\0';

    //��ʼ��
    dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
    dir_handle->started_reading_ = 0;
    dir_handle->dirent_ = new dirent();

    //������������WINDOWS��û��
    dir_handle->dirent_->d_ino = 0;
    dir_handle->dirent_->d_off = 0;

    dir_handle->dirent_->d_type = DT_UNKNOWN;
    dir_handle->dirent_->d_reclen = sizeof (dirent);

    dir_handle->dirent_->d_name[0] = '\0';
    //�������closedir
    return dir_handle;

#elif defined (ZCE_OS_LINUX)
    return ::opendir(dir_name);
#endif
}

//�ر��Ѿ��򿪵�Ŀ¼
int zce::closedir (DIR *dir_handle)
{
#if defined (ZCE_OS_WINDOWS)

    //�رվ��,
    if (dir_handle->current_handle_ != ZCE_INVALID_HANDLE)
    {
        ::FindClose (dir_handle->current_handle_);
    }

    dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
    dir_handle->started_reading_ = 0;

    //�ͷ���Ӧ����Դ
    delete dir_handle->dirent_;
    dir_handle->dirent_ = NULL;
    delete dir_handle;
    dir_handle = NULL;

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::closedir(dir_handle);
#endif
}

//
struct dirent *zce::readdir (DIR *dir_handle)
{
#if defined (ZCE_OS_WINDOWS)

    //WIN32��һ�ζ��ĺ������ͺ���ĺ�����һ����
    if (!dir_handle->started_reading_)
    {
        char scan_dirname[PATH_MAX + 16];
        //ǰ����֤����Ŀ¼����ȷʹ�ò������
        strncpy(scan_dirname, dir_handle->directory_name_, PATH_MAX);
        scan_dirname[PATH_MAX + 1] = '\0';
        size_t const lastchar = ::strlen (scan_dirname);
        //ǰ���Ѿ���֤����Ŀ¼,�ö��Ա���֮
        assert(lastchar > 0);

        //WINDOSĿǰʵ��֧�����ַֽڷ�
        if ( ZCE_IS_DIRECTORY_SEPARATOR(scan_dirname[lastchar - 1] ) )
        {
            ::strcat(scan_dirname, ("*"));
        }
        else
        {
            ::strcat(scan_dirname, ("\\*"));
        }

        dir_handle->current_handle_ = ::FindFirstFileA (scan_dirname,
                                                        &dir_handle->fdata_);
        dir_handle->started_reading_ = 1;
    }
    else
    {
        int const retval = ::FindNextFileA (dir_handle->current_handle_, &dir_handle->fdata_);

        if (retval == 0)
        {
            // Make sure to close the handle explicitly to avoid a leak!
            ::FindClose (dir_handle->current_handle_);
            dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
        }
    }

    //������ļ�
    if (dir_handle->current_handle_ != ZCE_INVALID_HANDLE)
    {

        //��Ŀ¼��ʶ����
        if (dir_handle->fdata_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            dir_handle->dirent_->d_type = DT_DIR;
        }
        else
        {
            dir_handle->dirent_->d_type = DT_REG;
        }
        ::strncpy (dir_handle->dirent_->d_name, dir_handle->fdata_.cFileName, PATH_MAX);
        dir_handle->dirent_->d_name[PATH_MAX] = '\0';



        return dir_handle->dirent_;
    }
    else
    {
        return 0;
    }

#elif defined (ZCE_OS_LINUX)
    return ::readdir(dir_handle);
#endif
}

//read dir ��������汾��
int zce::readdir_r (DIR *dir_handle,
                    dirent *entry,
                    dirent **result)
{

#if defined (ZCE_OS_WINDOWS)

    dirent *p_dirent = zce::readdir(dir_handle);

    //�Ľ�������Ӧ�ð�ȫ�ˡ�
    if (p_dirent)
    {
        *entry = *p_dirent;
        *result = entry;
    }
    else
    {
        //*resultΪNULLҲ��ʾ��ȡ�������
        *result = NULL;

        //������һ��������Ĵ���������һ���ж�����last_error�ж��Ƿ񷵻ش���
        //�����Է��������⣬FindNextFile ��û�з����ļ��󣬻����һ������EXDEV��
        //����������������⣬��лcharlie��derrick�������⣬�޸ĺ��Դ�û�в���
        //if (zce::last_error() != 0) return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //readdir_r�������Ƽ�
    //https://blog.csdn.net/gqtcgq/article/details/50359124
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    return ::readdir_r(dir_handle, entry, result);
#pragma GCC diagnostic pop
#endif

}


//��ȡĳ��ǰ׺����׺���ļ�����
int zce::readdir_nameary(const char *dirname,
                         const char *prefix_name,
                         const char *ext_name,
                         bool select_dir,
                         bool select_file,
                         std::vector<std::string> &file_name_ary)
{
    int retval = 0;
    DIR *dir_hdl = zce::opendir(dirname);
    if (dir_hdl == NULL)
    {
        return -1;
    }

    size_t ext_len = 0, prefix_len = 0;
    if (prefix_name)
    {
        prefix_len = ::strlen(prefix_name);
    }
    if (ext_name)
    {
        ext_len = ::strlen(ext_name);
    }

    //ѭ�������ļ��������չ����
    dirent dir_tmp, *dir_p = NULL;

    for (retval = zce::readdir_r(dir_hdl, &dir_tmp, &dir_p);
         dir_p && retval == 0;
         retval = zce::readdir_r(dir_hdl, &dir_tmp, &dir_p))
    {
        //Ŀ¼
        if (dir_tmp.d_type == DT_DIR )
        {
            if (select_dir == false)
            {
                continue;
            }
        }
        //�ļ�
        else if (dir_tmp.d_type == DT_REG )
        {
            if (select_file == false)
            {
                continue;
            }
        }
        else
        {
            continue;
        }


        //�Ƚ�ǰ׺
        size_t name_len = ::strlen(dir_tmp.d_name);
        if (prefix_name)
        {
            if (name_len < prefix_len ||
                0 != ::strncmp(dir_tmp.d_name, prefix_name, prefix_len))
            {
                continue;

            }
        }

        //�ȽϺ�׺�ļ�
        if (ext_name)
        {

            if (name_len <= ext_len ||
                //��windows ���Զ����Ӵ�Сд�ĺ�׺��
#ifdef ZCE_OS_WINDOWS
                0 != ::strcmp(dir_tmp.d_name + name_len - ext_len, ext_name))
#else
                0 != ::strcasecmp(dir_tmp.d_name + name_len - ext_len, ext_name))
#endif
            {
                continue;
            }

        }
        file_name_ary.push_back(dir_tmp.d_name);
    }

    zce::closedir(dir_hdl);

    if (retval != 0)
    {
        return retval;
    }
    return 0;
}

//����ɨ����һ��Ŀ¼
//const char *dirname��Ŀ¼����
//dirent **namelist[],���ز�����һ��dirent��ָ�����飬�м���������Ǻ�����2�����Ŀռ�����ͷ�
//ѡ�����ĺ���ָ��
//�ȽϺ�����������ָ��
int zce::scandir (const char *dirname,
                  dirent **namelist[],
                  int (*selector)(const struct dirent *),
                  int (*comparator)(const struct dirent **, const struct dirent **))
{

    //Windows��ʹ��opendir�Ⱥ���ʵ�֣�
#if defined (ZCE_OS_WINDOWS)

    assert(namelist);
    int retval = 0;

    DIR *dir_handle = zce::opendir (dirname);
    if (dir_handle == 0)
    {
        return -1;
    }

    dirent **vector_dir = NULL;
    dirent dir_tmp, *dir_p = NULL;

    int once_nfiles = 0;
    bool occur_fail = false;

    //�ҵ����ʵ��ļ�����,
    for (retval = zce::readdir_r (dir_handle, &dir_tmp, &dir_p);
         dir_p && retval == 0;
         retval = zce::readdir_r (dir_handle, &dir_tmp, &dir_p))
    {
        //�����ѡ����
        if (selector )
        {
            if ( (*selector)(dir_p) != 0)
            {
                ++once_nfiles ;
            }
        }
        else
        {
            ++once_nfiles ;
        }
    }

    //������ش���Ŀǰʵ�ֵ�readdir_r���᷵�ط�0ֵ�������������һ���ж�
    if (retval != 0)
    {
        occur_fail = true;
        once_nfiles = 0;
    }

    //�ر�DIR
    zce::closedir (dir_handle);

    if (occur_fail)
    {
        return -1;
    }

    //���һ�����ʵĶ�û�У�֮�䷵��
    if (0 == once_nfiles)
    {
        return 0;
    }

    //�ٴ�һ��ʹ��
    dir_handle = zce::opendir (dirname);

    if (dir_handle == 0)
    {
        return -1;
    }

    //����з��ֿ����õ��ļ�
    if (once_nfiles > 0)
    {
        vector_dir = (dirent **)::malloc (once_nfiles * sizeof(dirent *));
        if (!vector_dir)
        {
            return -1;
        }
    }

    //���β����ô��Ǵ�������������дrealloc���ຯ���������ǵڶ��κ͵�һ�ο��ܽ����һ��
    //ΪʲôҪ���Ӹ������ƣ���Ϊ���μ��֮�䣬�����б仯
    int twice_nfiles = 0;

    for (twice_nfiles = 0; twice_nfiles < once_nfiles; )
    {
        retval = zce::readdir_r (dir_handle, &dir_tmp, &dir_p);

        //�������ʧ��
        if (retval != 0 )
        {
            occur_fail = true;
            break;
        }

        //���û�з����ļ���
        if ( NULL == dir_p )
        {
            break;
        }

        if (selector && (*selector)(dir_p) == 0)
        {
            continue;
        }

        vector_dir[twice_nfiles] = (dirent *)::malloc (sizeof (dirent));
        if (vector_dir[twice_nfiles])
        {
            ::memcpy(vector_dir[twice_nfiles], &dir_tmp, sizeof(dirent));
        }
        else
        {
            return -1;
        }
        ++twice_nfiles;
    }

    //�ر�DIR
    zce::closedir (dir_handle);

    //��������˴����ͷŷ�����ڴ�
    if (occur_fail && vector_dir)
    {
        for (int i = 0; i < twice_nfiles; ++i)
        {
            ::free (vector_dir[i]);
        }

        ::free (vector_dir);
        return -1;
    }

    *namelist = vector_dir;

    if (comparator)
    {
        ::qsort (*namelist,
                 twice_nfiles,
                 sizeof (dirent *),
                 (int ( *)(const void *, const void *)) comparator);
    }

    //�Եڶ���ɨ������Ϊ׼
    return twice_nfiles;

#elif defined (ZCE_OS_LINUX)

#if (__GNUC__ == 4 && __GNUC_MINOR__ <= 1)
    return ::scandir(dirname,
                     namelist,
                     selector,
                     (int ( *)(const void *, const void *))comparator);

    //��ʵ�Ҳ����ر�ȷ��GCCʲô�汾�Ľ�������ط����ܿ϶�4.4�İ汾�����˱仯
#else
    return ::scandir(dirname,
                     namelist,
                     selector,
                     comparator);
#endif

#endif
}


//�ͷ�scandir ���ز���������ĸ��ַ������ݣ��Ǳ�׼����
void zce::free_scandir_result(int list_number, dirent *namelist[])
{
    ZCE_ASSERT(list_number > 0);

    for (int i = 0; i < list_number; ++i)
    {
        free(namelist[i]);
    }

    free(namelist);
}

//����Ŀ¼����Ƚ�
int zce::scandir_namesort (const struct dirent **left, const struct dirent **right)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strcmp ((*(left))->d_name,   (*(right))->d_name);
#elif defined (ZCE_OS_LINUX)
    return ::alphasort((const struct dirent **)left, (const struct dirent **)right);
#endif
}

const char *zce::basename (const char *path_name, char *file_name, size_t buf_len)
{
    const char *temp = NULL;

    //���ݲ�ͬ��ƽ̨�ҵ����һ���ָ���
#if defined (ZCE_OS_WINDOWS)

    //��ΪWindowsƽ̨֧�����ַָ����ţ�����ط��������⴦��һ��
    const char *temp1 = ::strrchr (path_name, WIN_DIRECTORY_SEPARATOR_CHAR1);
    const char *temp2 = ::strrchr (path_name, WIN_DIRECTORY_SEPARATOR_CHAR2);

    //ѡ�������Ǹ���Ϊ�ָ��
    if (temp1 > temp2)
    {
        temp = temp1;
    }
    else
    {
        temp = temp2;
    }

#elif defined (ZCE_OS_LINUX)
    temp = ::strrchr (path_name, LINUX_DIRECTORY_SEPARATOR_CHAR);
#endif

    //���û�з��ַָ���ţ��������ļ�������Ϊbase name
    if (0 == temp )
    {
        return ::strncpy(file_name, path_name, buf_len);
    }
    else
    {
        return ::strncpy(file_name, temp + 1, buf_len);
    }
}

const char *zce::dirname (const char *path_name, char *dir_name, size_t buf_len)
{

    const char *temp = NULL;

    //���ݲ�ͬ��ƽ̨�ҵ����һ���ָ���
#if defined (ZCE_OS_WINDOWS)

    //WINDOWSƽ̨�����ָ��������ܳ��֣�
    const char *temp1 = ::strrchr (path_name, WIN_DIRECTORY_SEPARATOR_CHAR1);
    const char *temp2 = ::strrchr (path_name, WIN_DIRECTORY_SEPARATOR_CHAR2);

    if (temp1 > temp2)
    {
        temp = temp1;
    }
    else
    {
        temp = temp2;
    }

#elif defined (ZCE_OS_LINUX)
    temp = ::strrchr (path_name, LINUX_DIRECTORY_SEPARATOR_CHAR);
#endif

    //���û��Ŀ¼���ƣ����Ƶ�ǰĿ¼·�����ظ��㣬�������Ա���һЩ�鷳��
    if (temp == 0)
    {
        return ::strncpy(dir_name, ZCE_CURRENT_DIRECTORY_STR, buf_len);
    }
    else
    {
        size_t len = temp - path_name + 1;
        dir_name[len] = 0;
        return ::strncpy (dir_name,
                          path_name,
                          len);
    }
}

//�õ���ǰĿ¼
char *zce::getcwd(char *buffer, int maxlen  )
{
    //��ʵ��װ������ȷ�ĺ궨�壬����ĺ�������_Ҳ����
#if defined (ZCE_OS_WINDOWS)
    return ::_getcwd (buffer, maxlen);
#elif defined (ZCE_OS_LINUX)
    return ::getcwd(buffer, maxlen);
#endif
}

//CDĳ��Ŀ¼
int zce::chdir(const char *dirname )
{
#if defined (ZCE_OS_WINDOWS)
    return ::_chdir (dirname);
#elif defined (ZCE_OS_LINUX)
    return ::chdir(dirname);
#endif
}

//����ĳ��Ŀ¼������,WINDOWS�£������Ǹ�������Ч
int zce::mkdir(const char *pathname, mode_t mode)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(mode);
    return ::_mkdir (pathname);
#elif defined (ZCE_OS_LINUX)
    return ::mkdir(pathname, mode);
#endif
}

//�ݹ�Ľ���Ŀ¼���Ǳ�׼�����������һ�ν������Ŀ¼�����������
int zce::mkdir_recurse(const char *pathname, mode_t mode)
{
    char process_dir[PATH_MAX + 1];
    memset(process_dir, 0, sizeof(process_dir));

    size_t path_len = strlen(pathname);

    int ret = 0;

    //ѭ��������ÿһ��Ŀ¼�����Խ���
    for (size_t i = 0; i < path_len; ++i)
    {
        if (ZCE_IS_DIRECTORY_SEPARATOR(pathname[i]))
        {

#if defined ZCE_OS_WINDOWS
            //Windows�£��������̷��Ĵ��ڣ�����F:\ABC\EDF���㽨��F:\�ǻᷢ�������,���Ҳ���EEXIST��������EINVAL�������Ա����ж�һ��
            if ( i > 0 &&  pathname[i - 1] == ':')
            {
                continue;
            }
#endif

            ::strncpy(process_dir, pathname, i + 1);
            ret = zce::mkdir(process_dir, mode);

            //���Ŀ¼�Ѿ����ڣ������д���
            if (ret != 0 && errno != EEXIST)
            {
                return ret;
            }
        }
    }

    //�����д��·���������\����/��β��������·������һ��
    if (path_len > 0 && false == ZCE_IS_DIRECTORY_SEPARATOR(pathname[path_len - 1]) )
    {
        ret = zce::mkdir(pathname, mode);
    }

    return 0;
}

//ɾ��ĳ��Ŀ¼
int zce::rmdir(const char *pathname)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_rmdir (pathname);
#elif defined (ZCE_OS_LINUX)
    return ::rmdir(pathname);
#endif
}


