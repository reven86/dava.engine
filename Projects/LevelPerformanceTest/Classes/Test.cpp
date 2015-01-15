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


#include "Test.h"
#include "SettingsManager.h"
#include "AppScreens.h"
#include "TextureHelper.h"

#define DEF_CAMERA_DIRECTION Vector3(1.f, 0.f, 0.f)

int32 Test::globalScreenId=PREDEFINED_SCREENS_COUNT;

Test::Test()
{
}

Test::Test(const FilePath& fullName)
{
	this->fullName = fullName;
	
	screenId = globalScreenId++;

	camRotateAnimation = 0;
}

void Test::LoadResources()
{
	isFinished = false;
	skipFrames = 100;

	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(fullName));
	DVASSERT_MSG(scene, "Could not load the scene");

	Camera* cam = new Camera();
	scene->AddCamera(cam);

	Core* core = DAVA::Core::Instance();
	float32 aspect = core->GetVirtualScreenHeight() / core->GetVirtualScreenWidth();

	cam->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
	cam->SetLeft(Vector3(1, 0, 0));
	cam->SetUp(Vector3(0, 0, 1));
    
    scene->SetCurrentCamera(cam);
	SafeRelease(cam);

	UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
	sceneView->SetScene(scene);
	AddControl(sceneView);
	SafeRelease(sceneView);

	Landscape* landscape = GetLandscape();
	DVASSERT_MSG(landscape, "There is no landscape in a scene");

	uint32 textureMemory = TextureHelper::GetSceneTextureMemory(scene);
	testData.SetTextureMemorySize(textureMemory);

	uint32 textureFilesSize = TextureHelper::GetSceneTextureFilesSize(scene);
	testData.SetTexturesFilesSize(textureFilesSize);

	testData.SetSceneFilePath(fullName);

	ZeroCurFpsStat();
	PreparePath();
    PrepareFpsStat();
    PrepareCameraPosition();
    MoveToNextPoint();

    SafeRelease(scene);
}

void Test::UnloadResources()
{
	RemoveAllControls();
}

void Test::WillAppear()
{
}

void Test::WillDisappear()
{
}

void Test::Input(UIEvent * event)
{
}

void Test::OnSectorCameraAnimationEnded(BaseObject* caller, void* userData, void* callerData)
{
	if(caller)
	{
		fpsStatItem.avFps[curSectorIndex] = curSectorFrames / curSectorTime;
		curSectorIndex++;
		curSectorFrames = 0;
		curSectorTime = 0.f;
	}

	if(curSectorIndex < SECTORS_COUNT)
	{
		float32 timeToRotate = 45.f / SettingsManager::Instance()->GetCameraRotationSpeed();
		camRotateAnimation = new LinearAnimation<float32>(this, &curCameraAngle, curCameraAngle + 45.f, timeToRotate, Interpolation::LINEAR);
		camRotateAnimation->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &Test::OnSectorCameraAnimationEnded));
		camRotateAnimation->Start(1);
	}
	else
	{
		SaveFpsStat();
		ZeroCurFpsStat();
		if(MoveToNextPoint())
			OnSectorCameraAnimationEnded(0, 0, 0);
		else
			isFinished = true;
	}
}

void Test::Update(float32 timeElapsed)
{
	if(skipFrames)
	{
		if(!--skipFrames)
		{
			OnSectorCameraAnimationEnded(0, 0, 0);
		}
	}
	else
	{
		curSectorFrames++;
		curSectorTime += timeElapsed;
	}

	Camera* cam = GetCamera();

	Vector3 newCamTarget = DEF_CAMERA_DIRECTION * Matrix4::MakeRotation(Vector3(0.f, 0.f, 1.f), DegToRad(curCameraAngle));
	newCamTarget += curCameraPosition;
	cam->SetTarget(newCamTarget);

	UIScreen::Update(timeElapsed);
}

void Test::SaveFpsStat()
{
	testData.AddStatItem(fpsStatItem);
}

void Test::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void Test::PreparePath()
{
	SettingsManager *settings = SettingsManager::Instance();
	
    Landscape *land = GetLandscape();
	AABBox3 boundingBox = land->GetBoundingBox();
	Vector3 min = boundingBox.min;
	Vector3 max = boundingBox.max;

	Vector2 rectSize = settings->GetLandscapePartitioningSize();

	float32 landWidth = max.x - min.x;
	float32 landLength = max.y - min.y;

	int32 partX = (int32)ceilf(landWidth / rectSize.x);
	int32 partY = (int32)ceilf(landLength / rectSize.y);
	
	int32 x = 0;
	int32 xDir = 1;
	for(int32 y = 0; y < partY; ++y)
    {
		Rect curRect;
		for(int32 i = 0; i < partX; ++i, x += xDir)
        {
			Vector2 v;
			v.Set(min.x + x * rectSize.x, min.y + y * rectSize.y);
			curRect.SetPosition(v);
			curRect.SetSize(rectSize);
            rectSequence.push_back(curRect);
		}
		x -= xDir;
		xDir = -xDir;
	}
}

bool Test::MoveToNextPoint()
{
    if(nextRectNum < rectSequence.size())
    {
		Rect nextRect = rectSequence[nextRectNum];

		curCameraPosition = GetRealPoint(nextRect.GetCenter());
		fpsStatItem.rect = nextRect;

		Camera* cam = GetCamera();
		cam->SetPosition(curCameraPosition);

		++nextRectNum;

		return true;
    }
	return false;
}

void Test::PrepareCameraPosition()
{
    Camera* cam = GetCamera();
    cam->SetPosition(curCameraPosition);
    cam->SetDirection(DEF_CAMERA_DIRECTION);

    nextRectNum = 0;
}

void Test::PrepareFpsStat()
{
	testData.Clear();
	fpsStatItem.rect = rectSequence[testData.GetItemCount()];

    Landscape *land = GetLandscape();
	AABBox3 boundingBox = land->GetBoundingBox();
	
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
					 (boundingBox.max - boundingBox.min).x);

	testData.SetLandscapeRect(Rect(landPos, landSize));
}

Vector3 Test::GetRealPoint(const Vector2& point)
{
    Vector3 realPoint(point);

	Landscape *land = GetLandscape();
    land->PlacePoint(realPoint, realPoint);

    realPoint.z += SettingsManager::Instance()->GetCameraElevation();
    
    return realPoint;
}

const FilePath & Test::GetFilePath() const
{
	return fullName;
}

void Test::ZeroCurFpsStat()
{
	curCameraAngle = -22.5f;
	curSectorFrames = 0;
	curSectorIndex = 0;
	curSectorTime = 0.f;

	for(int i = 0; i < SECTORS_COUNT; i++)
		fpsStatItem.avFps[i] = 0;
}

inline UI3DView* Test::GetSceneView()
{
	return (UI3DView*)GetChildren().front();
}

inline Scene* Test::GetScene()
{
	return GetSceneView()->GetScene();
}

inline Camera* Test::GetCamera()
{
	return GetScene()->GetCurrentCamera();
}

inline Landscape* Test::GetLandscape()
{
	SettingsManager* settings = SettingsManager::Instance();
	Entity* landscapeNode = GetScene()->FindByName(settings->GetLandscapeNodeName());
	Landscape* landscape = NULL;
	if (landscapeNode)
	{
		RenderComponent* renderComponent = dynamic_cast<RenderComponent*>(landscapeNode->GetComponent(Component::RENDER_COMPONENT));
		if (renderComponent)
		{
			landscape = dynamic_cast<Landscape*>(renderComponent->GetRenderObject());
		}
	}

	return landscape;
}

Texture* Test::GetLandscapeTexture()
{
	Landscape* landscape = GetLandscape();
	return landscape->CreateLandscapeTexture();
};

