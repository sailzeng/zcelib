#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/string/extend.h"
#include "zce/os_adapt/backtrace.h"

//=====================================================================================================================

//打印堆栈信息
int zce::backtrace_stack(std::vector<std::string>& str_ary)
{
    //跟踪函数的层数
    const size_t SIZE_OF_BACKTRACE_FUNC = 100;

    //这个方法是提供给Linux 下的GCC使用的
#if defined(ZCE_OS_LINUX)

    void* backtrace_stack_ptr[SIZE_OF_BACKTRACE_FUNC];
    char** symbols_strings;
    std::string line_data;
    int sz_of_stack = ::backtrace(backtrace_stack_ptr, SIZE_OF_BACKTRACE_FUNC);

    //
    symbols_strings = ::backtrace_symbols(backtrace_stack_ptr, sz_of_stack);
    if (symbols_strings == nullptr)
    {
        ZCE_LOG(RS_ERROR, "%s", "[BACKTRACE] backtrace_symbols return fail.");
    }

    //打印所有的堆栈信息,有些时候信息无法显示符号表，建议使用
    for (int j = 0; j < sz_of_stack; j++)
    {
        zce::foo_string_format(line_data, "{}. address {}:\t{}\t source file info[ {}: {}] ",
                               zce::aidout::o_int(j + 1, 3),
                               backtrace_stack_ptr[j],
                               symbols_strings ? symbols_strings[j] : "<no symbol>");
        str_ary.push_back(line_data);
        line_data.clear();
    }

    //释放空间
    ::free(symbols_strings);

#elif defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    //我还没有时间看完dbghelp所有的东西,目前的代码参考后一个版本居多,目前这个东东必须有pdb文件，
    //http://blog.csdn.net/skies457/article/details/7201185
    //https://github.com/adobe/chromium/blob/master/base/debug/stack_trace_win.cc
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
    if (!symbol)
    {
        return -1;
    }
    memset(symbol, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH) * sizeof(TCHAR));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
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
    int machine_type = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrFrame.Offset = context.Ebp;
#elif defined ZCE_WIN64
    int machine_type = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrFrame.Offset = context.Rbp;
#else
#endif
    // Get current process & thread.
    process = ::GetCurrentProcess();
    cur_thread = ::GetCurrentThread();

    //初始化  dbghelp library 并且加载symbol表，注意pdb文件的位置，
    if (!::SymInitialize(process, nullptr, TRUE))
    {
        ZCE_LOG(RS_ERROR, "SymInitialize fail,no symbol loaded error =%d,Please notice PDB file directory.",
                ::GetLastError());
        return -1;
    }

    //这些空间是绝对足够的，我也不做详细的检查了
    const size_t LINE_OUTLEN = 1024;
    std::string line_data;
    line_data.reserve(LINE_OUTLEN);
    BOOL load_symbol = FALSE, load_line = FALSE;
    bool save_ls_error = false, save_ll_error = false;
    int k = 0;
    // Enumerate call stack frame.
    while (::StackWalk64(machine_type,
           process,
           cur_thread,
           &stackframe,
           &context,
           nullptr,
           SymFunctionTableAccess64,
           SymGetModuleBase64,
           nullptr))
    {
        // 结束处理
        if (stackframe.AddrFrame.Offset == 0 || k > SIZE_OF_BACKTRACE_FUNC)
        {
            break;
        }

        // Get symbol.
        load_symbol = ::SymFromAddr(process, stackframe.AddrPC.Offset, nullptr, symbol);
        if (!load_symbol && !save_ls_error)
        {
            ZCE_LOG(RS_ERROR, "SymFromAddr fail,no debug symbol loaded for this function error =%d.",
                    ::GetLastError());
            //只记录一次，避免过多的日志浪费
            save_ls_error = true;
        }

        load_line = ::SymGetLineFromAddr64(process,
                                           stackframe.AddrPC.Offset,
                                           &displacement,
                                           &source_info);
        if (!load_line && !save_ll_error)
        {
            ZCE_LOG(RS_ERROR, "SymGetLineFromAddr64 fail. no debug file and line loaded for this function error =%d.",
                    ::GetLastError());
            save_ll_error = true;
        }
        //就算有偏移地址信息是否也可以辅助定位？
        //if (!load_symbol && !load_line)
        //{
        //    break;
        //}
        zce::foo_string_format(line_data, "{}. address {}:\t{}\t source file info[ {}: {}] ",
                               zce::aidout::o_int(k + 1, 3),
                               zce::aidout::o_hexint(stackframe.AddrPC.Offset, 16),
                               load_symbol ? symbol->Name : "<no symbol>",
                               load_line ? source_info.FileName : "<no source file info>",
                               load_line ? source_info.LineNumber : 0);

        str_ary.push_back(line_data);
        line_data.clear();
        ++k;
    }

    // Clean up and exit.
    ::SymCleanup(process);
    free(symbol);

#endif
    //
    return 0;
}

//调试打印内存信息，就是简单的内存翻译为16进制字符串
int zce::backtrace_stack(FILE* stream)
{
    int ret = 0;
    //%zu不知道VC从什么年代支持的
    std::vector<std::string> str_ary;
    ret = zce::backtrace_stack(str_ary);
    for (std::string& out : str_ary)
    {
        fprintf(stream, "%s\n", out.c_str());
    }
    return ret;
}

//辅助打印一个指针内部数据的函数，用16进制的方式打印日志
int zce::backtrace_stack(zce::LOG_PRIORITY dbg_lvl,
                         const char* dbg_info)
{
    int ret = 0;
    ZCE_LOG(dbg_lvl, "[BACKTRACE_STACK] out pointer [%s].", dbg_info);
    std::vector<std::string> str_ary;
    zce::backtrace_stack(str_ary);
    for (std::string& out : str_ary)
    {
        //方便你grep
        ZCE_LOG(dbg_lvl, "[BACKTRACE_STACK] %s.", out.c_str());
    }

    return ret;
}