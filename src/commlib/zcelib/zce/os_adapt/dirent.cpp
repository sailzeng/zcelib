#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/dirent.h"

//打开一个目录，用于读取
DIR* zce::opendir(const char* dir_name)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD fileAttribute = ::GetFileAttributesA(dir_name);

    if (fileAttribute == INVALID_FILE_ATTRIBUTES
        || !(fileAttribute & FILE_ATTRIBUTE_DIRECTORY))
    {
        return nullptr;
    }

    DIR* dir_handle = new DIR();

    //前面已经保证了是目录，这儿不会溢出
    ::strncpy(dir_handle->directory_name_, dir_name, PATH_MAX);
    dir_handle->directory_name_[PATH_MAX - 1] = '\0';

    //初始化
    dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
    dir_handle->already_first_read_ = 0;

    dir_handle->dirent_ = new dirent();
    //这两个参数在WINDOWS下没用
    dir_handle->dirent_->d_ino = 0;
    dir_handle->dirent_->d_off = 0;
    dir_handle->dirent_->d_type = DT_UNKNOWN;
    dir_handle->dirent_->d_reclen = sizeof(dirent);
    dir_handle->dirent_->d_name[0] = '\0';

    //必须调用closedir
    return dir_handle;

#elif defined (ZCE_OS_LINUX)
    return ::opendir(dir_name);
#endif
}

//关闭已经打开的目录
int zce::closedir(DIR* dir_handle)
{
#if defined (ZCE_OS_WINDOWS)

    //关闭句柄,
    if (dir_handle->current_handle_ != ZCE_INVALID_HANDLE)
    {
        ::FindClose(dir_handle->current_handle_);
    }

    dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
    dir_handle->already_first_read_ = 0;

    //释放相应的资源
    delete dir_handle->dirent_;
    dir_handle->dirent_ = nullptr;
    delete dir_handle;
    dir_handle = nullptr;

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::closedir(dir_handle);
#endif
}

//
dirent* zce::readdir(DIR* dir_handle)
{
#if defined (ZCE_OS_WINDOWS)

    //WIN32第一次读的函数，和后面的函数不一样。
    if (!dir_handle->already_first_read_)
    {
        char scan_dirname[PATH_MAX + 16];
        //前面验证过是目录，正确使用不会溢出
        strncpy(scan_dirname, dir_handle->directory_name_, PATH_MAX);
        scan_dirname[PATH_MAX + 1] = '\0';
        size_t const lastchar = ::strlen(scan_dirname);
        //前面已经保证了是目录,用断言保护之
        assert(lastchar > 0);

        //WINDOS目前实际支持两种分节符
        if (ZCE_IS_DIRECTORY_SEPARATOR(scan_dirname[lastchar - 1]))
        {
            ::strcat(scan_dirname, ("*"));
        }
        else
        {
            ::strcat(scan_dirname, ("\\*"));
        }

        dir_handle->current_handle_ = ::FindFirstFileA(scan_dirname,
                                                       &dir_handle->fdata_);
        dir_handle->already_first_read_ = 1;
    }
    else
    {
        int const retval = ::FindNextFileA(dir_handle->current_handle_,
                                           &dir_handle->fdata_);

        if (retval == 0)
        {
            // Make sure to close the handle explicitly to avoid a leak!
            ::FindClose(dir_handle->current_handle_);
            dir_handle->current_handle_ = ZCE_INVALID_HANDLE;
        }
    }

    //如果有文件
    if (dir_handle->current_handle_ != ZCE_INVALID_HANDLE)
    {
        //把目录标识出来
        if (dir_handle->fdata_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            dir_handle->dirent_->d_type = DT_DIR;
        }
        else
        {
            dir_handle->dirent_->d_type = DT_REG;
        }
        ::strncpy(dir_handle->dirent_->d_name,
                  dir_handle->fdata_.cFileName,
                  PATH_MAX);
        dir_handle->dirent_->d_name[PATH_MAX - 1] = '\0';

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

//read dir 可以重入版本，
int zce::readdir_r(DIR* dir_handle,
                   dirent* entry,
                   dirent** result)
{
#if defined (ZCE_OS_WINDOWS)

    dirent* p_dirent = zce::readdir(dir_handle);

    //改进后，重入应该安全了。
    if (p_dirent)
    {
        *entry = *p_dirent;
        *result = entry;
    }
    else
    {
        //*result为nullptr也表示读取到了最后
        *result = nullptr;

        //曾经有一次在下面的代码增加了一段判定，用last_error判定是否返回错误
        //但测试发现有问题，FindNextFile 在没有发现文件后，会放入一个错误EXDEV，
        //所以这个方法有问题，感谢charlie和derrick发现问题，修改后自大没有测试
        //if (zce::last_error() != 0) return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //readdir_r并不被推荐

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    return ::readdir_r(dir_handle, entry, result);
#pragma GCC diagnostic pop
#endif
}

//读取某个前缀，后缀的文件名称
int zce::readdir_direntary(const char* dirname,
                           const char* prefix_name,
                           const char* ext_name,
                           bool select_dir,
                           bool select_file,
                           std::vector<dirent>& dirent_ary)
{
    DIR* dir_hdl = zce::opendir(dirname);
    if (dir_hdl == nullptr)
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

    //循环所有文件，检测扩展名称
    dirent dir_tmp, * dir_p = nullptr;
    for (;;)
    {
        dir_p = zce::readdir(dir_hdl);
        if (dir_p == nullptr)
        {
            break;
        }
        dir_tmp = *dir_p;
        //目录
        if (dir_tmp.d_type == DT_DIR && select_dir == false)
        {
            continue;
        }
        //文件
        else if (dir_tmp.d_type == DT_REG && select_file == false)
        {
            continue;
        }
        else
        {
        }

        //文件前后缀比较，在windows 下自动忽视大小写的后缀？
        //比较前缀
        size_t name_len = ::strlen(dir_tmp.d_name);
        if (prefix_name)
        {
            if (name_len < prefix_len ||
#if defined ZCE_OS_LINUX
                0 != ::strncmp(dir_tmp.d_name, prefix_name, prefix_len))
#elif defined ZCE_OS_WINDOWS
                0 != ::strncasecmp(dir_tmp.d_name, prefix_name, prefix_len))
#else
#endif
            {
                continue;
            }
        }

        //比较后缀文件
        if (ext_name)
        {
            if (name_len <= ext_len ||

#if defined ZCE_OS_LINUX
                0 != ::strcmp(dir_tmp.d_name + name_len - ext_len, ext_name))
#elif defined ZCE_OS_WINDOWS
                0 != ::strcasecmp(dir_tmp.d_name + name_len - ext_len, ext_name))
#else
#endif
            {
                continue;
            }
        }
        dirent_ary.push_back(dir_tmp);
    }
    zce::closedir(dir_hdl);

    return 0;
}

//过滤扫描检查一个目录
//const char *dirname，目录名称
//dirent **namelist[],返回参数，一个dirent的指针数组，切记这个参数是函数内2层分配的空间必须释放
//选择器的函数指针
//比较函数排序函数的指针
int zce::scandir(const char* dirname,
                 struct dirent*** namelist,
                 int (*selector)(const struct dirent*),
                 int (*comparator)(const struct dirent**, const struct dirent**))
{
    //Windows下使用opendir等函数实现，
#if defined (ZCE_OS_WINDOWS)

    assert(namelist);

    DIR* dir_handle = zce::opendir(dirname);
    if (dir_handle == nullptr)
    {
        return -1;
    }

    dirent** vector_dir = nullptr;
    dirent dir_tmp, * dir_p = nullptr;

    int once_nfiles = 0;

    //找到合适的文件个数,
    for (;;)
    {
        dir_p = zce::readdir(dir_handle);
        if (dir_p == nullptr)
        {
            break;
        }
        dir_tmp = *dir_p;
        //如果有选择器
        if (selector)
        {
            if ((*selector)(dir_p) != 0)
            {
                ++once_nfiles;
            }
        }
        else
        {
            ++once_nfiles;
        }
    }

    //关闭DIR
    zce::closedir(dir_handle);
    //如果一个合适的都没有，之间返回
    if (0 == once_nfiles)
    {
        return 0;
    }

    //再打开一次使用
    dir_handle = zce::opendir(dirname);
    if (dir_handle == 0)
    {
        return -1;
    }

    //如果有发现可以用的文件 once_nfiles > 0
    vector_dir = (dirent**)::malloc(once_nfiles * sizeof(dirent*));
    if (!vector_dir)
    {
        return -1;
    }

    //两次操作好处是代码清晰，不用写realloc这类函数，坏处是第二次和第一次可能结果不一致
    //为什么要增加个数限制，因为两次检查之间，可能有变化
    int twice_nfiles = 0;
    bool occur_fail = false;
    for (twice_nfiles = 0; twice_nfiles < once_nfiles; )
    {
        dir_p = zce::readdir(dir_handle);
        //如果没有发现文件了
        if (dir_p == nullptr)
        {
            break;
        }
        dir_tmp = *dir_p;
        //如果有选择器函数指针，而且能选上
        if (selector && (*selector)(dir_p) == 0)
        {
            continue;
        }

        vector_dir[twice_nfiles] = (dirent*)::malloc(sizeof(dirent));
        if (vector_dir[twice_nfiles])
        {
            ::memcpy(vector_dir[twice_nfiles], &dir_tmp, sizeof(dirent));
        }
        else
        {
            occur_fail = true;
            break;
        }
        ++twice_nfiles;
    }

    //关闭DIR
    zce::closedir(dir_handle);
    //如果出现了错误，释放分配的内存
    if (occur_fail && vector_dir)
    {
        for (int i = 0; i < twice_nfiles; ++i)
        {
            ::free(vector_dir[i]);
        }
        ::free(vector_dir);
        return -1;
    }

    *namelist = vector_dir;
    if (comparator)
    {
        ::qsort(*namelist,
                twice_nfiles,
                sizeof(dirent*),
                (int (*)(const void*, const void*)) comparator);
    }

    //以第二次扫描数据为准
    return twice_nfiles;

#elif defined (ZCE_OS_LINUX)

#if (__GNUC__ == 4 && __GNUC_MINOR__ <= 1)
    return ::scandir(dirname,
                     namelist,
                     selector,
                     (int (*)(const void*, const void*))comparator);

    //其实我不是特别确定GCC什么版本改进了这个地方，能肯定4.4的版本发生了变化
#else
    return ::scandir(dirname,
                     namelist,
                     selector,
                     comparator);
#endif

#endif
}

//释放scandir 返回参数的里面的各种分配数据，非标准函数
void zce::free_scandir_result(int list_number,
                              struct dirent** namelist)
{
    ZCE_ASSERT(list_number > 0);

    for (int i = 0; i < list_number; ++i)
    {
        //dirent在
        free(namelist[i]);
    }
    free(namelist);
}

//用于目录排序比较
int zce::scandir_namesort(const struct dirent** left, c
                          onst struct dirent** right)
{
#if defined (ZCE_OS_WINDOWS)
    return ::strcmp((*(left))->d_name, (*(right))->d_name);
#elif defined (ZCE_OS_LINUX)
    return ::alphasort((const struct dirent**)left, (const struct dirent**)right);
#endif
}

const char* zce::basename(const char* path_name,
                          char* file_name,
                          size_t buf_len)
{
    const char* temp = nullptr;

    //根据不同的平台找到最后一个分隔符
#if defined (ZCE_OS_WINDOWS)

    //因为Windows平台支持两种分隔符号，这个地方必须特殊处理一下
    const char* temp1 = ::strrchr(path_name, WIN_DIRECTORY_SEPARATOR_CHAR1);
    const char* temp2 = ::strrchr(path_name, WIN_DIRECTORY_SEPARATOR_CHAR2);

    //选择最后的那个作为分割点
    if (temp1 > temp2)
    {
        temp = temp1;
    }
    else
    {
        temp = temp2;
    }

#elif defined (ZCE_OS_LINUX)
    temp = ::strrchr(path_name, LINUX_DIRECTORY_SEPARATOR_CHAR);
#endif

    //如果没有发现分割符号，用整个文件名称作为base name
    if (0 == temp)
    {
        return ::strncpy(file_name, path_name, buf_len);
    }
    else
    {
        return ::strncpy(file_name, temp + 1, buf_len);
    }
}

const char* zce::dirname(const char* path_name,
                         char* dir_name,
                         size_t buf_len)
{
    const char* temp = nullptr;

    //根据不同的平台找到最后一个分隔符
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS平台两个分隔符都可能出现，
    const char* temp1 = ::strrchr(path_name, WIN_DIRECTORY_SEPARATOR_CHAR1);
    const char* temp2 = ::strrchr(path_name, WIN_DIRECTORY_SEPARATOR_CHAR2);

    if (temp1 > temp2)
    {
        temp = temp1;
    }
    else
    {
        temp = temp2;
    }

#elif defined (ZCE_OS_LINUX)
    temp = ::strrchr(path_name, LINUX_DIRECTORY_SEPARATOR_CHAR);
#endif

    //如果没有目录名称，复制当前目录路径返回给你，这样可以避免一些麻烦。
    if (temp == 0)
    {
        return ::strncpy(dir_name, ZCE_CURRENT_DIRECTORY_STR, buf_len);
    }
    else
    {
        size_t len = temp - path_name + 1;
        dir_name[len] = 0;
        return ::strncpy(dir_name,
                         path_name,
                         len);
    }
}

//得到当前目录
char* zce::getcwd(char* buffer, int maxlen)
{
    //其实安装我们明确的宏定义，下面的函数不用_也可以
#if defined (ZCE_OS_WINDOWS)
    return ::_getcwd(buffer, maxlen);
#elif defined (ZCE_OS_LINUX)
    return ::getcwd(buffer, maxlen);
#endif
}

//CD某个目录
int zce::chdir(const char* dirname)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_chdir(dirname);
#elif defined (ZCE_OS_LINUX)
    return ::chdir(dirname);
#endif
}

//建立某个目录，单层,WINDOWS下，后面那个参数无效
int zce::mkdir(const char* pathname, mode_t mode)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(mode);
    return ::_mkdir(pathname);
#elif defined (ZCE_OS_LINUX)
    return ::mkdir(pathname, mode);
#endif
}

//递归的建立目录，非标准函数，如果想一次建立多层目录，用这个函数
int zce::mkdir_recurse(const char* pathname, mode_t mode)
{
    char thread_dir[PATH_MAX + 1];
    memset(thread_dir, 0, sizeof(thread_dir));

    size_t path_len = strlen(pathname);
    int ret = 0;

    //循环处理，对每一层目录都尝试建立
    for (size_t i = 0; i < path_len; ++i)
    {
        if (ZCE_IS_DIRECTORY_SEPARATOR(pathname[i]))
        {
#if defined ZCE_OS_WINDOWS
            //Windows下，由于有盘符的存在，比如F:\ABC\EDF，你建立F:\是会发生错误的,而且不是EEXIST（好像是EINVAL），所以必须判断一下
            if (i > 0 && pathname[i - 1] == ':')
            {
                continue;
            }
#endif

            ::strncpy(thread_dir, pathname, i + 1);
            ret = zce::mkdir(thread_dir, mode);

            //如果目录已经存在，不进行处理
            if (ret != 0 && errno != EEXIST)
            {
                return ret;
            }
        }
    }

    //如果你写的路径最后不是以\或者/结尾，以整个路径创建一下
    if (path_len > 0 && false == ZCE_IS_DIRECTORY_SEPARATOR(pathname[path_len - 1]))
    {
        ret = zce::mkdir(pathname, mode);
    }

    return 0;
}

//删除某个目录
int zce::rmdir(const char* pathname)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_rmdir(pathname);
#elif defined (ZCE_OS_LINUX)
    return ::rmdir(pathname);
#endif
}