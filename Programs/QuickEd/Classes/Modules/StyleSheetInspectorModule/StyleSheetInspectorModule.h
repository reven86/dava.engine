#pragma once

#include "Model/PackageHierarchy/PackageListener.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <Base/RefPtr.h>

namespace DAVA
{
class UIControl;
}
class QListWidget;

class StyleSheetInspectorModule : public DAVA::TArc::ClientModule, public PackageListener
{
public:
    StyleSheetInspectorModule();
    ~StyleSheetInspectorModule() override;

private:
    void PostInit() override;

    void InitUI();
    void AddListener();
    void InitFieldBinder();

    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;
    void StyleSheetsWereRebuilt() override;

    void OnSelectionChanged(const DAVA::Any& selectionValue);

    void Update();

    QListWidget* listWidget = nullptr;
    DAVA::RefPtr<DAVA::UIControl> currentControl;

    ContinuousUpdater updater;
    std::unique_ptr<DAVA::TArc::FieldBinder> selectionFieldBinder;

    DAVA_VIRTUAL_REFLECTION(SpritesPackerModule, DAVA::TArc::ClientModule);
};

