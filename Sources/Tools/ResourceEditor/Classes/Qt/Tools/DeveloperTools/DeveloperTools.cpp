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



#include "DeveloperTools.h"
#include "Main/mainwindow.h"
#include "Scene/System/SelectionSystem.h"

#include "DAVAEngine.h"
using namespace DAVA;

DeveloperTools::DeveloperTools(QObject *parent /* = 0 */)
    : QObject(parent)
{

}

void DeveloperTools::OnDebugFunctionsGridCopy()
{
    SceneEditor2 * currentScene = QtMainWindow::Instance()->GetCurrentScene();
    float32 x = 0;
    float32 y = 0;
    float32 z = 0;
    const float32 xshift = 10.0;
    const float32 yshift = 10.0;
    const float32 zshift = 0.0;

    if (currentScene->selectionSystem->GetSelectionCount() == 1)
    {
        Entity * entity = currentScene->selectionSystem->GetSelectionEntity(0);

        const Matrix4 & matrix = entity->GetLocalTransform();

        for (uint32 x = 0; x < 10; ++x)
        {
            for (uint32 y = 0; y < 10; ++y)
            {
                Matrix4 translation;
                translation.CreateTranslation(Vector3(x * xshift, y * yshift, z * zshift));

                Matrix4 newMatrix = matrix * translation;
                Entity * clonedEntity = entity->Clone();
                clonedEntity->SetLocalTransform(newMatrix);

                RenderObject * renderObject = GetRenderObject(clonedEntity);
                NMaterial * material = renderObject->GetRenderBatch(0)->GetMaterial();
                float32 inGlossiness = (float32)x / 9.0;
                float32 inSpecularity = (float32)y / 9.0;
                material->SetPropertyValue(FastName("inGlossiness"), Shader::UT_FLOAT, 1, &inGlossiness);
                material->SetPropertyValue(FastName("inSpecularity"), Shader::UT_FLOAT, 1, &inSpecularity);

                currentScene->AddNode(clonedEntity);
            }
        }
    }
}
