// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

//#import "C:\\Program Files\\Microsoft Office 15\\Root\\Office15\\EXCEL.EXE" no_namespace
// CWorksheets wrapper class

class CWorksheets : public COleDispatchDriver
{
public:
    CWorksheets(){} // Calls COleDispatchDriver default constructor
    CWorksheets(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
    CWorksheets(const CWorksheets& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

    // Attributes
public:

    // Operations
public:


    // Worksheets methods
public:
    LPDISPATCH get_Application()
    {
        LPDISPATCH result;
        InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
        return result;
    }
    long get_Creator()
    {
        long result;
        InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
        return result;
    }
    LPDISPATCH get_Parent()
    {
        LPDISPATCH result;
        InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
        return result;
    }
    LPDISPATCH Add(VARIANT& Before, VARIANT& After, VARIANT& Count, VARIANT& Type)
    {
        LPDISPATCH result;
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0xb5, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, &Before, &After, &Count, &Type);
        return result;
    }
    void Copy(VARIANT& Before, VARIANT& After)
    {
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0x227, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &Before, &After);
    }
    long get_Count()
    {
        long result;
        InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
        return result;
    }
    void Delete()
    {
        InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
    }
    void FillAcrossSheets(LPDISPATCH Range, long Type)
    {
        static BYTE parms[] = VTS_DISPATCH VTS_I4;
        InvokeHelper(0x1d5, DISPATCH_METHOD, VT_EMPTY, NULL, parms, Range, Type);
    }
    LPDISPATCH get_Item(VARIANT& Index)
    {
        LPDISPATCH result;
        static BYTE parms[] = VTS_VARIANT;
        InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms, &Index);
        return result;
    }
    void Move(VARIANT& Before, VARIANT& After)
    {
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0x27d, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &Before, &After);
    }
    LPUNKNOWN get__NewEnum()
    {
        LPUNKNOWN result;
        InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, NULL);
        return result;
    }
    void __PrintOut(VARIANT& From, VARIANT& To, VARIANT& Copies, VARIANT& Preview, VARIANT& ActivePrinter, VARIANT& PrintToFile, VARIANT& Collate)
    {
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0x389, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate);
    }
    void PrintPreview(VARIANT& EnableChanges)
    {
        static BYTE parms[] = VTS_VARIANT;
        InvokeHelper(0x119, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &EnableChanges);
    }
    void Select(VARIANT& Replace)
    {
        static BYTE parms[] = VTS_VARIANT;
        InvokeHelper(0xeb, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &Replace);
    }
    LPDISPATCH get_HPageBreaks()
    {
        LPDISPATCH result;
        InvokeHelper(0x58a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
        return result;
    }
    LPDISPATCH get_VPageBreaks()
    {
        LPDISPATCH result;
        InvokeHelper(0x58b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
        return result;
    }
    VARIANT get_Visible()
    {
        VARIANT result;
        InvokeHelper(0x22e, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
        return result;
    }
    void put_Visible(VARIANT& newValue)
    {
        static BYTE parms[] = VTS_VARIANT;
        InvokeHelper(0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, &newValue);
    }
    LPDISPATCH get__Default(VARIANT& Index)
    {
        LPDISPATCH result;
        static BYTE parms[] = VTS_VARIANT;
        InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms, &Index);
        return result;
    }
    void _PrintOut(VARIANT& From, VARIANT& To, VARIANT& Copies, VARIANT& Preview, VARIANT& ActivePrinter, VARIANT& PrintToFile, VARIANT& Collate, VARIANT& PrToFileName)
    {
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0x6ec, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate, &PrToFileName);
    }
    void PrintOut(VARIANT& From, VARIANT& To, VARIANT& Copies, VARIANT& Preview, VARIANT& ActivePrinter, VARIANT& PrintToFile, VARIANT& Collate, VARIANT& PrToFileName, VARIANT& IgnorePrintAreas)
    {
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0x939, DISPATCH_METHOD, VT_EMPTY, NULL, parms, &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate, &PrToFileName, &IgnorePrintAreas);
    }
    LPDISPATCH Add2(VARIANT& Before, VARIANT& After, VARIANT& Count, VARIANT& NewLayout)
    {
        LPDISPATCH result;
        static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
        InvokeHelper(0xbee, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, &Before, &After, &Count, &NewLayout);
        return result;
    }

    // Worksheets properties
public:

};
