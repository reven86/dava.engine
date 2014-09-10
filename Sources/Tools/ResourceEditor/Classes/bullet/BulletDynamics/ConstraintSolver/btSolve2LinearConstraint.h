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

#ifndef BT_SOLVE_2LINEAR_CONSTRAINT_H
#define BT_SOLVE_2LINEAR_CONSTRAINT_H

#include "LinearMath/btMatrix3x3.h"
#include "LinearMath/btVector3.h"


class btRigidBody;



/// constraint class used for lateral tyre friction.
class	btSolve2LinearConstraint
{
	btScalar	m_tau;
	btScalar	m_damping;

public:

	btSolve2LinearConstraint(btScalar tau,btScalar damping)
	{
		m_tau = tau;
		m_damping = damping;
	}
	//
	// solve unilateral constraint (equality, direct method)
	//
	void resolveUnilateralPairConstraint(		
														   btRigidBody* body0,
		btRigidBody* body1,

		const btMatrix3x3& world2A,
						const btMatrix3x3& world2B,
						
						const btVector3& invInertiaADiag,
						const btScalar invMassA,
						const btVector3& linvelA,const btVector3& angvelA,
						const btVector3& rel_posA1,
						const btVector3& invInertiaBDiag,
						const btScalar invMassB,
						const btVector3& linvelB,const btVector3& angvelB,
						const btVector3& rel_posA2,

					  btScalar depthA, const btVector3& normalA, 
					  const btVector3& rel_posB1,const btVector3& rel_posB2,
					  btScalar depthB, const btVector3& normalB, 
					  btScalar& imp0,btScalar& imp1);


	//
	// solving 2x2 lcp problem (inequality, direct solution )
	//
	void resolveBilateralPairConstraint(
			btRigidBody* body0,
						btRigidBody* body1,
		const btMatrix3x3& world2A,
						const btMatrix3x3& world2B,
						
						const btVector3& invInertiaADiag,
						const btScalar invMassA,
						const btVector3& linvelA,const btVector3& angvelA,
						const btVector3& rel_posA1,
						const btVector3& invInertiaBDiag,
						const btScalar invMassB,
						const btVector3& linvelB,const btVector3& angvelB,
						const btVector3& rel_posA2,

					  btScalar depthA, const btVector3& normalA, 
					  const btVector3& rel_posB1,const btVector3& rel_posB2,
					  btScalar depthB, const btVector3& normalB, 
					  btScalar& imp0,btScalar& imp1);

/*
	void resolveAngularConstraint(	const btMatrix3x3& invInertiaAWS,
						const btScalar invMassA,
						const btVector3& linvelA,const btVector3& angvelA,
						const btVector3& rel_posA1,
						const btMatrix3x3& invInertiaBWS,
						const btScalar invMassB,
						const btVector3& linvelB,const btVector3& angvelB,
						const btVector3& rel_posA2,

					  btScalar depthA, const btVector3& normalA, 
					  const btVector3& rel_posB1,const btVector3& rel_posB2,
					  btScalar depthB, const btVector3& normalB, 
					  btScalar& imp0,btScalar& imp1);

*/

};

#endif //BT_SOLVE_2LINEAR_CONSTRAINT_H
