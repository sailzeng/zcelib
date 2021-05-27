#pragma once

#include "zce/util/lord_rings.h"

namespace zce
{
template<typename T>
class point_pool :public zce::lordrings<T*> pool_;
{
};
}