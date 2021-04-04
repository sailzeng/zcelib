
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_string_extend.h"
#include "zce_os_adapt_backtrace.h"

//=====================================================================================================================

//��ӡ��ջ��Ϣ
int zce::backtrace_stack(std::vector<std::string> &str_ary)
{

    //���ٺ����Ĳ���
    const size_t SIZE_OF_BACKTRACE_FUNC = 100;

    //����������ṩ��Linux �µ�GCCʹ�õ�
#if defined(ZCE_OS_LINUX)

    void *backtrace_stack_ptr[SIZE_OF_BACKTRACE_FUNC];
    char **symbols_strings;
    std::string line_data;
    int sz_of_stack = ::backtrace(backtrace_stack_ptr, SIZE_OF_BACKTRACE_FUNC);

    //
    symbols_strings = ::backtrace_symbols(backtrace_stack_ptr, sz_of_stack);
    if (symbols_strings == NULL)
    {
        ZCE_LOG(RS_ERROR, "%s", "[BACKTRACE] backtrace_symbols return fail.");
    }

    //��ӡ���еĶ�ջ��Ϣ,��Щʱ����Ϣ�޷���ʾ���ű�����ʹ��
    for (int j = 0; j < sz_of_stack; j++)
    {
        zce::foo_string_format(line_data, "%?. address %?:\t%?\t source file info[ %?: %?] ",
                               zce::Int_Out_Helper(j + 1, 3),
                               backtrace_stack_ptr[j],
                               symbols_strings ? symbols_strings[j] : "<no symbol>");
        str_ary.push_back(line_data);
        line_data.clear();
    }

    //�ͷſռ�
    ::free(symbols_strings);

#elif defined(ZCE_OS_WINDOWS) && ZCE_SUPPORT_WINSVR2008 == 1

    //�һ�û��ʱ�俴��dbghelp���еĶ���,Ŀǰ�Ĵ���ο���һ���汾�Ӷ�,Ŀǰ�������������pdb�ļ���
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

    //��ʼ��  dbghelp library ���Ҽ���symbol��ע��pdb�ļ���λ�ã�
    if (!::SymInitialize(process, NULL, TRUE))
    {
        ZCE_LOG(RS_ERROR, "SymInitialize fail,no symbol loaded error =%d,Please notice PDB file directory.",
                ::GetLastError());
        return -1;
    }

    //��Щ�ռ��Ǿ����㹻�ģ���Ҳ������ϸ�ļ����
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
                         NULL,
                         SymFunctionTableAccess64,
                         SymGetModuleBase64,
                         NULL))
    {
        // ��������
        if (stackframe.AddrFrame.Offset == 0 || k > SIZE_OF_BACKTRACE_FUNC)
        {
            break;
        }

        // Get symbol.
        load_symbol = ::SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol);
        if (!load_symbol && !save_ls_error)
        {
            ZCE_LOG(RS_ERROR, "SymFromAddr fail,no debug symbol loaded for this function error =%d.",
                    ::GetLastError());
            //ֻ��¼һ�Σ�����������־�˷�
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
        //������ƫ�Ƶ�ַ��Ϣ�Ƿ�Ҳ���Ը�����λ��
        //if (!load_symbol && !load_line)
        //{
        //    break;
        //}
        zce::foo_string_format(line_data, "%?. address %?:\t%?\t source file info[ %?: %?] ",
                               zce::Int_Out_Helper(k + 1, 3),
                               zce::Int_HexOut_Helper(stackframe.AddrPC.Offset, 16),
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

//���Դ�ӡ�ڴ���Ϣ�����Ǽ򵥵��ڴ淭��Ϊ16�����ַ���
int zce::backtrace_stack(FILE *stream)
{
    int ret = 0;
    //%zu��֪��VC��ʲô���֧�ֵ�
    std::vector<std::string> str_ary;
    ret = zce::backtrace_stack(str_ary);
    for (std::string &out : str_ary)
    {
        fprintf(stream, "%s\n", out.c_str());
    }
    return ret;
}


//������ӡһ��ָ���ڲ����ݵĺ�������16���Ƶķ�ʽ��ӡ��־
int zce::backtrace_stack(ZCE_LOG_PRIORITY dbg_lvl,
                         const char *dbg_info)
{
    int ret = 0;
    ZCE_LOG(dbg_lvl, "[BACKTRACE_STACK] out pointer [%s].", dbg_info);
    std::vector<std::string> str_ary;
    zce::backtrace_stack(str_ary);
    for (std::string &out : str_ary)
    {
        //������grep
        ZCE_LOG(dbg_lvl, "[BACKTRACE_STACK] %s.", out.c_str());
    }

    return ret;
}