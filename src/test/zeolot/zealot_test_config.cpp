


#include "zealot_predefine.h"



//
int test_conf_xml(int  /*argc*/, char * /*argv*/[])
{
    ZCE_Conf_PropertyTree pt_tree;
    ZCE_XML_Implement xml_imp;
    xml_imp.read("",&pt_tree);

    return 0;
}

int test_conf_ini(int  /*argc*/, char * /*argv*/[])
{
    return 0;
}