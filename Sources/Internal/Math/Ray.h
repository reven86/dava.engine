#ifndef __DAVAENGINE_RAY_H__
#define __DAVAENGINE_RAY_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
/**	
	\ingroup math
	\brief Ray in 2D space.
 */
class Ray2
{
public:
    Vector2 origin;
    Vector2 direction;
};

/**	
	\ingroup math
	\brief Ray in 3D space.
 */
class Ray3
{
public:
    Vector3 origin;
    Vector3 direction;
};
};

#endif // __DAVAENGINE_RAY_H__