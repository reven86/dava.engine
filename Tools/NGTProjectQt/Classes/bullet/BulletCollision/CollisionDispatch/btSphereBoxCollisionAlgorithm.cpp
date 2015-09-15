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

#include "btSphereBoxCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "bullet/BulletCollision/CollisionShapes/btSphereShape.h"
#include "bullet/BulletCollision/CollisionShapes/btBoxShape.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"
//#include <stdio.h>

btSphereBoxCollisionAlgorithm::btSphereBoxCollisionAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* col0,btCollisionObject* col1, bool isSwapped)
: btActivatingCollisionAlgorithm(ci,col0,col1),
m_ownManifold(false),
m_manifoldPtr(mf),
m_isSwapped(isSwapped)
{
	btCollisionObject* sphereObj = m_isSwapped? col1 : col0;
	btCollisionObject* boxObj = m_isSwapped? col0 : col1;
	
	if (!m_manifoldPtr && m_dispatcher->needsCollision(sphereObj,boxObj))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(sphereObj,boxObj);
		m_ownManifold = true;
	}
}


btSphereBoxCollisionAlgorithm::~btSphereBoxCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}



void btSphereBoxCollisionAlgorithm::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)dispatchInfo;
	(void)resultOut;
	if (!m_manifoldPtr)
		return;

	btCollisionObject* sphereObj = m_isSwapped? body1 : body0;
	btCollisionObject* boxObj = m_isSwapped? body0 : body1;


	btSphereShape* sphere0 = (btSphereShape*)sphereObj->getCollisionShape();

	btVector3 normalOnSurfaceB;
	btVector3 pOnBox,pOnSphere;
	btVector3 sphereCenter = sphereObj->getWorldTransform().getOrigin();
	btScalar radius = sphere0->getRadius();
	
	btScalar dist = getSphereDistance(boxObj,pOnBox,pOnSphere,sphereCenter,radius);

	resultOut->setPersistentManifold(m_manifoldPtr);

	if (dist < SIMD_EPSILON)
	{
		btVector3 normalOnSurfaceB = (pOnBox- pOnSphere).normalize();

		/// report a contact. internally this will be kept persistent, and contact reduction is done

		resultOut->addContactPoint(normalOnSurfaceB,pOnBox,dist);
		
	}

	if (m_ownManifold)
	{
		if (m_manifoldPtr->getNumContacts())
		{
			resultOut->refreshContactPoints();
		}
	}

}

btScalar btSphereBoxCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* col0,btCollisionObject* col1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return btScalar(1.);
}


btScalar btSphereBoxCollisionAlgorithm::getSphereDistance(btCollisionObject* boxObj, btVector3& pointOnBox, btVector3& v3PointOnSphere, const btVector3& sphereCenter, btScalar fRadius ) 
{

	btScalar margins;
	btVector3 bounds[2];
	btBoxShape* boxShape= (btBoxShape*)boxObj->getCollisionShape();
	
	bounds[0] = -boxShape->getHalfExtentsWithoutMargin();
	bounds[1] = boxShape->getHalfExtentsWithoutMargin();

	margins = boxShape->getMargin();//also add sphereShape margin?

	const btTransform&	m44T = boxObj->getWorldTransform();

	btVector3	boundsVec[2];
	btScalar	fPenetration;

	boundsVec[0] = bounds[0];
	boundsVec[1] = bounds[1];

	btVector3	marginsVec( margins, margins, margins );

	// add margins
	bounds[0] += marginsVec;
	bounds[1] -= marginsVec;

	/////////////////////////////////////////////////

	btVector3	tmp, prel, n[6], normal, v3P;
	btScalar   fSep = btScalar(10000000.0), fSepThis;

	n[0].setValue( btScalar(-1.0),  btScalar(0.0),  btScalar(0.0) );
	n[1].setValue(  btScalar(0.0), btScalar(-1.0),  btScalar(0.0) );
	n[2].setValue(  btScalar(0.0),  btScalar(0.0), btScalar(-1.0) );
	n[3].setValue(  btScalar(1.0),  btScalar(0.0),  btScalar(0.0) );
	n[4].setValue(  btScalar(0.0),  btScalar(1.0),  btScalar(0.0) );
	n[5].setValue(  btScalar(0.0),  btScalar(0.0),  btScalar(1.0) );

	// convert  point in local space
	prel = m44T.invXform( sphereCenter);
	
	bool	bFound = false;

	v3P = prel;

	for (int i=0;i<6;i++)
	{
		int j = i<3? 0:1;
		if ( (fSepThis = ((v3P-bounds[j]) .dot(n[i]))) > btScalar(0.0) )
		{
			v3P = v3P - n[i]*fSepThis;		
			bFound = true;
		}
	}
	
	//

	if ( bFound )
	{
		bounds[0] = boundsVec[0];
		bounds[1] = boundsVec[1];

		normal = (prel - v3P).normalize();
		pointOnBox = v3P + normal*margins;
		v3PointOnSphere = prel - normal*fRadius;

		if ( ((v3PointOnSphere - pointOnBox) .dot (normal)) > btScalar(0.0) )
		{
			return btScalar(1.0);
		}

		// transform back in world space
		tmp = m44T( pointOnBox);
		pointOnBox    = tmp;
		tmp  = m44T( v3PointOnSphere);		
		v3PointOnSphere = tmp;
		btScalar fSeps2 = (pointOnBox-v3PointOnSphere).length2();
		
		//if this fails, fallback into deeper penetration case, below
		if (fSeps2 > SIMD_EPSILON)
		{
			fSep = - btSqrt(fSeps2);
			normal = (pointOnBox-v3PointOnSphere);
			normal *= btScalar(1.)/fSep;
		}

		return fSep;
	}

	//////////////////////////////////////////////////
	// Deep penetration case

	fPenetration = getSpherePenetration( boxObj,pointOnBox, v3PointOnSphere, sphereCenter, fRadius,bounds[0],bounds[1] );

	bounds[0] = boundsVec[0];
	bounds[1] = boundsVec[1];

	if ( fPenetration <= btScalar(0.0) )
		return (fPenetration-margins);
	else
		return btScalar(1.0);
}

btScalar btSphereBoxCollisionAlgorithm::getSpherePenetration( btCollisionObject* boxObj,btVector3& pointOnBox, btVector3& v3PointOnSphere, const btVector3& sphereCenter, btScalar fRadius, const btVector3& aabbMin, const btVector3& aabbMax) 
{

	btVector3 bounds[2];

	bounds[0] = aabbMin;
	bounds[1] = aabbMax;

	btVector3	p0, tmp, prel, n[6], normal;
	btScalar   fSep = btScalar(-10000000.0), fSepThis;

	// set p0 and normal to a default value to shup up GCC
	p0.setValue(btScalar(0.), btScalar(0.), btScalar(0.));
	normal.setValue(btScalar(0.), btScalar(0.), btScalar(0.));

	n[0].setValue( btScalar(-1.0),  btScalar(0.0),  btScalar(0.0) );
	n[1].setValue(  btScalar(0.0), btScalar(-1.0),  btScalar(0.0) );
	n[2].setValue(  btScalar(0.0),  btScalar(0.0), btScalar(-1.0) );
	n[3].setValue(  btScalar(1.0),  btScalar(0.0),  btScalar(0.0) );
	n[4].setValue(  btScalar(0.0),  btScalar(1.0),  btScalar(0.0) );
	n[5].setValue(  btScalar(0.0),  btScalar(0.0),  btScalar(1.0) );

	const btTransform&	m44T = boxObj->getWorldTransform();

	// convert  point in local space
	prel = m44T.invXform( sphereCenter);

	///////////

	for (int i=0;i<6;i++)
	{
		int j = i<3 ? 0:1;
		if ( (fSepThis = ((prel-bounds[j]) .dot( n[i]))-fRadius) > btScalar(0.0) )	return btScalar(1.0);
		if ( fSepThis > fSep )
		{
			p0 = bounds[j];	normal = (btVector3&)n[i];
			fSep = fSepThis;
		}
	}

	pointOnBox = prel - normal*(normal.dot((prel-p0)));
	v3PointOnSphere = pointOnBox + normal*fSep;

	// transform back in world space
	tmp  = m44T( pointOnBox);		
	pointOnBox    = tmp;
	tmp  = m44T( v3PointOnSphere);		v3PointOnSphere = tmp;
	normal = (pointOnBox-v3PointOnSphere).normalize();

	return fSep;

}

