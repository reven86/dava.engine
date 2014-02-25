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




#ifndef __BULLET_OBJECT_H__
#define __BULLET_OBJECT_H__

#include "DAVAEngine.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#include "Scene3d/UserNode.h"

using namespace DAVA;

class BulletObject : public BaseObject
{
protected:
    ~BulletObject();
public:
    
    BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *_meshNode, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, UserNode *_userNode, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const AABBox3 &b, const Matrix4 &pWorldTransform);
	
	void UpdateCollisionObject(void);
	
	inline btCollisionObject * GetCollisionObject(void)
	{
		return collisionObject;
	}

	void Draw(const Matrix4 & worldTransform, MeshInstanceNode * node);

	inline void SetUpdateFlag(bool flag)
	{
		updateFlag = flag;
	}
	
protected:

	void DeleteCollisionObject();	
	void CreateCollisionObject();

	void CreateLightObject(float32 radius);
	void CreateBoxObject();
	void CreateFromEntity();
	void CreateFromAABox();

	Entity * entity;
	btCollisionWorld *collWorld;
	Matrix4 *collisionPartTransform;
	btCollisionObject *collisionObject;
    btTriangleMesh* trimesh;
	btCollisionShape * shape;
	Matrix4 createdWith;
	MeshInstanceNode * meshNode;
	UserNode * userNode;
	AABBox3 * box;
	bool updateFlag;
};

#endif