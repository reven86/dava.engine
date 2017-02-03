#pragma once

#include "UIControls/ExclusiveSet.h"
#include "UIControls/BinaryExclusiveSet.h"

#include <UI/UIControl.h>
#include <UI/UIList.h>
#include <UI/UIButton.h>
#include <Render/2D/Font.h>

class QualitySettingsDialogDelegate
{
public:
    virtual void OnQualitySettingsEditDone() = 0;
};

namespace DAVA
{
class Scene;
}

class QualitySettingsDialog final
: public DAVA::UIControl
  ,
  public DAVA::UIListDelegate
  ,
  public ExclusiveSetListener
{
public:
    QualitySettingsDialog();

    void SetParentControl(UIControl*);
    void SetDelegate(QualitySettingsDialogDelegate*);

    void SetCurrentScene(DAVA::Scene*);

    void Show();

private:
    // UIListDelegate
    DAVA::float32 CellHeight(DAVA::UIList* list, DAVA::int32 index) override;
    DAVA::int32 ElementsCount(DAVA::UIList* list) override;
    DAVA::UIListCell* CellAtIndex(DAVA::UIList* list, DAVA::int32 index) override;

    // ExclusiveSetListener
    void OnOptionChanged(ExclusiveSet*);

    void OnButtonOk(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonCancel(DAVA::BaseObject* caller, void* param, void* callerData);
    void OnButtonApply(DAVA::BaseObject* caller, void* param, void* callerData);

    void BuildQualityControls();
    void ApplyQualitySettings();
    void ResetQualitySettings();

    void ApplyTextureQuality();
    void ApplyMaterialQuality();
    void ApplyParticlesQuality();
    void ReloadEntityEmitters(DAVA::Entity*);

private:
    DAVA::UIControl* parentControl = nullptr;
    QualitySettingsDialogDelegate* delegate = nullptr;

    DAVA::Scene* scene = nullptr;

    DAVA::float32 cellHeight = 0.f;
    DAVA::Font* font = nullptr;

    DAVA::Vector<DAVA::ScopedPtr<DAVA::UIListCell>> cells;

    DAVA::ScopedPtr<ExclusiveSet> textureQualityBox;
    DAVA::ScopedPtr<ExclusiveSet> anisotropyQualityBox;
    DAVA::ScopedPtr<ExclusiveSet> multisamplingQualityBox;
    DAVA::Vector<DAVA::ScopedPtr<ExclusiveSet>> materialQualityBoxes;
    DAVA::ScopedPtr<ExclusiveSet> particleQualityBox;
    DAVA::Vector<DAVA::ScopedPtr<BinaryExclusiveSet>> qualityOptionBoxes;
    DAVA::ScopedPtr<BinaryExclusiveSet> metalOptionBox;

    DAVA::UIButton* okButton = nullptr;
    DAVA::UIButton* cancelButton = nullptr;
    DAVA::UIButton* applyButton = nullptr;
};
