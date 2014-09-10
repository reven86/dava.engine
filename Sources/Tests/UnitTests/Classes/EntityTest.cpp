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


#include "EntityTest.h"

//IMPLEMENT_COMPONENT(TestVisibilityAABBoxComponent);
//IMPLEMENT_COMPONENT(TestVisibilityBSphereComponent);
//IMPLEMENT_COMPONENT(TestDrawMeshComponent);


int32 testResults[16];

//class VisibilityAABBoxSystem
//{
//public:
//	EntityManager * manager;
//
//	void Run()
//	{
//		TemplatePool<AABBox3> * boxes = manager->GetLinkedTemplatePools<AABBox3>("meshAABox");
//		while(boxes)
//		{
//			EntityFamily * family = boxes->GetEntityFamily();
//			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("meshVisibilityFlag");
//
//			int32 count = boxes->GetCount();
//			AABBox3 * box = boxes->GetPtr(0);
//			uint32 * flag = visibilityFlags->GetPtr(0);
//
//			for(int32 i = 0; i < count; ++i)
//			{
//				*flag = box->max.x > 0;
//
//				box++;
//				flag++;
//			}
//
//			boxes = boxes->next;
//		}
//	}
//};
//
//class VisibilityBSphereSystem
//{
//public:
//	EntityManager * manager;
//
//	void Run()
//	{
//		TemplatePool<Sphere> * spheres = manager->GetLinkedTemplatePools<Sphere>("meshBSphere");
//		while(spheres)
//		{
//			EntityFamily * family = spheres->GetEntityFamily();
//			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("meshVisibilityFlag");
//
//			int32 count = spheres->GetCount();
//			Sphere * sphere = spheres->GetPtr(0);
//			uint32 * flag = visibilityFlags->GetPtr(0);
//
//			for(int32 i = 0; i < count; ++i)
//			{
//				*flag = sphere->center.x > 0;
//
//				sphere++;
//				flag++;
//			}
//
//			spheres = spheres->next;
//		}
//	}
//};
//
//class DrawMeshSystem
//{
//public:
//	EntityManager * manager;
//
//	void Run()
//	{
//		TemplatePool<uint32> * visibilityFlags = manager->GetLinkedTemplatePools<uint32>("meshVisibilityFlag");
//
//		int32 * result = testResults;
//		while(visibilityFlags)
//		{
//			int32 count = visibilityFlags->GetCount();
//			uint32 * flag = visibilityFlags->GetPtr(0);
//			for(int32 i = 0; i < count; ++i)
//			{
//				*result = *flag;
//
//				result++;
//				flag++;
//			}
//
//			visibilityFlags = visibilityFlags->next;
//		}
//	}
//
//};

EntityTest::EntityTest()
:	TestTemplate<EntityTest>("Entity Test")
{

}

void EntityTest::LoadResources()
{
	RegisterFunction(this, &EntityTest::DummyComponents, "DummyComponents", 0);
}

void EntityTest::UnloadResources()
{

}

void EntityTest::DummyComponents(PerfFuncData * data)
{
//	TestVisibilityAABBoxComponent::Create();
//	TestVisibilityBSphereComponent::Create();
//	TestDrawMeshComponent::Create();
//
//	EntityManager * manager = new EntityManager();
//
//	VisibilityAABBoxSystem visibilityAABBoxSystem;
//	visibilityAABBoxSystem.manager = manager;
//
//	DrawMeshSystem drawSystem;
//	drawSystem.manager = manager;
//
//	VisibilityBSphereSystem visibilityBSphereSystem;
//	visibilityBSphereSystem.manager = manager;
//
//	Entity * entity0 = EntityManager::Instance()->CreateEntity();
//	entity0->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity0->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity1 = EntityManager::Instance()->CreateEntity();
//	entity1->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity1->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity2 = EntityManager::Instance()->CreateEntity();
//	entity2->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity2->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity3 = EntityManager::Instance()->CreateEntity();
//	entity3->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity3->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity4 = EntityManager::Instance()->CreateEntity();
//	entity4->AddComponent(TestVisibilityBSphereComponent::Get());
//	entity4->AddComponent(TestDrawMeshComponent::Get());
//	Sphere sphere4;
//	sphere4.center = Vector3(1.f, 1.f, 1.f);
//
//	Entity * entity5 = EntityManager::Instance()->CreateEntity();
//	entity5->AddComponent(TestVisibilityBSphereComponent::Get());
//	entity5->AddComponent(TestDrawMeshComponent::Get());
//	Sphere sphere5;
//	sphere5.center = Vector3(-1.f, -1.f, -1.f);
//
//	manager->Flush();
//
//	entity0->SetData("meshAABox", AABBox3(Vector3(-5.f, -5.f, -5.f), Vector3(-4.f, -4.f, -4.f)));
//	entity0->SetData("meshVisibilityFlag", (uint32)0);
//	entity1->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity1->SetData("meshVisibilityFlag", (uint32)0);
//	entity2->SetData("meshAABox", AABBox3(Vector3(-2.f, -2.f, -2.f), Vector3(-1.f, -1.f, -1.f)));
//	entity2->SetData("meshVisibilityFlag", (uint32)0);
//	entity3->SetData("meshAABox", AABBox3(Vector3(3.f, 3.f, 3.f), Vector3(4.f, 4.f, 4.f)));
//	entity3->SetData("meshVisibilityFlag", (uint32)0);
//	entity4->SetData("meshBSphere", sphere4);
//	entity4->SetData("meshVisibilityFlag", (uint32)0);
//	entity5->SetData("meshBSphere", sphere5);
//	entity5->SetData("meshVisibilityFlag", (uint32)0);
//
//	visibilityAABBoxSystem.Run();
//	visibilityBSphereSystem.Run();
//	drawSystem.Run();
//
//	TEST_VERIFY(*(entity0->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity1->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity2->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity3->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity4->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity5->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//
//	entity2->RemoveComponent(TestVisibilityAABBoxComponent::Get());
//
//	manager->Flush();
//
//	entity0->SetData("meshVisibilityFlag", (uint32)1);
//	entity1->SetData("meshVisibilityFlag", (uint32)1);
//	entity2->SetData("meshVisibilityFlag", (uint32)1);
//	entity3->SetData("meshVisibilityFlag", (uint32)1);
//	entity4->SetData("meshVisibilityFlag", (uint32)1);
//	entity5->SetData("meshVisibilityFlag", (uint32)1);
//
//	visibilityAABBoxSystem.Run();
//	visibilityBSphereSystem.Run();
//	drawSystem.Run();
//
//	TEST_VERIFY(*(entity0->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity1->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity2->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity3->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity4->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity5->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//
//	Entity * entity6 = EntityManager::Instance()->CreateEntity();
//	entity6->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity6->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity7 = EntityManager::Instance()->CreateEntity();
//	entity7->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity7->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity8 = EntityManager::Instance()->CreateEntity();
//	entity8->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity8->AddComponent(TestDrawMeshComponent::Get());;
//
//	Entity * entity9 = EntityManager::Instance()->CreateEntity();
//	entity9->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity9->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity10 = EntityManager::Instance()->CreateEntity();
//	entity10->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity10->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity11 = EntityManager::Instance()->CreateEntity();
//	entity11->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity11->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity12 = EntityManager::Instance()->CreateEntity();
//	entity12->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity12->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity13 = EntityManager::Instance()->CreateEntity();
//	entity13->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity13->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity14 = EntityManager::Instance()->CreateEntity();
//	entity14->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity14->AddComponent(TestDrawMeshComponent::Get());
//
//	Entity * entity15 = EntityManager::Instance()->CreateEntity();
//	entity15->AddComponent(TestVisibilityAABBoxComponent::Get());
//	entity15->AddComponent(TestDrawMeshComponent::Get());
//
//	manager->Flush();
//
//	entity6->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity6->SetData("meshVisibilityFlag", (uint32)0);
//	entity7->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity7->SetData("meshVisibilityFlag", (uint32)0);
//	entity8->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity8->SetData("meshVisibilityFlag", (uint32)0);
//	entity9->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity9->SetData("meshVisibilityFlag", (uint32)0);
//	entity10->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity10->SetData("meshVisibilityFlag", (uint32)0);
//	entity11->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(0.f, 1.f, 1.f)));
//	entity11->SetData("meshVisibilityFlag", (uint32)0);
//	entity12->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity12->SetData("meshVisibilityFlag", (uint32)0);
//	entity13->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity13->SetData("meshVisibilityFlag", (uint32)0);
//	entity14->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(0.f, 1.f, 1.f)));
//	entity14->SetData("meshVisibilityFlag", (uint32)0);
//	entity15->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
//	entity15->SetData("meshVisibilityFlag", (uint32)0);
//
//
//	visibilityAABBoxSystem.Run();
//	visibilityBSphereSystem.Run();
//	drawSystem.Run();
//
//	TEST_VERIFY(*(entity0->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity1->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity2->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity3->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity4->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity5->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity6->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity7->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity8->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity9->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity10->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity11->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity12->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity13->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity14->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity15->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//
//	manager->DestroyEntity(entity11);
//	manager->Flush();
//	visibilityAABBoxSystem.Run();
//	visibilityBSphereSystem.Run();
//	drawSystem.Run();
//
//	TEST_VERIFY(*(entity0->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity1->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity2->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity3->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity4->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity5->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity6->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity7->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity8->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity9->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity10->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(entity11 == 0);
//	TEST_VERIFY(*(entity12->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity13->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	TEST_VERIFY(*(entity14->GetData<uint32>("meshVisibilityFlag")) == (uint32)0);
//	TEST_VERIFY(*(entity15->GetData<uint32>("meshVisibilityFlag")) == (uint32)1);
//	manager->Dump();
//
//	SafeRelease(manager);
}
