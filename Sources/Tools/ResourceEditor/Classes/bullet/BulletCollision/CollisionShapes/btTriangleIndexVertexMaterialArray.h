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

///This file was created by Alex Silverman

#ifndef BT_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H
#define BT_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H

#include "btTriangleIndexVertexArray.h"


ATTRIBUTE_ALIGNED16( struct)	btMaterialProperties
{
    ///m_materialBase ==========> 2 btScalar values make up one material, friction then restitution
    int m_numMaterials;
    const unsigned char * m_materialBase;
    int m_materialStride;
    PHY_ScalarType m_materialType;
    ///m_numTriangles <=========== This exists in the btIndexedMesh object for the same subpart, but since we're
    ///                           padding the structure, it can be reproduced at no real cost
    ///m_triangleMaterials =====> 1 integer value makes up one entry
    ///                           eg: m_triangleMaterials[1] = 5; // This will set triangle 2 to use material 5
    int m_numTriangles; 
    const unsigned char * m_triangleMaterialsBase;
    int m_triangleMaterialStride;
    ///m_triangleType <========== Automatically set in addMaterialProperties
    PHY_ScalarType m_triangleType;
};

typedef btAlignedObjectArray<btMaterialProperties>	MaterialArray;

///Teh btTriangleIndexVertexMaterialArray is built on TriangleIndexVertexArray
///The addition of a material array allows for the utilization of the partID and
///triangleIndex that are returned in the ContactAddedCallback.  As with
///TriangleIndexVertexArray, no duplicate is made of the material data, so it
///is the users responsibility to maintain the array during the lifetime of the
///TriangleIndexVertexMaterialArray.
ATTRIBUTE_ALIGNED16(class) btTriangleIndexVertexMaterialArray : public btTriangleIndexVertexArray
{
protected:
    MaterialArray       m_materials;
		
public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

    btTriangleIndexVertexMaterialArray()
	{
	}

    btTriangleIndexVertexMaterialArray(int numTriangles,int* triangleIndexBase,int triangleIndexStride,
        int numVertices,btScalar* vertexBase,int vertexStride,
        int numMaterials, unsigned char* materialBase, int materialStride,
        int* triangleMaterialsBase, int materialIndexStride);

    virtual ~btTriangleIndexVertexMaterialArray() {}

    void	addMaterialProperties(const btMaterialProperties& mat, PHY_ScalarType triangleType = PHY_INTEGER)
    {
        m_materials.push_back(mat);
        m_materials[m_materials.size()-1].m_triangleType = triangleType;
    }

    virtual void getLockedMaterialBase(unsigned char **materialBase, int& numMaterials, PHY_ScalarType& materialType, int& materialStride,
        unsigned char ** triangleMaterialBase, int& numTriangles, int& triangleMaterialStride, PHY_ScalarType& triangleType ,int subpart = 0);

    virtual void getLockedReadOnlyMaterialBase(const unsigned char **materialBase, int& numMaterials, PHY_ScalarType& materialType, int& materialStride,
        const unsigned char ** triangleMaterialBase, int& numTriangles, int& triangleMaterialStride, PHY_ScalarType& triangleType, int subpart = 0);

}
;

#endif //BT_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H
