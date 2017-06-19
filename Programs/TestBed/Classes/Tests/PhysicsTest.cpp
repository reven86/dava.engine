#include "Tests/PhysicsTest.h"
#include "TestBed.h"

#include <Physics/PhysicsModule.h>
#include <Physics/Private/PhysicsMath.h>
#include <Physics/PhysicsConfigs.h>

#include <UI/Update/UIUpdateComponent.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>

#include <physx/PxScene.h>
#include <physx/PxPhysics.h>
#include <physx/PxRigidActor.h>
#include <physx/PxActor.h>
#include <physx/pvd/PxPvdSceneClient.h>
#include <PxShared/foundation/PxMat44.h>
#include <PxShared/foundation/PxTransform.h>

PhysicsTest::PhysicsTest(TestBed& app)
    : BaseScreen(app, "PhysicsTest")
{
}

void PhysicsTest::LoadResources()
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();

    BaseScreen::LoadResources();
    const DAVA::EngineContext* ctx = app.GetEngine().GetContext();
    DAVA::Physics* physicsModule = ctx->moduleManager->GetModule<DAVA::Physics>();

    scene = physicsModule->CreateScene(DAVA::PhysicsSceneConfig());

    physx::PxPhysics* lowLevelPhysics = physicsModule->GetPhysics();
    physx::PxRigidBody* actor = lowLevelPhysics->createRigidDynamic(physx::PxTransform(physx::PxIDENTITY()));

    physx::PxMaterial* material = lowLevelPhysics->createMaterial(0.5f, 0.5f, 1.f);
    physx::PxCapsuleGeometry geometry(5.0f, 30.0f);
    physx::PxShape* shape = lowLevelPhysics->createShape(geometry, *material, true);
    actor->attachShape(*shape);
    actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);

    shape->release();
    material->release();

    scene->addActor(*actor);
    simulationBlock = malloc(simulationBlockSize);
}

void PhysicsTest::UnloadResources()
{
    free(simulationBlock);
    scene->release();
    scene = nullptr;
    BaseScreen::UnloadResources();
}

void PhysicsTest::Update(DAVA::float32 timeElapsed)
{
    scene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize, true);
    physx::PxU32 errorState = 0;
    scene->fetchResults(true, &errorState);
    physx::PxPvdSceneClient* client = scene->getScenePvdClient();
    if (client != nullptr)
    {
        client->updateCamera("PhysicsTestCamera", physx::PxVec3(10.0f, 10.0f, 10.0f), physx::PxVec3(0.0f, 0.0f, 1.0f), physx::PxVec3(0.0f, 0.0f, 0.0f));
    }
}
