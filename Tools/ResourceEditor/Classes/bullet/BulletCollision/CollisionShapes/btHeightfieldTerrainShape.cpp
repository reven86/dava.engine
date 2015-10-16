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

#include "btHeightfieldTerrainShape.h"

#include "LinearMath/btTransformUtil.h"



btHeightfieldTerrainShape::btHeightfieldTerrainShape
(
int heightStickWidth, int heightStickLength, const void* heightfieldData,
btScalar heightScale, btScalar minHeight, btScalar maxHeight,int upAxis,
PHY_ScalarType hdt, bool flipQuadEdges
)
{
	initialize(heightStickWidth, heightStickLength, heightfieldData,
	           heightScale, minHeight, maxHeight, upAxis, hdt,
	           flipQuadEdges);
}



btHeightfieldTerrainShape::btHeightfieldTerrainShape(int heightStickWidth, int heightStickLength,const void* heightfieldData,btScalar maxHeight,int upAxis,bool useFloatData,bool flipQuadEdges)
{
	// legacy constructor: support only float or unsigned char,
	// 	and min height is zero
	PHY_ScalarType hdt = (useFloatData) ? PHY_FLOAT : PHY_UCHAR;
	btScalar minHeight = 0.0;

	// previously, height = uchar * maxHeight / 65535.
	// So to preserve legacy behavior, heightScale = maxHeight / 65535
	btScalar heightScale = maxHeight / 65535;

	initialize(heightStickWidth, heightStickLength, heightfieldData,
	           heightScale, minHeight, maxHeight, upAxis, hdt,
	           flipQuadEdges);
}



void btHeightfieldTerrainShape::initialize
(
int heightStickWidth, int heightStickLength, const void* heightfieldData,
btScalar heightScale, btScalar minHeight, btScalar maxHeight, int upAxis,
PHY_ScalarType hdt, bool flipQuadEdges
)
{
	// validation
	btAssert(heightStickWidth > 1 && "bad width");
	btAssert(heightStickLength > 1 && "bad length");
	btAssert(heightfieldData && "null heightfield data");
	// btAssert(heightScale) -- do we care?  Trust caller here
	btAssert(minHeight <= maxHeight && "bad min/max height");
	btAssert(upAxis >= 0 && upAxis < 3 &&
	    "bad upAxis--should be in range [0,2]");
	btAssert(hdt != PHY_UCHAR || hdt != PHY_FLOAT || hdt != PHY_SHORT &&
	    "Bad height data type enum");

	// initialize member variables
	m_shapeType = TERRAIN_SHAPE_PROXYTYPE;
	m_heightStickWidth = heightStickWidth;
	m_heightStickLength = heightStickLength;
	m_minHeight = minHeight;
	m_maxHeight = maxHeight;
	m_width = (btScalar) (heightStickWidth - 1);
	m_length = (btScalar) (heightStickLength - 1);
	
	m_width2 = - m_width * btScalar(0.5f);
	m_length2 = - m_length * btScalar(0.5f);
	
	m_heightScale = heightScale;
	m_heightfieldDataUnknown = heightfieldData;
	m_heightDataType = hdt;
	m_flipQuadEdges = flipQuadEdges;
	m_localScaling.setValue(btScalar(1.), btScalar(1.), btScalar(1.));

    m_localAabbMin.setValue(0, 0, m_minHeight);
    m_localAabbMax.setValue(m_width, m_length, m_maxHeight);

    // remember origin (defined as exact middle of aabb)
	m_localOrigin = btScalar(0.5) * (m_localAabbMin + m_localAabbMax);
}



btHeightfieldTerrainShape::~btHeightfieldTerrainShape()
{
}



void btHeightfieldTerrainShape::getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const
{
	btVector3 halfExtents = (m_localAabbMax-m_localAabbMin)* m_localScaling * btScalar(0.5);

	btVector3 localOrigin(0, 0, 0);
    localOrigin[2] = (m_minHeight + m_maxHeight) * btScalar(0.5);
    localOrigin *= m_localScaling;

	btMatrix3x3 abs_b = t.getBasis().absolute();  
	btVector3 center = t.getOrigin();
	btVector3 extent = btVector3(abs_b[0].dot(halfExtents),
		   abs_b[1].dot(halfExtents),
		  abs_b[2].dot(halfExtents));
	extent += btVector3(getMargin(),getMargin(),getMargin());

	aabbMin = center - extent;
	aabbMax = center + extent;
}


/// This returns the "raw" (user's initial) height, not the actual height.
/// The actual height needs to be adjusted to be relative to the center
///   of the heightfield's AABB.
btScalar btHeightfieldTerrainShape::getRawHeightFieldValue(int x, int y) const
{
    //i've commented this lines to improve speed!
    return m_heightfieldDataFloat[(y * m_heightStickWidth) + x];
}

/// this returns the vertex in bullet-local coordinates
btVector3 btHeightfieldTerrainShape::getVertex(int x, int y) const
{
	btAssert(x>=0);
	btAssert(y>=0);
	btAssert(x<m_heightStickWidth);
	btAssert(y<m_heightStickLength);
    return btVector3(m_width2 + btScalar(x), m_length2 + btScalar(y), getRawHeightFieldValue(x, y) - m_localOrigin.m_floats[2]);
}



static inline int
getQuantized
(
btScalar x
)
{
	if (x < 0.0) {
		return (int) (x - 0.5);
	}
	return (int) (x + 0.5);
}



/// given input vector, return quantized version
/**
  This routine is basically determining the gridpoint indices for a given
  input vector, answering the question: "which gridpoint is closest to the
  provided point?".

  "with clamp" means that we restrict the point to be in the heightfield's
  axis-aligned bounding box.
 */
void btHeightfieldTerrainShape::quantizeWithClamp(int* out, const btVector3& point,int /*isMax*/) const
{
	btVector3 clampedPoint(point);
	clampedPoint.setMax(m_localAabbMin);
	clampedPoint.setMin(m_localAabbMax);

	out[0] = getQuantized(clampedPoint.getX());
	out[1] = getQuantized(clampedPoint.getY());
	out[2] = getQuantized(clampedPoint.getZ());
		
}



/// process all triangles within the provided axis-aligned bounding box
/**
  basic algorithm:
    - convert input aabb to local coordinates (scale down and shift for local origin)
    - convert input aabb to a range of heightfield grid points (quantize)
    - iterate over all triangles in that subset of the grid
 */
void btHeightfieldTerrainShape::processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const
{
    //BTREVIEW 2.2%
    // scale down the input aabb's so they are in local (non-scaled) coordinates
    btVector3 localAabbMin = aabbMin / m_localScaling; // btVector3(1.f / m_localScaling[0], 1.f / m_localScaling[1], 1.f / m_localScaling[2]);
    btVector3 localAabbMax = aabbMax / m_localScaling; // btVector3(1.f / m_localScaling[0], 1.f / m_localScaling[1], 1.f / m_localScaling[2]);

    // account for local origin
	localAabbMin += m_localOrigin;
	localAabbMax += m_localOrigin;

	//quantize the aabbMin and aabbMax, and adjust the start/end ranges
	int	quantizedAabbMin[3];
	int	quantizedAabbMax[3];
    quantizeWithClamp(quantizedAabbMin, localAabbMin, 0);
    quantizeWithClamp(quantizedAabbMax, localAabbMax, 1);

    // expand the min/max quantized values
	// this is to catch the case where the input aabb falls between grid points!
    for (int i = 0; i < 3; ++i)
    {
        quantizedAabbMin[i]--;
		quantizedAabbMax[i]++;
    }

    int startX = 0;
    int endX = m_heightStickWidth - 1;
    int startY = 0;
    int endY = m_heightStickLength - 1;

    if (quantizedAabbMin[0] > startX)
        startX = quantizedAabbMin[0];

    if (quantizedAabbMax[0] < endX)
        endX = quantizedAabbMax[0];

    if (quantizedAabbMin[1] > startY)
        startY = quantizedAabbMin[1];

    if (quantizedAabbMax[1] < endY)
        endY = quantizedAabbMax[1];

    auto hData = m_heightfieldDataFloat;
    if (m_flipQuadEdges)
    {
        float fStartX = m_width2 + static_cast<float>(startX);
        float fy = m_length2 + static_cast<float>(startY);
        float fz = m_localOrigin.m_floats[2];
        btVector3 localScale = m_localScaling;
        for (int y = startY; y < endY; ++y, fy += 1.0f)
        {
            int r0 = y * m_heightStickWidth;
            int r1 = (y + 1) * m_heightStickWidth;
            float fx = fStartX;
            float y0x0 = m_heightfieldDataFloat[startX + r0] - fz;
            float y1x0 = m_heightfieldDataFloat[startX + r1] - fz;
            for (int x = startX; x < endX; ++x, fx += 1.0f)
            {
                float y0x1 = m_heightfieldDataFloat[x + 1 + r0] - fz;
                float y1x1 = m_heightfieldDataFloat[x + 1 + r1] - fz;

                btVector3 vertices[3];
                vertices[0] = localScale * btVector3(fx, fy, y0x0);
                vertices[1] = localScale * btVector3(fx + 1.0f, fy, y0x1);
                vertices[2] = localScale * btVector3(fx + 1.0f, fy + 1.0f, y1x1);
                callback->processTriangle(vertices, x, y);

                vertices[1] = vertices[2];
                vertices[2] = localScale * btVector3(fx, fy + 1.0f, y1x0);
                callback->processTriangle(vertices, x, y);

                y0x0 = y0x1;
                y1x0 = y1x1;
            }
        }
    }
    else
    {
        assert(false);
        /*
		btVector3 localScaling = m_localScaling;
		for (int j = startJ; j < endJ; j++)
		{
			for (int x = startX; x < endX; x++)
			{
				btVector3 vertices[4];
				vertices[0] = localScaling * getVertex(x, j);
				vertices[1] = localScaling * getVertex(x, j + 1);
				vertices[2] = localScaling * getVertex(x + 1, j);
				callback->processTriangle(vertices, x, j);
				vertices[0] = localScaling * getVertex(x + 1, j);
				vertices[1] = localScaling * getVertex(x, j + 1);
				vertices[2] = localScaling * getVertex(x + 1, j + 1);
				callback->processTriangle(vertices, x, j);
			}
		}
		*/
    }
}

void	btHeightfieldTerrainShape::calculateLocalInertia(btScalar ,btVector3& inertia) const
{
	//moving concave objects not supported
	
	inertia.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
}

void	btHeightfieldTerrainShape::setLocalScaling(const btVector3& scaling)
{
	m_localScaling = scaling;
}

const btVector3& btHeightfieldTerrainShape::getLocalScaling() const
{
	return m_localScaling;
}
