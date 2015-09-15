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

#ifndef BT_MANIFOLD_CONTACT_POINT_H
#define BT_MANIFOLD_CONTACT_POINT_H

#include "bullet/LinearMath/btVector3.h"
#include "bullet/LinearMath/btTransformUtil.h"

#ifdef PFX_USE_FREE_VECTORMATH
	#include "physics_effects/base_level/solver/pfx_constraint_row.h"
typedef sce::PhysicsEffects::PfxConstraintRow btConstraintRow;
#else
	// Don't change following order of parameters
	ATTRIBUTE_ALIGNED16(struct) btConstraintRow {
		btScalar m_normal[3];
		btScalar m_rhs;
		btScalar m_jacDiagInv;
		btScalar m_lowerLimit;
		btScalar m_upperLimit;
		btScalar m_accumImpulse;
	};
	typedef btConstraintRow PfxConstraintRow;
#endif //PFX_USE_FREE_VECTORMATH



/// ManifoldContactPoint collects and maintains persistent contactpoints.
/// used to improve stability and performance of rigidbody dynamics response.
class btManifoldPoint
	{
		public:
			btManifoldPoint()
				:m_userPersistentData(0),
				m_appliedImpulse(0.f),
				m_lateralFrictionInitialized(false),
				m_appliedImpulseLateral1(0.f),
				m_appliedImpulseLateral2(0.f),
				m_contactMotion1(0.f),
				m_contactMotion2(0.f),
				m_contactCFM1(0.f),
				m_contactCFM2(0.f),
				m_lifeTime(0)
			{
			}

			btManifoldPoint( const btVector3 &pointA, const btVector3 &pointB, 
					const btVector3 &normal, 
					btScalar distance ) :
					m_localPointA( pointA ), 
					m_localPointB( pointB ), 
					m_normalWorldOnB( normal ), 
					m_distance1( distance ),
					m_combinedFriction(btScalar(0.)),
					m_combinedRestitution(btScalar(0.)),
					m_userPersistentData(0),
					m_appliedImpulse(0.f),
					m_lateralFrictionInitialized(false),
					m_appliedImpulseLateral1(0.f),
					m_appliedImpulseLateral2(0.f),
					m_contactMotion1(0.f),
					m_contactMotion2(0.f),
					m_contactCFM1(0.f),
					m_contactCFM2(0.f),
					m_lifeTime(0)
			{
				mConstraintRow[0].m_accumImpulse = 0.f;
				mConstraintRow[1].m_accumImpulse = 0.f;
				mConstraintRow[2].m_accumImpulse = 0.f;
			}

			

			btVector3 m_localPointA;			
			btVector3 m_localPointB;			
			btVector3	m_positionWorldOnB;
			///m_positionWorldOnA is redundant information, see getPositionWorldOnA(), but for clarity
			btVector3	m_positionWorldOnA;
			btVector3 m_normalWorldOnB;
		
			btScalar	m_distance1;
			btScalar	m_combinedFriction;
			btScalar	m_combinedRestitution;

         //BP mod, store contact triangles.
         int	   m_partId0;
         int      m_partId1;
         int      m_index0;
         int      m_index1;
				
			mutable void*	m_userPersistentData;
			btScalar		m_appliedImpulse;

			bool			m_lateralFrictionInitialized;
			btScalar		m_appliedImpulseLateral1;
			btScalar		m_appliedImpulseLateral2;
			btScalar		m_contactMotion1;
			btScalar		m_contactMotion2;
			btScalar		m_contactCFM1;
			btScalar		m_contactCFM2;

			int				m_lifeTime;//lifetime of the contactpoint in frames
			
			btVector3		m_lateralFrictionDir1;
			btVector3		m_lateralFrictionDir2;



			btConstraintRow mConstraintRow[3];


			btScalar getDistance() const
			{
				return m_distance1;
			}
			int	getLifeTime() const
			{
				return m_lifeTime;
			}

			const btVector3& getPositionWorldOnA() const {
				return m_positionWorldOnA;
//				return m_positionWorldOnB + m_normalWorldOnB * m_distance1;
			}

			const btVector3& getPositionWorldOnB() const
			{
				return m_positionWorldOnB;
			}

			void	setDistance(btScalar dist)
			{
				m_distance1 = dist;
			}
			
			///this returns the most recent applied impulse, to satisfy contact constraints by the constraint solver
			btScalar	getAppliedImpulse() const
			{
				return m_appliedImpulse;
			}

			

	};

#endif //BT_MANIFOLD_CONTACT_POINT_H
