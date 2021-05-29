#pragma once

#include "zce/util/lord_rings.h"

namespace zce
{
template<typename POOL_OBJ>
class point_pool :public zce::lordrings<POOL_OBJ*>
{
};
}