#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_boost_close_assist.h"

namespace ZCE_LIB
{


template <> close_assist<FILE *>::~close_assist()
{
    ::fclose(to_close_);
}



template <> close_assist<ZCE_HANDLE>::~close_assist()
{
    ZCE_OS::close(to_close_);
}


}
