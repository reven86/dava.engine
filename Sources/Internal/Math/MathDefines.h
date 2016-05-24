#ifndef __DAVAENGINE_MATH_DEFINES_H__
#define __DAVAENGINE_MATH_DEFINES_H__

#include "Base/BaseTypes.h"

/*
 It contein's defines from Math2D.h to resolve circular reference
 between Math2D.h and Math2DTemplateClasses.h
 */

namespace DAVA
{
    
#define FLOAT_EQUAL(f1, f2) (fabsf(f1 - f2) < DAVA::EPSILON)
#define FLOAT_EQUAL_EPS(f1, f2, EPS) (fabsf(f1 - f2) < EPS)
}

#endif // __DAVAENGINE_MATH_DEFINES_H__
