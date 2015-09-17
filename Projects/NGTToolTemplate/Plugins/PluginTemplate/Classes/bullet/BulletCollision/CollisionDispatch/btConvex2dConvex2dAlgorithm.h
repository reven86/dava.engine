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

#ifndef BT_CONVEX_2D_CONVEX_2D_ALGORITHM_H
#define BT_CONVEX_2D_CONVEX_2D_ALGORITHM_H

#include "bullet/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"
#include "bullet/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "bullet/LinearMath/btTransformUtil.h" //for btConvexSeparatingDistanceUtil

class btConvexPenetrationDepthSolver;


///The convex2dConvex2dAlgorithm collision algorithm support 2d collision detection for btConvex2dShape
///Currently it requires the btMinkowskiPenetrationDepthSolver, it has support for 2d penetration depth computation
class btConvex2dConvex2dAlgorithm : public btActivatingCollisionAlgorithm
{
	btSimplexSolverInterface*		m_simplexSolver;
	btConvexPenetrationDepthSolver* m_pdSolver;

	
	bool	m_ownManifold;
	btPersistentManifold*	m_manifoldPtr;
	bool			m_lowLevelOfDetail;
	
	int m_numPerturbationIterations;
	int m_minimumPointsPerturbationThreshold;

public:

	btConvex2dConvex2dAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* body0,btCollisionObject* body1, btSimplexSolverInterface* simplexSolver, btConvexPenetrationDepthSolver* pdSolver, int numPerturbationIterations, int minimumPointsPerturbationThreshold);


	virtual ~btConvex2dConvex2dAlgorithm();

	virtual void processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

	virtual btScalar calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

	virtual	void	getAllContactManifolds(btManifoldArray&	manifoldArray)
	{
		///should we use m_ownManifold to avoid adding duplicates?
		if (m_manifoldPtr && m_ownManifold)
			manifoldArray.push_back(m_manifoldPtr);
	}


	void	setLowLevelOfDetail(bool useLowLevel);


	const btPersistentManifold*	getManifold()
	{
		return m_manifoldPtr;
	}

	struct CreateFunc :public 	btCollisionAlgorithmCreateFunc
	{

		btConvexPenetrationDepthSolver*		m_pdSolver;
		btSimplexSolverInterface*			m_simplexSolver;
		int m_numPerturbationIterations;
		int m_minimumPointsPerturbationThreshold;

		CreateFunc(btSimplexSolverInterface*			simplexSolver, btConvexPenetrationDepthSolver* pdSolver);
		
		virtual ~CreateFunc();

		virtual	btCollisionAlgorithm* CreateCollisionAlgorithm(btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* body0,btCollisionObject* body1)
		{
			void* mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(btConvex2dConvex2dAlgorithm));
			return new(mem) btConvex2dConvex2dAlgorithm(ci.m_manifold,ci,body0,body1,m_simplexSolver,m_pdSolver,m_numPerturbationIterations,m_minimumPointsPerturbationThreshold);
		}
	};


};

#endif //BT_CONVEX_2D_CONVEX_2D_ALGORITHM_H
