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
Copyright (c) 2011 Advanced Micro Devices, Inc.  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


///This file was written by Erwin Coumans


#ifndef _BT_POLYHEDRAL_FEATURES_H
#define _BT_POLYHEDRAL_FEATURES_H

#include "bullet/LinearMath/btTransform.h"
#include "bullet/LinearMath/btAlignedObjectArray.h"

#define TEST_INTERNAL_OBJECTS 1


struct btFace
{
	btAlignedObjectArray<int>	m_indices;
//	btAlignedObjectArray<int>	m_connectedFaces;
	btScalar	m_plane[4];
};


class btConvexPolyhedron
{
	public:
	btConvexPolyhedron();
	virtual	~btConvexPolyhedron();

	btAlignedObjectArray<btVector3>	m_vertices;
	btAlignedObjectArray<btFace>	m_faces;
	btAlignedObjectArray<btVector3> m_uniqueEdges;

	btVector3		m_localCenter;
	btVector3		m_extents;
	btScalar		m_radius;
	btVector3		mC;
	btVector3		mE;

	void	initialize();
	bool testContainment() const;

	void project(const btTransform& trans, const btVector3& dir, btScalar& min, btScalar& max) const;
};

	
#endif //_BT_POLYHEDRAL_FEATURES_H


