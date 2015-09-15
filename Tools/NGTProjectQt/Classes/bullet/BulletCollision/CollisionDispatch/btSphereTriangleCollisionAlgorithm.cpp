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
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "btSphereTriangleCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "bullet/BulletCollision/CollisionShapes/btSphereShape.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "SphereTriangleDetector.h"


btSphereTriangleCollisionAlgorithm::btSphereTriangleCollisionAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* col0,btCollisionObject* col1,bool swapped)
: btActivatingCollisionAlgorithm(ci,col0,col1),
m_ownManifold(false),
m_manifoldPtr(mf),
m_swapped(swapped)
{
	if (!m_manifoldPtr)
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(col0,col1);
		m_ownManifold = true;
	}
}

btSphereTriangleCollisionAlgorithm::~btSphereTriangleCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void btSphereTriangleCollisionAlgorithm::processCollision (btCollisionObject* col0,btCollisionObject* col1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
		return;

	btCollisionObject* sphereObj = m_swapped? col1 : col0;
	btCollisionObject* triObj = m_swapped? col0 : col1;

	btSphereShape* sphere = (btSphereShape*)sphereObj->getCollisionShape();
	btTriangleShape* triangle = (btTriangleShape*)triObj->getCollisionShape();
	
	/// report a contact. internally this will be kept persistent, and contact reduction is done
	resultOut->setPersistentManifold(m_manifoldPtr);
	SphereTriangleDetector detector(sphere,triangle, m_manifoldPtr->getContactBreakingThreshold());
	
	btDiscreteCollisionDetectorInterface::ClosestPointInput input;
	input.m_maximumDistanceSquared = btScalar(BT_LARGE_FLOAT);///@todo: tighter bounds
	input.m_transformA = sphereObj->getWorldTransform();
	input.m_transformB = triObj->getWorldTransform();

	bool swapResults = m_swapped;

	detector.getClosestPoints(input,*resultOut,dispatchInfo.m_debugDraw,swapResults);

	if (m_ownManifold)
		resultOut->refreshContactPoints();
	
}

btScalar btSphereTriangleCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* col0,btCollisionObject* col1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return btScalar(1.);
}
