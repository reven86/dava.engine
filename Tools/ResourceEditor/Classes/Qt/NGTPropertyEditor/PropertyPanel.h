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

#ifndef __RESOURCEEDITOR_PROPERTYPANEL_H__
#define __RESOURCEEDITOR_PROPERTYPANEL_H__

#include "QtTools/Updaters/LazyUpdater.h"
#include "Extensions.h"

#include "QtTools/WarningGuard/QtWarningsHandler.h"

#include <core_ui_framework/i_view.hpp>
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_reflection/reflected_object.hpp>

#include <memory>
#include <QObject>

namespace DAVA
{
class InspBase;
class InspInfo;
}

class SceneEditor2;
class SelectableGroup;
class ReflectedPropertyModel;

class PropertyPanel : public QObject, public IViewEventListener, public EntityInjectDataExtension::Delegate
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
    DECLARE_REFLECTED

public:
    PropertyPanel();
    ~PropertyPanel();

    void Initialize(IUIFramework& uiFramework, IUIApplication& uiApplication);
    void Finalize();

    ObjectHandle GetPropertyTree() const;
    void SetPropertyTree(const ObjectHandle& dummyTree);

    Q_SLOT void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SetObject(const std::vector<DAVA::InspBase*>& object);

protected:
    void timerEvent(QTimerEvent* e) override;

private:
    void onFocusIn(IView* view) override;
    void onFocusOut(IView* view) override;

    void UpdateModel();

    void StartBatch(const DAVA::String& name, DAVA::uint32 commandCount) override;
    void Exec(Command2::Pointer&& command) override;
    void EndBatch() override;

private:
    std::unique_ptr<IView> view;
    std::unique_ptr<ReflectedPropertyModel> model;

    LazyUpdater updater;

    int updateTimerId = -1;

    bool visible = false;
    bool isSelectionDirty = false;
    std::vector<DAVA::InspBase*> selectedObjects;
};

#endif // __RESOURCEEDITOR_PROPERTYPANEL_H__