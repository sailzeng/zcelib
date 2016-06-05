
#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_trace_aid_debug.h"


//=====================================================================================================================



//=====================================================================================================================

int ZCE_LIB::backtrace_stack(ZCE_LOG_PRIORITY dbg_lvl,
                             const char *dbg_info)
{

    //跟踪函数的层数
    const size_t SIZE_OF_BACKTRACE_FUNC = 100;

#if defined(ZCE_OS_LINUX)
    ZCE_LOG(dbg_lvl, "[BACKTRACE]This program compiled by Linux GCC. %s", dbg_info);
    //Windows 下必须是2008或者VISTA之后的SDK才支持，
#elif defined(ZCE_OS_WINDOWS) && ZCE_SUPPORT_WINSVR2008 == 1
    ZCE_LOG(dbg_lvl, "[BACKTRACE]This program compiled by Windows Visual studio .%s", dbg_info);
#else
    ZCE_UNUSED_ARG(SIZE_OF_BACKTRACE_FUNC);
    ZCE_LOG(dbg_lvl, "[BACKTRACE]back_trace_stack don't support this system.%s", dbg_info);
#endif


    //这个方法是提供给Linux 下的GCC使用的
#if defined(ZCE_OS_LINUX)


    void *backtrace_buffer[SIZE_OF_BACKTRACE_FUNC];
    char **symbols_strings;

    int sz_of_stack = ::backtrace(backtrace_buffer, SIZE_OF_BACKTRACE_FUNC);

    //
    symbols_strings = ::backtrace_symbols(backtrace_buffer, sz_of_stack);

    if (symbols_strings == NULL)
    {
        ZCE_LOG(dbg_lvl, "%s", "[BACKTRACE] backtrace_symbols return fail.");
    }

    //打印所有的堆栈信息,有些时候信息无法显示符号表，建议使用
    for (int j = 0; j < sz_of_stack; j++)
    {
        ZCE_LOG(dbg_lvl, "[BACKTRACE] %u, %s.", j + 1, symbols_strings[j]);
    }

    //释放空间
    ::free(symbols_strings);

#elif defined(ZCE_OS_WINDOWS) && ZCE_SUPPORT_WINSVR2008 == 1

    //我还没有时间看完dbghelp所有的东西,目前的代码参考后一个版本居多,目前这个东东必须有pdb文件，
    //http://blog.csdn.net/skies457/article/details/7201185

    // Max length of symbols' name.
    const size_t MAX_NAME_LENGTH = 256;

    // Store register addresses.
    CONTEXT context;
    // Call stack.
    STACKFRAME64 stackframe;
    // Handle to current process & thread.
    HANDLE process, cur_thread;
    // Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.
    // Debugging symbol's information.
    PSYMBOL_INFO symbol;
    // Source information (file name & line number)
    IMAGEHLP_LINE64 source_info;
    // Source line displacement.
    DWORD displacement;

    // Initialize PSYMBOL_INFO structure.
    // Allocate a properly-sized block.
    symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH) * sizeof(char));
    memset(symbol, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH) * sizeof(TCHAR));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
    symbol->MaxNameLen = MAX_NAME_LENGTH;

    // Initialize IMAGEHLP_LINE64 structure.
    memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
    source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Initialize STACKFRAME64 structure.
    RtlCaptureContext(&context);            // Get context.
    memset(&stackframe, 0, sizeof(STACKFRAME64));

    // Fill in register addresses (EIP, ESP, EBP).

    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrStack.Mode = AddrModeFlat;
    stackframe.AddrFrame.Mode = AddrModeFlat;

#if defined ZCE_WIN32
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrFrame.Offset = context.Ebp;
#elif defined ZCE_WIN64
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrFrame.Offset = context.Rbp;
#else
#endif
    // Get current process & thread.
    process = GetCurrentProcess();
    cur_thread = GetCurrentThread();

    // Initialize dbghelp library.
    if (!SymInitialize(process, NULL, TRUE))
    {
        return -1;
    }

    //这些空间是绝对足够的，我也不做详细的检查了
    const size_t LINE_OUTLEN = 1024;
    char line_out[LINE_OUTLEN];
    int use_len = 0;

    uint32_t k = 0;
    // Enumerate call stack frame.
    while (StackWalk64(IMAGE_FILE_MACHINE_I386,
                       process,
                       cur_thread,
                       &stackframe,
                       &context,
                       NULL,
                       SymFunctionTableAccess64,
                       SymGetModuleBase64,
                       NULL))
    {
        use_len = 0;
        // End reaches.
        if (stackframe.AddrFrame.Offset == 0 || k > SIZE_OF_BACKTRACE_FUNC)
        {
            break;
        }

        // Get symbol.
        if (SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol))
        {
            use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, " %s", symbol->Name);

        }

        if (SymGetLineFromAddr64(process, stackframe.AddrPC.Offset,
                                 &displacement,
                                 &source_info))
        {
            // Get source information.
            use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, "\t[ %s: %d] at addr 0x %08llX",
                                source_info.FileName,
                                source_info.LineNumber,
                                stackframe.AddrPC.Offset);
        }
        else
        {
            // If err_code == 0x1e7, no symbol was found.
            if (GetLastError() == 0x1E7)
            {
                use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, "%s", "\tNo debug symbol loaded for this function.");
            }
        }
        ZCE_LOG(dbg_lvl, "[BACKTRACE] %u, %s.", k + 1, line_out);
        ++k;
    }

    SymCleanup(process);    // Clean up and exit.
    free(symbol);

#endif

    //
    return 0;

}



//辅助打印一个指针内部数据的函数，用16进制的方式打印
//用 11 02 03 0E E0         ..... 格式的输出，指针信息。
void ZCE_LIB::debug_pointer(ZCE_LOG_PRIORITY dbg_lvl,
                            const char *dbg_info,
                            const unsigned char  *debug_ptr,
                            size_t data_len)
{
    ZCE_LOG(dbg_lvl, "[DEBUG_POINTER] out pointer address[%p] [%s].", dbg_info, dbg_info);
    //60个字符换行
    const unsigned int LINE_OUT_NUM = 60;

    //这些空间是绝对足够的，我也不做详细的检查了
    const size_t LINE_OUTLEN = 1024;
    char line_out[LINE_OUTLEN];
    int use_len = 0;

    unsigned char ascii_str[LINE_OUT_NUM + 1];
    ascii_str[LINE_OUT_NUM] = '\0';

    size_t j = 0;
    for (size_t i = 0; i < data_len ; ++i, ++j)
    {

        //换行
        if (i % LINE_OUT_NUM == 0 && i != 0  )
        {
            use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, "  %s", ascii_str);

            ZCE_LOG(dbg_lvl, "[DEBUG_POINTER] %s.", line_out);
            //从头开始记录
            j = 0;
            use_len = 0;

        }
        //这个地方效率有点低，后面可以考虑优化一下
        unsigned char bytmp = *(debug_ptr + i);
        use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, " %02X ", bytmp);

        //只考虑能显示的字符，特殊字符更换为'.'
        if (bytmp <= 0x20 || bytmp >= 0xFA )
        {
            bytmp = '.';
        }
        ascii_str [j] = bytmp;
    }

    //如果不是LINE_OUT_NUM 长度整除，要对齐，输出最后的字符串
    if (data_len % LINE_OUT_NUM != 0 )
    {
        //为了对齐，打印空格
        for (size_t k = 0; k < LINE_OUT_NUM - data_len % LINE_OUT_NUM; k++)
        {
            use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, "%s", "    ");
        }

        ascii_str[j] = '\0';
        use_len += snprintf(line_out + use_len, LINE_OUTLEN - use_len, "  %s\n", ascii_str);
    }
    ZCE_LOG(dbg_lvl, "[DEBUG_POINTER] %s", line_out);

    return;
}
