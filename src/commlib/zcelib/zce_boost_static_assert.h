/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_static_assert.h
* @author     Linsongchen
* @version    
* @date       Saturday, December 28, 2013
* @brief      
*             
*             
* @details    
*             
*             
*             
* @note       
*             
*/


#ifndef ZCE_LIB_BOOST_STATIC_ASSERT_H_
#define ZCE_LIB_BOOST_STATIC_ASSERT_H_

#include "zce_shm_predefine.h"

template <bool x>
struct _STATIC_ASSERT_HELPER;

template <>
struct _STATIC_ASSERT_HELPER<true>
{
};

#define STATIC_ASSERT(x) { _STATIC_ASSERT_HELPER<((x) != 0)> STATIC_ASSERT_FAILURE; (void)STATIC_ASSERT_FAILURE;}
#define NESTED_TEMPLATE template

#endif //ZCE_LIB_BOOST_STATIC_ASSERT_H_

