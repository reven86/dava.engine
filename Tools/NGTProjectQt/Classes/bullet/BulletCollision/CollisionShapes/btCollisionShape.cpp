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
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "bullet/LinearMath/btSerializer.h"

/*
  Make sure this dummy function never changes so that it
  can be used by probes that are checking whether the
  library is actually installed.
*/
extern "C" 
{
void btBulletCollisionProbe ();

void btBulletCollisionProbe () {}
}



void	btCollisionShape::getBoundingSphere(btVector3& center,btScalar& radius) const
{
	btTransform tr;
	tr.setIdentity();
	btVector3 aabbMin,aabbMax;

	getAabb(tr,aabbMin,aabbMax);

	radius = (aabbMax-aabbMin).length()*btScalar(0.5);
	center = (aabbMin+aabbMax)*btScalar(0.5);
}


btScalar	btCollisionShape::getContactBreakingThreshold(btScalar defaultContactThreshold) const
{
	return getAngularMotionDisc() * defaultContactThreshold;
}

btScalar	btCollisionShape::getAngularMotionDisc() const
{
	///@todo cache this value, to improve performance
	btVector3	center;
	btScalar disc;
	getBoundingSphere(center,disc);
	disc += (center).length();
	return disc;
}

void btCollisionShape::calculateTemporalAabb(const btTransform& curTrans,const btVector3& linvel,const btVector3& angvel,btScalar timeStep, btVector3& temporalAabbMin,btVector3& temporalAabbMax) const
{
	//start with static aabb
	getAabb(curTrans,temporalAabbMin,temporalAabbMax);

	btScalar temporalAabbMaxx = temporalAabbMax.getX();
	btScalar temporalAabbMaxy = temporalAabbMax.getY();
	btScalar temporalAabbMaxz = temporalAabbMax.getZ();
	btScalar temporalAabbMinx = temporalAabbMin.getX();
	btScalar temporalAabbMiny = temporalAabbMin.getY();
	btScalar temporalAabbMinz = temporalAabbMin.getZ();

	// add linear motion
	btVector3 linMotion = linvel*timeStep;
	///@todo: simd would have a vector max/min operation, instead of per-element access
	if (linMotion.x() > btScalar(0.))
		temporalAabbMaxx += linMotion.x(); 
	else
		temporalAabbMinx += linMotion.x();
	if (linMotion.y() > btScalar(0.))
		temporalAabbMaxy += linMotion.y(); 
	else
		temporalAabbMiny += linMotion.y();
	if (linMotion.z() > btScalar(0.))
		temporalAabbMaxz += linMotion.z(); 
	else
		temporalAabbMinz += linMotion.z();

	//add conservative angular motion
	btScalar angularMotion = angvel.length() * getAngularMotionDisc() * timeStep;
	btVector3 angularMotion3d(angularMotion,angularMotion,angularMotion);
	temporalAabbMin = btVector3(temporalAabbMinx,temporalAabbMiny,temporalAabbMinz);
	temporalAabbMax = btVector3(temporalAabbMaxx,temporalAabbMaxy,temporalAabbMaxz);

	temporalAabbMin -= angularMotion3d;
	temporalAabbMax += angularMotion3d;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
const char*	btCollisionShape::serialize(void* dataBuffer, btSerializer* serializer) const
{
	btCollisionShapeData* shapeData = (btCollisionShapeData*) dataBuffer;
	char* name = (char*) serializer->findNameForPointer(this);
	shapeData->m_name = (char*)serializer->getUniquePointer(name);
	if (shapeData->m_name)
	{
		serializer->serializeName(name);
	}
	shapeData->m_shapeType = m_shapeType;
	//shapeData->m_padding//??
	return "btCollisionShapeData";
}

void	btCollisionShape::serializeSingleShape(btSerializer* serializer) const
{
	int len = calculateSerializeBufferSize();
	btChunk* chunk = serializer->allocate(len,1);
	const char* structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk,structType,BT_SHAPE_CODE,(void*)this);
}