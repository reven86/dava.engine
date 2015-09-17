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


#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "EntityGroup.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

namespace DAVA { class Scene; }

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
    Q_OBJECT

signals:
    // scene
    void Opened(DAVA::Scene *scene);
    void Closed(DAVA::Scene *scene);

    void Loaded(DAVA::Scene *scene);
    void Saved(DAVA::Scene *scene);

    void Activated(DAVA::Scene *scene);
    void Deactivated(DAVA::Scene *scene);

    // entities
    void SelectionChanged(DAVA::Scene *scene, const EntityGroup *selected, const EntityGroup *deselected);

    void SolidChanged(DAVA::Scene *scene, const DAVA::Entity *entity, bool value);
    // mouse
    void MouseOver(DAVA::Scene *scene, const EntityGroup *entities);
    void MouseOverSelection(DAVA::Scene *scene, const EntityGroup *entities);

    
public:
    void EmitOpened(DAVA::Scene *scene) { emit Opened(scene); }
    void EmitClosed(DAVA::Scene *scene) { emit Closed(scene); }

    void EmitLoaded(DAVA::Scene *scene) { emit Loaded(scene); }
    void EmitSaved(DAVA::Scene *scene) { emit Saved(scene); }

    void EmitActivated(DAVA::Scene *scene) { emit Activated(scene); }
    void EmitDeactivated(DAVA::Scene *scene) { emit Deactivated(scene); }

    void EmitSelectionChanged(DAVA::Scene *scene, const EntityGroup *selected, const EntityGroup *deselected) { emit SelectionChanged(scene, selected, deselected); }
    void EmitSolidChanged(DAVA::Scene *scene, const DAVA::Entity *entity, bool value) { emit SolidChanged(scene, entity, value); }

    void EmitMouseOver(DAVA::Scene *scene, const EntityGroup *entities) { emit MouseOver(scene, entities); }
    void EmitMouseOverSelection(DAVA::Scene *scene, const EntityGroup *entities) { emit MouseOverSelection(scene, entities); }
};

#endif // __SCENE_MANAGER_H__
