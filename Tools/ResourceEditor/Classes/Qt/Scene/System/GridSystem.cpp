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


#include "Scene/System/GridSystem.h"
#include "Qt/Settings/SettingsManager.h"

// framework
#include "Render/RenderHelper.h"

#define LOWEST_GRID_STEP 0.1f
#define LOWEST_GRID_SIZE 1.0f

SceneGridSystem::SceneGridSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{
}

SceneGridSystem::~SceneGridSystem()
{

}

void SceneGridSystem::Process(float timeElapsed)
{

}


void SceneGridSystem::Draw()
{
    float gridStep = SettingsManager::GetValue(Settings::Scene_GridStep).AsFloat();
	float gridMax = SettingsManager::GetValue(Settings::Scene_GridSize).AsFloat();

    if(gridStep >= LOWEST_GRID_STEP && gridMax >= LOWEST_GRID_SIZE)
    {
	    for(DAVA::float32 x = -gridMax; x <= gridMax; x += gridStep)
	    {
		    DAVA::Vector3 v1(x, -gridMax, 0);
		    DAVA::Vector3 v2(x, gridMax, 0);
		
		    DAVA::Vector3 v3(-gridMax, x, 0);
		    DAVA::Vector3 v4(gridMax, x, 0);
		
		    if (x!= 0.0f)
		    {
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, DAVA::Color(0.4f, 0.4f, 0.4f, 1.0f));
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v4, DAVA::Color(0.4f, 0.4f, 0.4f, 1.0f));
            }
	    }

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(DAVA::Vector3(-gridMax, 0, 0), DAVA::Vector3(gridMax, 0, 0), DAVA::Color(0.0f, 0.0f, 0.0f, 1.0f));
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(DAVA::Vector3(0, -gridMax, 0), DAVA::Vector3(0, gridMax, 0), DAVA::Color(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

void SceneGridSystem::ProcessCommand(const Command2 *command, bool redo)
{

}
