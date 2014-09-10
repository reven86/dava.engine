/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btConeShape.h"



btConeShape::btConeShape (btScalar radius,btScalar height): btConvexInternalShape (),
m_radius (radius),
m_height(height)
{
	m_shapeType = CONE_SHAPE_PROXYTYPE;
	setConeUpIndex(1);
	btVector3 halfExtents;
	m_sinAngle = (m_radius / btSqrt(m_radius * m_radius + m_height * m_height));
}

btConeShapeZ::btConeShapeZ (btScalar radius,btScalar height):
btConeShape(radius,height)
{
	setConeUpIndex(2);
}

btConeShapeX::btConeShapeX (btScalar radius,btScalar height):
btConeShape(radius,height)
{
	setConeUpIndex(0);
}

///choose upAxis index
void	btConeShape::setConeUpIndex(int upIndex)
{
	switch (upIndex)
	{
	case 0:
			m_coneIndices[0] = 1;
			m_coneIndices[1] = 0;
			m_coneIndices[2] = 2;
		break;
	case 1:
			m_coneIndices[0] = 0;
			m_coneIndices[1] = 1;
			m_coneIndices[2] = 2;
		break;
	case 2:
			m_coneIndices[0] = 0;
			m_coneIndices[1] = 2;
			m_coneIndices[2] = 1;
		break;
	default:
		btAssert(0);
	};
}

btVector3 btConeShape::coneLocalSupport(const btVector3& v) const
{
	
	btScalar halfHeight = m_height * btScalar(0.5);

 if (v[m_coneIndices[1]] > v.length() * m_sinAngle)
 {
	btVector3 tmp;

	tmp[m_coneIndices[0]] = btScalar(0.);
	tmp[m_coneIndices[1]] = halfHeight;
	tmp[m_coneIndices[2]] = btScalar(0.);
	return tmp;
 }
  else {
    btScalar s = btSqrt(v[m_coneIndices[0]] * v[m_coneIndices[0]] + v[m_coneIndices[2]] * v[m_coneIndices[2]]);
    if (s > SIMD_EPSILON) {
      btScalar d = m_radius / s;
	  btVector3 tmp;
	  tmp[m_coneIndices[0]] = v[m_coneIndices[0]] * d;
	  tmp[m_coneIndices[1]] = -halfHeight;
	  tmp[m_coneIndices[2]] = v[m_coneIndices[2]] * d;
	  return tmp;
    }
    else  {
		btVector3 tmp;
		tmp[m_coneIndices[0]] = btScalar(0.);
		tmp[m_coneIndices[1]] = -halfHeight;
		tmp[m_coneIndices[2]] = btScalar(0.);
		return tmp;
	}
  }

}

btVector3	btConeShape::localGetSupportingVertexWithoutMargin(const btVector3& vec) const
{
		return coneLocalSupport(vec);
}

void	btConeShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
	for (int i=0;i<numVectors;i++)
	{
		const btVector3& vec = vectors[i];
		supportVerticesOut[i] = coneLocalSupport(vec);
	}
}


btVector3	btConeShape::localGetSupportingVertex(const btVector3& vec)  const
{
	btVector3 supVertex = coneLocalSupport(vec);
	if ( getMargin()!=btScalar(0.) )
	{
		btVector3 vecnorm = vec;
		if (vecnorm .length2() < (SIMD_EPSILON*SIMD_EPSILON))
		{
			vecnorm.setValue(btScalar(-1.),btScalar(-1.),btScalar(-1.));
		} 
		vecnorm.normalize();
		supVertex+= getMargin() * vecnorm;
	}
	return supVertex;
}


void	btConeShape::setLocalScaling(const btVector3& scaling)
{
	int axis = m_coneIndices[1];
	int r1 = m_coneIndices[0];
	int r2 = m_coneIndices[2];
	m_height *= scaling[axis] / m_localScaling[axis];
	m_radius *= (scaling[r1] / m_localScaling[r1] + scaling[r2] / m_localScaling[r2]) / 2;
	m_sinAngle = (m_radius / btSqrt(m_radius * m_radius + m_height * m_height));
	btConvexInternalShape::setLocalScaling(scaling);
}