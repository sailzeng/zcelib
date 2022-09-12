#include "predefine.h"

//
int test_conf_xml(int  /*argc*/, char* /*argv*/[])
{
    zce::PropertyTree pt_tree;

    int ret = zce::cfg::read_xml("conf/test_0002.xml", &pt_tree);
    if (ret != 0)
    {
        return ret;
    }
    int32_t abc = 0;
    ret = pt_tree.path_get_leaf("root.size.width", "", abc);
    if (ret != 0)
    {
        std::cout << "read fail." << std::endl;
        return ret;
    }
    std::cout << "read:" << abc << std::endl;
    return 0;
}

int test_conf_ini(int  /*argc*/, char* /*argv*/[])
{
    zce::PropertyTree pt_tree;

    int ret = zce::cfg::read_ini("conf/test_0001.ini", &pt_tree);
    if (ret != 0)
    {
        return ret;
    }
    int32_t abc, def = 0;
    ret = pt_tree.path_get_leaf("SECTION_A", "KEY1", abc);
    if (ret != 0)
    {
        std::cout << "read fail." << std::endl;
        return ret;
    }
    std::cout << "read:" << abc << std::endl;

    ret = pt_tree.path_get_leaf("SECTION_A", "KEY2", def);
    if (ret != 0)
    {
        std::cout << "read fail." << std::endl;
        return ret;
    }
    std::cout << "read:" << def << std::endl;

    return 0;
}