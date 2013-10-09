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



#include "DAVAEngine.h"
#include "GameCore.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ResourcePacker2D.h"

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneValidator.h"
#include "SceneEditor/TextureSquarenessChecker.h"

#include "CommandLine/CommandLineManager.h"
#include "CommandLine/SceneExporter/SceneExporter.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "version.h"

using namespace DAVA;


/*
    Is it possible, to store size before vector, seems doable and cache friendly, because to tranverse vector firstly you 
    need to read size.
 */
template<class T>
class CacheFriendlyVector
{
public:
    uint8 * data;
};



class EntityX
{
//    Vector<Component*> fastAccessArray;
//    Vector<Component*> allComponents;
    Component ** fastAccessArray;
    Component ** allComponents;
    EntityX ** children;
};


class EntitySTL
{
    CacheFriendlyVector<Component*> fastAccessArray;
    CacheFriendlyVector<Component*> allComponents;
    CacheFriendlyVector<EntitySTL*> children;
};

/*
    => 24 bytes for each entity. (8 * 3 = 24 = 64bit system) (12bytes on 32 bit system)
    =>
    
    Action update transform: (тут не может не быть cache misses)
 
    Entity * entity = GetEntity(); // cache miss
    TransformComponent * tc = entity->fastTransforms[COMPONENT_TRANSFORM]; // cache miss
    tc->UpdateLocalTransform(transform); // cache miss
 
 
    TransformSystem:
    for each object go to his children and add them, for them do the same, linearly
 
 */


void FrameworkDidLaunched()
{
    uint32 size = sizeof(EntityX);
    uint32 size2 = sizeof(EntitySTL);
    
    new SceneExporter();
    new EditorSettings();
	new EditorConfig();
    new SceneValidator();
    new TextureSquarenessChecker();

    
    SceneValidator::Instance()->SetPathForChecking(EditorSettings::Instance()->GetProjectPath());
    
#if defined(__DAVAENGINE_IPHONE__)
	KeyedArchive * appOptions = new KeyedArchive();
	appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    
	DAVA::Core::Instance()->SetVirtualScreenSize(480, 320);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(480, 320, "Gfx");
#else
	KeyedArchive * appOptions = new KeyedArchive();
    
    int32 width = (int32)DAVA::Core::Instance()->GetVirtualScreenWidth();
    int32 height = (int32)DAVA::Core::Instance()->GetVirtualScreenHeight();
    if(width <= 0 || height <= 0)
    {
        width = EditorSettings::Instance()->GetScreenWidth();
        height = EditorSettings::Instance()->GetScreenHeight();

        DAVA::Core::Instance()->SetVirtualScreenSize(width, height);
    }
    

	appOptions->SetString("title", Format("dava framework - resource editor | %s-%s", DAVAENGINE_VERSION, RESOURCE_EDITOR_VERSION));
	appOptions->SetInt32("width",	width);
	appOptions->SetInt32("height", height);

	appOptions->SetInt32("fullscreen", 0);
	appOptions->SetInt32("bpp", 32); 

	DAVA::Core::Instance()->RegisterAvailableResourceSize(width, height, "Gfx");
#endif
    
	GameCore * core = new GameCore();
	DAVA::Core::SetApplicationCore(core);
	DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::Core::Instance()->EnableReloadResourceOnResize(false);

	SafeRelease(appOptions);
}


void FrameworkWillTerminate()
{
	SceneValidator::Instance()->Release();
	EditorConfig::Instance()->Release();
	EditorSettings::Instance()->Release();
    TextureSquarenessChecker::Instance()->Release();
}
