
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_dlfcn.h"

//----------------------------------------------------------------------------------------
//动态库使用的一部分函数封装
//关闭动态库使用的函数
int zce::dlclose (ZCE_SHLIB_HANDLE handle)
{

#if defined (ZCE_OS_WINDOWS)
    BOOL bret = ::FreeLibrary (handle);

    if (!bret)
    {
        return -1;
    }

    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::dlclose (handle);
#endif //#if defined (ZCE_OS_LINUX)

}

const char *zce::dlerror (void)
{
#if defined (ZCE_OS_WINDOWS)

    //仅仅是用于兼容
    const size_t ERROR_BUFFER_LEN = 256;
    static char error_buf[ERROR_BUFFER_LEN + 1];
    error_buf[ERROR_BUFFER_LEN] = '\0';

    snprintf (error_buf, ERROR_BUFFER_LEN, "error code %d", ::GetLastError());
    return error_buf;

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::dlerror ();
#endif //#if defined (ZCE_OS_LINUX)

}

ZCE_SHLIB_HANDLE  zce::dlopen (const char *fname,
                               int mode)
{

#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG (mode);
    return ::LoadLibraryA (fname);
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::dlopen ((fname), mode);
#endif //#if defined (ZCE_OS_LINUX)

}

void *zce::dlsym (ZCE_SHLIB_HANDLE handle,
                  const char *symbolname)
{

#if defined (ZCE_OS_WINDOWS)
    return ::GetProcAddress (handle, symbolname);
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::dlsym (handle, symbolname);
#endif //

}

