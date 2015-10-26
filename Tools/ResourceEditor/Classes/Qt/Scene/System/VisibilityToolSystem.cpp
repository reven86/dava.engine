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


#include "VisibilityToolSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "Deprecated/EditorConfig.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneHelper.h"
#include "Commands2/VisibilityToolActions.h"

#include "Render/Material/NMaterialNames.h"

const Vector2 CROSS_TEXTURE_SIZE = Vector2(32.0f, 32.0f);

VisibilityToolSystem::VisibilityToolSystem(Scene* scene)
    : LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
    , textureLevel(Landscape::TEXTURE_COLOR)
{
    crossTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/setPointCursorWhite.png");
    crossTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
}

VisibilityToolSystem::~VisibilityToolSystem()
{
	SafeRelease(crossTexture);
}

LandscapeEditorDrawSystem::eErrorType VisibilityToolSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}
	
	LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
	if ( canBeEnabledError!= LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return canBeEnabledError;
	}
	
	LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
	if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}

    SetState(State::NotActive);

    selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	Texture* visibilityToolTexture = drawSystem->GetVisibilityToolProxy()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetToolTexture(visibilityToolTexture, false);
    landscapeSize = static_cast<float>(visibilityToolTexture->GetWidth());
    textureSize = static_cast<uint32>(landscapeSize + 0.5f);
    pointsRowSize = textureSize / textureStepSizeX;
    totalRowsInPoints = textureSize / textureStepSizeY;

    drawSystem->EnableCursor();
	drawSystem->SetCursorSize(0);

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool VisibilityToolSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

    SetState(State::NotActive);

    selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();

	drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, false);

	enabled = false;
	return !enabled;
}

void VisibilityToolSystem::Process(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
		return;

    if (state == State::ComputingVisibility)
    {
        if (remainingVisibilityTests.empty())
        {
            DrawResults();
            SetState(State::NotActive);
        }
        else
        {
            ProcessNextVisibilityTests();
        }
    }
    else if (editingIsEnabled && isIntersectsLandscape)
    {
		if (prevCursorPos != cursorPosition)
		{
			prevCursorPos = cursorPosition;
		}
	}
}

void VisibilityToolSystem::Input(DAVA::UIEvent *event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	UpdateCursorPosition();

    if (state != State::AddingPoint)
    {
		return;
	}

	if (UIEvent::PHASE_KEYCHAR == event->phase)
	{
		if (event->tid == DVKEY_ESCAPE)
		{
            CancelAddingCheckPoint();
        }
	}
	else if (event->tid == UIEvent::BUTTON_1)
	{
		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
            {
                if (isIntersectsLandscape)
				{
					editingIsEnabled = true;
				}
				break;
            }

            case UIEvent::PHASE_ENDED:
            {
                if (editingIsEnabled)
				{
                    if (state == State::AddingPoint)
                    {
                        CommitLatestCheckPoint();
                    }
					editingIsEnabled = false;
				}
				break;
            }

            default:
                break;
        }
	}
}

void VisibilityToolSystem::SetState(State newState)
{
    if (state == newState)
        state = State::NotActive;
    else
        state = newState;

    if (state == State::AddingPoint)
    {
        drawSystem->SetCursorTexture(crossTexture);
        drawSystem->SetCursorSize(Max(CROSS_TEXTURE_SIZE.x, CROSS_TEXTURE_SIZE.y) / landscapeSize);
    }
    else
    {
        drawSystem->SetCursorSize(0);
    }

    if (newState == State::ComputingVisibility)
    {
        SetVisibilityAreaInternal();
    }

    SceneSignals::Instance()->EmitVisibilityToolStateChanged(dynamic_cast<SceneEditor2*>(GetScene()), state);
}

inline float randf()
{
    return float(rand()) / float(RAND_MAX);
}

uint32 VisibilityToolSystem::StartAddingVisibilityPoint()
{
    SetState(State::AddingPoint);
    checkPoints.emplace_back();

    Color newColor(0.125f + 0.125f * randf(), 0.125f + 0.125f * randf(), 0.125f + 0.125f * randf(), 1.0f);
    newColor.color[checkPoints.size() % 3] += 0.25f + 0.25f * randf();
    checkPoints.back().color = newColor;

    return static_cast<uint32>(checkPoints.size() - 1);
}

void VisibilityToolSystem::CancelAddingCheckPoint()
{
    DVASSERT(checkPoints.size() > 0);

    checkPoints.pop_back();
    SetState(State::NotActive);
}

void VisibilityToolSystem::ComputeVisibilityArea()
{
    SetState(State::ComputingVisibility);
}

void VisibilityToolSystem::CommitLatestCheckPoint()
{
    VisibilityToolProxy* visibilityToolProxy = drawSystem->GetVisibilityToolProxy();
    visibilityToolProxy->UpdateVisibilityPointSet(true);
    visibilityToolProxy->SetVisibilityPoint(cursorPosition);

    Vector2 xy = drawSystem->TexturePointToLandscapePoint(textureLevel, landscapeSize * cursorPosition);
    float z = drawSystem->GetHeightAtTexturePoint(textureLevel, landscapeSize * cursorPosition) + 2.5f;

    CheckPoint& point = checkPoints.back();
    point.relativePosition = cursorPosition;
    point.worldPosition = Vector3(xy, z);
    point.result.resize(pointsRowSize * totalRowsInPoints);
    point.numPoints = 0;

    RenderSystem2D::Instance()->BeginRenderTargetPass(visibilityToolProxy->GetTexture(), true);
    for (const auto& pt : checkPoints)
    {
        Rect drawRect(pt.relativePosition * landscapeSize - 0.5f * CROSS_TEXTURE_SIZE, CROSS_TEXTURE_SIZE);
        RenderSystem2D::Instance()->DrawTexture(crossTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, drawRect);
    }
    RenderSystem2D::Instance()->EndRenderTargetPass();

    SetState(State::NotActive);
}

void VisibilityToolSystem::SetVisibilityAreaInternal()
{
    if (checkPoints.empty())
    {
        SetState(State::NotActive);
        return;
	}

    remainingVisibilityTests.reserve(checkPoints.size() * textureSize * textureSize);

    for (uint32 y = 0; y < textureSize; y += textureStepSizeY)
    {
        for (uint32 x = 0; x < textureSize; x += textureStepSizeX)
        {
            for (uint32 i = 0, e = checkPoints.size(); i < e; ++i)
            {
                remainingVisibilityTests.emplace_back(i, Vector2(float(x), float(y)));
            }
        }
    }

    for (auto& p : checkPoints)
    {
        p.numPoints = 0;
    }

    totalVisibilityTests = remainingVisibilityTests.size();
}

void VisibilityToolSystem::ExcludeEntities(EntityGroup *entities) const
{
    if (!entities || (entities->Size() == 0)) return;
    
    uint32 count = entities->Size();
    while(count)
    {
        Entity *object = entities->GetEntity(count - 1);
        bool needToExclude = false;

        KeyedArchive * customProps = GetCustomPropertiesArchieve(object);
        if(customProps)
        {   // exclude not collised by bullet objects
            const int32 collisiontype = customProps->GetInt32( "CollisionType", 0 );
            if(     (ResourceEditor::ESOT_TREE == collisiontype)
                ||  (ResourceEditor::ESOT_BUSH == collisiontype)
                ||  (ResourceEditor::ESOT_FRAGILE_PROJ_INV == collisiontype)
                ||  (ResourceEditor::ESOT_FALLING == collisiontype)
                ||  (ResourceEditor::ESOT_SPEED_TREE == collisiontype)
                )
            {
                needToExclude = true;
            }
        }
        
        if(!needToExclude)
        {
            RenderObject *ro = GetRenderObject(object);
            if(ro)
            {
                switch (ro->GetType())
                {
                    case RenderObject::TYPE_LANDSCAPE:
                    case RenderObject::TYPE_SPEED_TREE:
                    case RenderObject::TYPE_SPRITE:
                    case RenderObject::TYPE_VEGETATION:
                    case RenderObject::TYPE_PARTICLE_EMTITTER:
                        needToExclude = true;
                        break;
                        
                    default:
                        break;
                }
            }
        }

        if(!needToExclude)
        {   // exclude sky
            
            Vector<NMaterial *> materials;
            SceneHelper::EnumerateMaterialInstances(object, materials);
            
            const uint32 matCount = materials.size();
            for(uint32 m = 0; m < matCount; ++m)
            {
                NMaterial * material = materials[m];
                while (material && !material->GetEffectiveFXName().IsValid())
                    material = material->GetParent();

                if (material)
                {
                    if ((NMaterialName::SKYOBJECT == material->GetEffectiveFXName()))
                    {
                        needToExclude = true;
                        break;
                    }
                }
            }
        }

        if(needToExclude)
        {
            entities->Rem(object);
        }
        
        --count;
    }
}

void VisibilityToolSystem::Draw()
{
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    for (const auto& point : checkPoints)
    {
        drawer->DrawIcosahedron(point.worldPosition, 0.5f, Color::White,
                                RenderHelper::eDrawType::DRAW_WIRE_DEPTH);
    }
}

void VisibilityToolSystem::DrawResults()
{
    auto rs = RenderSystem2D::Instance();
    auto vtp = drawSystem->GetVisibilityToolProxy();
    rs->BeginRenderTargetPass(vtp->GetTexture(), true);

    DrawVisibilityAreaPoints();

    for (const auto& point : checkPoints)
    {
        DrawVisibilityAreaMark(point);
    }
    rs->EndRenderTargetPass();
}

void VisibilityToolSystem::DrawVisibilityAreaPoints()
{
    uint32 numPoints = checkPoints.front().numPoints;
    for (const auto& p : checkPoints)
    {
        DVASSERT(p.numPoints == numPoints);
        numPoints = p.numPoints;
    }

    auto multiplyColor = [](Color& cIn, const Color& cOver) {
        cIn.r = 1.0f - (1.0f - cIn.r) * (1.0f - cOver.r);
        cIn.g = 1.0f - (1.0f - cIn.g) * (1.0f - cOver.g);
        cIn.b = 1.0f - (1.0f - cIn.b) * (1.0f - cOver.b);
        cIn.a = Max(cIn.a, cOver.a);
    };

    auto rs = RenderSystem2D::Instance();
    auto vtp = drawSystem->GetVisibilityToolProxy();
    float dx = static_cast<float>(textureStepSizeX);
    float dy = static_cast<float>(textureStepSizeX);

    for (uint32 i = 0; i < numPoints; ++i)
    {
        uint32 c0 = i % pointsRowSize;
        uint32 r0 = i / pointsRowSize;
        uint32 c1 = Min(c0 + 1, pointsRowSize - 1);
        uint32 r1 = Min(r0 + 1, totalRowsInPoints - 1);
        DVASSERT(c0 + r0 * pointsRowSize == i);

        Color c00 = Color(0.0f, 0.0f, 0.0f, 0.0f);
        Color c01 = Color(0.0f, 0.0f, 0.0f, 0.0f);
        Color c10 = Color(0.0f, 0.0f, 0.0f, 0.0f);
        Color c11 = Color(0.0f, 0.0f, 0.0f, 0.0f);

        int x = 0;
        int y = 0;

        for (const auto& p : checkPoints)
        {
            const auto& p00 = p.result[c0 + r0 * pointsRowSize];
            const auto& p01 = p.result[c1 + r0 * pointsRowSize];
            const auto& p10 = p.result[c0 + r1 * pointsRowSize];
            const auto& p11 = p.result[c1 + r1 * pointsRowSize];
            multiplyColor(c00, p00.second);
            multiplyColor(c01, p01.second);
            multiplyColor(c10, p10.second);
            multiplyColor(c11, p11.second);
            x = p00.first.x;
            y = p00.first.y;
        }

        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        rs->FillGradientRect(Rect(fx, fy, dx, dy), c00, c01, c10, c11);
    }
}

void VisibilityToolSystem::DrawVisibilityAreaMark(const CheckPoint& point)
{
    Rect pointRect(point.relativePosition * landscapeSize - 0.5f * CROSS_TEXTURE_SIZE, CROSS_TEXTURE_SIZE);
    RenderSystem2D::Instance()->DrawTexture(crossTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, pointRect);
}

void VisibilityToolSystem::SaveTexture(const FilePath& filePath)
{
	if (filePath.IsEmpty())
	{
		return;
	}
    
    Texture* visibilityToolTexture = drawSystem->GetVisibilityToolProxy()->GetTexture();

	Image* image = visibilityToolTexture->CreateImageFromMemory();
    ImageSystem::Instance()->Save(filePath, image);
    SafeRelease(image);
}

VisibilityToolSystem::State VisibilityToolSystem::GetState()
{
	return state;
}

int32 VisibilityToolSystem::GetProgress()
{
    if (totalVisibilityTests == 0)
        return 0;

    float aspect = 1.0f - float(remainingVisibilityTests.size()) / float(totalVisibilityTests);
    return static_cast<int>(100.0f * aspect + 0.5f);
}

void VisibilityToolSystem::PerformVisibilityTest(const VisibilityTests::value_type& test)
{
    auto& sourcePoint = checkPoints.at(test.first);

    float32 baseZ = drawSystem->GetHeightAtTexturePoint(textureLevel, test.second) + 0.25f; // TODO: use Z from point
    Vector3 target(drawSystem->TexturePointToLandscapePoint(textureLevel, test.second), baseZ);

    Color resultColor = Color(0.0f, 0.0f, 0.0f, 0.0f);

    Vector3 direction = target - sourcePoint.worldPosition;
    float32 angle = atan2(direction.z, direction.xy().Length());

    bool angleInRange = ((angle >= 0.0f) && (angle <= sourcePoint.angleUp)) || ((angle <= 0.0f) && (angle >= -sourcePoint.angleDown));

    if (angleInRange)
    {
        const EntityGroup* intersectedObjects = collisionSystem->ObjectsRayTest(sourcePoint.worldPosition, target);
        EntityGroup entityGroup(*intersectedObjects);
        ExcludeEntities(&entityGroup);

        if (entityGroup.Size() == 0)
        {
            Vector3 p;
            if (collisionSystem->LandRayTest(sourcePoint.worldPosition, target, p) == false)
            {
                resultColor = sourcePoint.color;
            }
        }
    }

    sourcePoint.result[sourcePoint.numPoints++] = { Point2i(int(test.second.x), int(test.second.y)), resultColor };
}

void VisibilityToolSystem::ProcessNextVisibilityTests()
{
    uint32 testsToPerform = Min(4096u + rand() % 4096u, remainingVisibilityTests.size());
    for (uint32_t i = 0; i < testsToPerform; ++i)
    {
        PerformVisibilityTest(remainingVisibilityTests.at(i));
    }
    remainingVisibilityTests.erase(remainingVisibilityTests.begin(), remainingVisibilityTests.begin() + testsToPerform);
}

void VisibilityToolSystem::CancelComputing()
{
    SetState(State::NotActive);
    remainingVisibilityTests.clear();
}
