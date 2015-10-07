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


#ifndef BT_MANIFOLD_RESULT_H
#define BT_MANIFOLD_RESULT_H

class btCollisionObject;
#include "bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"
class btManifoldPoint;

#include "bullet/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h"

#include "bullet/LinearMath/btTransform.h"

typedef bool (*ContactAddedCallback)(btManifoldPoint& cp,	const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1);
extern ContactAddedCallback		gContactAddedCallback;

//#define DEBUG_PART_INDEX 1


///btManifoldResult is a helper class to manage  contact results.
class btManifoldResult : public btDiscreteCollisionDetectorInterface::Result
{
protected:

	btPersistentManifold* m_manifoldPtr;

	//we need this for compounds
	btTransform	m_rootTransA;
	btTransform	m_rootTransB;

	btCollisionObject* m_body0;
	btCollisionObject* m_body1;
	int	m_partId0;
	int m_partId1;
	int m_index0;
	int m_index1;
	

public:

	btManifoldResult()
#ifdef DEBUG_PART_INDEX
		:
	m_partId0(-1),
	m_partId1(-1),
	m_index0(-1),
	m_index1(-1)
#endif //DEBUG_PART_INDEX
	{
	}

	btManifoldResult(btCollisionObject* body0,btCollisionObject* body1);

	virtual ~btManifoldResult() {};

	void	setPersistentManifold(btPersistentManifold* manifoldPtr)
	{
		m_manifoldPtr = manifoldPtr;
	}

	const btPersistentManifold*	getPersistentManifold() const
	{
		return m_manifoldPtr;
	}
	btPersistentManifold*	getPersistentManifold()
	{
		return m_manifoldPtr;
	}

	virtual void setShapeIdentifiersA(int partId0,int index0)
	{
		m_partId0=partId0;
		m_index0=index0;
	}

	virtual void setShapeIdentifiersB(	int partId1,int index1)
	{
		m_partId1=partId1;
		m_index1=index1;
	}


	virtual void addContactPoint(const btVector3& normalOnBInWorld,const btVector3& pointInWorld,btScalar depth);

	SIMD_FORCE_INLINE	void refreshContactPoints()
	{
		btAssert(m_manifoldPtr);
		if (!m_manifoldPtr->getNumContacts())
			return;

		bool isSwapped = m_manifoldPtr->getBody0() != m_body0;

		if (isSwapped)
		{
			m_manifoldPtr->refreshContactPoints(m_rootTransB,m_rootTransA);
		} else
		{
			m_manifoldPtr->refreshContactPoints(m_rootTransA,m_rootTransB);
		}
	}

	const btCollisionObject* getBody0Internal() const
	{
		return m_body0;
	}

	const btCollisionObject* getBody1Internal() const
	{
		return m_body1;
	}
	
};

#endif //BT_MANIFOLD_RESULT_H
