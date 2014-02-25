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




#include "BulletObject.h"
//#include "bullet/btBulletCollisionCommon.h"
#include "Scene3D/Components/RenderComponent.h"
#include "bullet/BulletCollision/CollisionShapes/btShapeHull.h"


BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *_meshNode, const Matrix4 &pWorldTransform)
:	collWorld(collisionWorld),
	collisionPartTransform(&((Matrix4&)pWorldTransform)),
	meshNode(_meshNode),
	updateFlag(true),
	shape(NULL),
	collisionObject(0),
	trimesh(0),
	userNode(0),
	entity(0),
	box(0)
{    
	CreateCollisionObject();
}

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, UserNode *_userNode, const Matrix4 &pWorldTransform)
	:collWorld(collisionWorld),
	collisionPartTransform(&((Matrix4&)pWorldTransform)),
	userNode(_userNode),
	updateFlag(true),
	shape(NULL),
	collisionObject(0),
	trimesh(0),
	meshNode(0),
	entity(0),
	box(0)
{
	CreateBoxObject();
}

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const Matrix4 &pWorldTransform)
:	collWorld(collisionWorld),
	collisionPartTransform(&((Matrix4&)pWorldTransform)),
	userNode(0),
	updateFlag(true),
	shape(NULL),
	collisionObject(0),
	trimesh(0),
	meshNode(0),
	entity(_entity),
	box(0)
{
	CreateFromEntity();
}

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const AABBox3 &_box, const Matrix4 &pWorldTransform)
:	collWorld(collisionWorld),
	collisionPartTransform(&((Matrix4&)pWorldTransform)),
	userNode(0),
	updateFlag(true),
	shape(NULL),
	collisionObject(0),
	trimesh(0),
	meshNode(0),
	entity(_entity),
	box(0)
{
	box = new AABBox3(_box);
	CreateFromAABox();
}


BulletObject::~BulletObject()
{
	DeleteCollisionObject();
	collWorld = 0;
	collisionPartTransform = 0;
	SafeDelete(box);
}

void BulletObject::DeleteCollisionObject()
{    
	if (collisionObject) 
	{
		collWorld->removeCollisionObject(collisionObject);
		SafeDelete(collisionObject);
    }
	SafeDelete(shape);
	SafeDelete(trimesh);	
}

void BulletObject::CreateFromEntity()
{
	bool wasPolygonGroup = false;

	RenderObject * renderObject = ((RenderComponent*)entity->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject();
	uint32 batchesCount = renderObject->GetRenderBatchCount();
	for(uint32 batchIndex = 0; batchIndex < batchesCount; ++batchIndex)
	{
		RenderBatch * batch = renderObject->GetRenderBatch(batchIndex);
		PolygonGroup * pg = batch->GetPolygonGroup();
		if(pg)
		{
			if(!wasPolygonGroup)
			{
				collisionObject = new btCollisionObject();
				trimesh = new btTriangleMesh();
				createdWith = entity->GetWorldTransform();
				wasPolygonGroup = true;
			}

			for(int32 i = 0; i < pg->indexCount / 3; i++)
			{
				uint16 index0 = pg->indexArray[i*3];
				uint16 index1 = pg->indexArray[i*3+1];
				uint16 index2 = pg->indexArray[i*3+2];
				Vector3 v;
				pg->GetCoord(index0, v);
				v = v * createdWith;
				btVector3 vertex0(v.x, v.y, v.z);
				pg->GetCoord(index1, v);
				v = v * createdWith;
				btVector3 vertex1(v.x, v.y, v.z);
				pg->GetCoord(index2, v);
				v = v * createdWith;
				btVector3 vertex2(v.x, v.y, v.z);

				trimesh->addTriangle(vertex0,vertex1,vertex2, false);
			}
		}
	}

	if(wasPolygonGroup)
	{
		shape = new btBvhTriangleMeshShape(trimesh, true, true);    
		collisionObject->setCollisionShape(shape);
		collWorld->addCollisionObject(collisionObject);
	}
}

void BulletObject::CreateCollisionObject()
{
	collisionObject = new btCollisionObject();
	
	trimesh = new btTriangleMesh();
	
	//const Vector<StaticMesh*> & meshes = meshNode->GetMeshes();
	//const Vector<int32> & indexes = meshNode->GetPolygonGroupIndexes();
	const Vector<PolygonGroupWithMaterial*> & polygroups = meshNode->GetPolygonGroups();
    
    
	uint32 meshesSize = (uint32)polygroups.size();

	createdWith = meshNode->GetWorldTransform();
	if (meshesSize > 0)
	{
		for (uint32 k = 0; k < meshesSize; ++k)
		{
			PolygonGroup * pg = polygroups[k]->GetPolygonGroup();    
			
			int i;
			for (i = 0; i < pg->indexCount / 3; i++)
			{
				uint16 index0 = pg->indexArray[i*3];
				uint16 index1 = pg->indexArray[i*3+1];
				uint16 index2 = pg->indexArray[i*3+2];
				Vector3 v;
				pg->GetCoord(index0, v);
				v = v * createdWith;
				btVector3 vertex0(v.x, v.y, v.z);
				pg->GetCoord(index1, v);
				v = v * createdWith;
				btVector3 vertex1(v.x, v.y, v.z);
				pg->GetCoord(index2, v);
				v = v * createdWith;
				btVector3 vertex2(v.x, v.y, v.z);
				
				trimesh->addTriangle(vertex0,vertex1,vertex2, false);
			}
		}
		shape = new btBvhTriangleMeshShape(trimesh, true, true);    

        collisionObject->setCollisionShape(shape);
        collWorld->addCollisionObject(collisionObject);
	}
	
}

void BulletObject::CreateLightObject(float32 radius)
{
	collisionObject = new btCollisionObject();
	shape = new btSphereShape(radius);
	collisionObject->setCollisionShape(shape);
	collWorld->addCollisionObject(collisionObject);
}

void BulletObject::CreateBoxObject()
{
	createdWith = userNode->GetWorldTransform();
	collisionObject = new btCollisionObject();
	shape = new btBoxShape(btVector3(userNode->drawBox.max.x, userNode->drawBox.max.y, userNode->drawBox.max.z));
	collisionObject->setCollisionShape(shape);

	btTransform trans;
	trans.setIdentity();
	trans.setFromOpenGLMatrix(createdWith.data);
	collisionObject->setWorldTransform(trans);
	
	collWorld->addCollisionObject(collisionObject);
}

void BulletObject::CreateFromAABox()
{
	createdWith = entity->GetWorldTransform();
	collisionObject = new btCollisionObject();
	shape = new btBoxShape(btVector3(box->max.x, box->max.y, box->max.z));
	collisionObject->setCollisionShape(shape);

	btTransform trans;
	trans.setIdentity();
	trans.setFromOpenGLMatrix(createdWith.data);
	collisionObject->setWorldTransform(trans);

	collWorld->addCollisionObject(collisionObject);
}

void BulletObject::UpdateCollisionObject()
{
	if (!updateFlag)
		return;

	if (!(*collisionPartTransform == createdWith))
	{
		DeleteCollisionObject();

		if(box)
			CreateFromAABox();
		else if(entity)
			CreateFromEntity();
		else if(meshNode)
			CreateCollisionObject();
		else if(userNode)
			CreateBoxObject();
	}
	
	/*
    btTransform btt;
    btt.setIdentity();
	
	//scale
	Vector3 scale = collisionPartTransform->GetScaleVector();
	shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
	
	//origin
	Vector3 origin = collisionPartTransform->GetTranslationVector();
	btt.setOrigin(btVector3(origin.x, origin.y, origin.z));
	
	//rotation
	Quaternion qt;
	qt.Construct(*collisionPartTransform);
	btt.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
					
    btt.setFromOpenGLMatrix(collisionPartTransform->data);
	collisionObject->setWorldTransform(btt);
	*/
}

void BulletObject::Draw(const Matrix4 & worldTransform, MeshInstanceNode * node)
{
//	btTransform tr = collisionObject->getWorldTransform();
//	btVector3 min, max;
//	shape->getAabb(tr, min, max);
//	AABBox3 bbox(Vector3(min.x(), min.y(), min.z()),
//				 Vector3(max.x(), max.y(), max.z()));	
//	
//
//	RenderHelper::Instance()->DrawBox(bbox);

	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 finalMatrix = worldTransform * prevMatrix;
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

	RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
	
//	int sz = triangles.size() / 3;
//	for (int i = 0; i < sz; i++)
//	{
//		Vector3 & p0 = triangles[i*3];
//		Vector3 & p1 = triangles[i*3 + 1];
//		Vector3 & p2 = triangles[i*3 + 2];
//		RenderHelper::Instance()->DrawLine(p0, p1);
//		RenderHelper::Instance()->DrawLine(p1, p2);
//		RenderHelper::Instance()->DrawLine(p0, p2);
//	}

//	AABBox3 bbox1 = node->GetBoundingBox();
//	RenderHelper::Instance()->DrawBox(bbox1);
	
	
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);	
	
}




