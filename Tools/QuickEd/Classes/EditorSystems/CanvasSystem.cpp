#include "Base/Introspection.h"
#include "UI/UIControl.h"
#include "Base/BaseTypes.h"

#include "CanvasSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "Preferences/PreferencesRegistrator.h"

#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/ControlProperties/RootProperty.h"

using namespace DAVA;

namespace CanvasSystem_namespace
{
class ColorControl : public UIControl
{
public:
    ColorControl();
    ~ColorControl() override = default;

private:
    uint32 GetBackgroundColorIndex() const;
    void SetBackgroundColorIndex(uint32 index);

    Color GetBackgroundColor0() const;
    void SetBackgroundColor0(const Color& color);

    Color GetBackgroundColor1() const;
    void SetBackgroundColor1(const Color& color);

    Color GetBackgroundColor2() const;
    void SetBackgroundColor2(const Color& color);

    Color backgroundColor0;
    Color backgroundColor1;
    Color backgroundColor2;
    uint32 backgroundColorIndex = 0;

public:
    INTROSPECTION_EXTEND(ColorControl, UIControl,
                         PROPERTY("backgroundColor0", "Preview Widget/Background color 0", GetBackgroundColor0, SetBackgroundColor0, I_VIEW | I_EDIT | I_SAVE | I_PREFERENCE)
                         PROPERTY("backgroundColor1", "Preview Widget/Background color 1", GetBackgroundColor1, SetBackgroundColor1, I_VIEW | I_EDIT | I_SAVE | I_PREFERENCE)
                         PROPERTY("backgroundColor2", "Preview Widget/Background color 2", GetBackgroundColor2, SetBackgroundColor2, I_VIEW | I_EDIT | I_SAVE | I_PREFERENCE)
                         PROPERTY("backgroundColorIndex", "Preview Widget/Background color index", GetBackgroundColorIndex, SetBackgroundColorIndex, I_SAVE | I_PREFERENCE)
                         )

    REGISTER_PREFERENCES(ColorControl)
};

REGISTER_PREFERENCES_ON_START(ColorControl,
                              PREF_ARG("backgroundColor0", Color::Transparent),
                              PREF_ARG("backgroundColor1", Color(1.0f, 1.0f, 1.0f, 1.0f)),
                              PREF_ARG("backgroundColor2", Color(0.0f, 0.0f, 0.0f, 1.0f)),
                              PREF_ARG("backgroundColorIndex", static_cast<uint32>(0))
                              )

class GridControl : public UIControl
{
public:
    GridControl();
    ~GridControl() override = default;
    void SetSize(const Vector2& size) override;

private:
    void Draw(const UIGeometricData& geometricData) override;
    ScopedPtr<UIControl> colorControl;
};

GridControl::GridControl()
    : colorControl(new ColorControl)
{
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    background->SetSprite("~res:/Gfx/GrayGrid", 0);
    colorControl->SetName("Color control");

    UIControl::AddControl(colorControl);
}

void GridControl::SetSize(const Vector2& size)
{
    colorControl->SetSize(size);
    UIControl::SetSize(size);
}

void GridControl::Draw(const UIGeometricData& geometricData)
{
    if (0.0f != geometricData.scale.x)
    {
        float32 invScale = 1.0f / geometricData.scale.x;
        UIGeometricData unscaledGd;
        unscaledGd.scale = Vector2(invScale, invScale);
        unscaledGd.size = geometricData.size * geometricData.scale.x;
        unscaledGd.AddGeometricData(geometricData);
        UIControl::Draw(unscaledGd);
    }
}

ColorControl::ColorControl()
{
    background->SetDrawType(UIControlBackground::DRAW_FILL);
}

Color ColorControl::GetBackgroundColor0() const
{
    return backgroundColor0;
}

void ColorControl::SetBackgroundColor0(const Color& color)
{
    backgroundColor0 = color;
    if (backgroundColorIndex == 0)
    {
        background->SetColor(backgroundColor0);
    }
}

Color ColorControl::GetBackgroundColor1() const
{
    return backgroundColor1;
}

void ColorControl::SetBackgroundColor1(const Color& color)
{
    backgroundColor1 = color;
    if (backgroundColorIndex == 1)
    {
        background->SetColor(backgroundColor1);
    }
}

Color ColorControl::GetBackgroundColor2() const
{
    return backgroundColor2;
}

void ColorControl::SetBackgroundColor2(const Color& color)
{
    backgroundColor2 = color;
    if (backgroundColorIndex == 2)
    {
        background->SetColor(backgroundColor2);
    }
}

uint32 ColorControl::GetBackgroundColorIndex() const
{
    return backgroundColorIndex;
}

void ColorControl::SetBackgroundColorIndex(uint32 index)
{
    Color color;
    backgroundColorIndex = index;
    switch (index)
    {
    case 0:
        color = backgroundColor0;
        break;
    case 1:
        color = backgroundColor1;
        break;
    case 2:
        color = backgroundColor2;
        break;
    default:
        DVASSERT_MSG(false, "unsupported background index");
        return;
    }
    background->SetColor(color);
}

} //unnamed namespe

class BackgroundController final
{
public:
    BackgroundController(UIControl* nestedControl);
    ~BackgroundController() = default;
    UIControl* GetGridControl() const;
    bool IsNestedControl(const UIControl* control) const;
    void RecalculateBackgroundProperties(ControlNode* node);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from);
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/);
    void UpdateCounterpoise();
    void AdjustToNestedControl();
    static bool IsPropertyAffectBackground(AbstractProperty* property);

    Signal<> ContentSizeChanged;
    Signal<const Vector2&> RootControlPosChanged;

private:
    void CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition) const;
    void FitGridIfParentIsNested(PackageBaseNode* node);
    RefPtr<UIControl> gridControl;
    RefPtr<UIControl> counterpoiseControl;
    RefPtr<UIControl> positionHolderControl;
    UIControl* nestedControl = nullptr;
};

BackgroundController::BackgroundController(UIControl* nestedControl_)
    : gridControl(new CanvasSystem_namespace::GridControl())
    , counterpoiseControl(new UIControl())
    , positionHolderControl(new UIControl())
    , nestedControl(nestedControl_)
{
    DVASSERT(nullptr != nestedControl);
    String name = nestedControl->GetName().c_str();
    name = name.empty() ? "unnamed" : name;
    gridControl->SetName(FastName("Grid control of " + name));
    counterpoiseControl->SetName(FastName("counterpoise of " + name));
    positionHolderControl->SetName(FastName("Position holder of " + name));
    gridControl->AddControl(positionHolderControl.Get());
    positionHolderControl->AddControl(counterpoiseControl.Get());
    counterpoiseControl->AddControl(nestedControl);
}

UIControl* BackgroundController::GetGridControl() const
{
    return gridControl.Get();
}

bool BackgroundController::IsNestedControl(const UIControl* control) const
{
    return control == nestedControl;
}

void BackgroundController::RecalculateBackgroundProperties(ControlNode* node)
{
    if (node->GetControl() == nestedControl)
    {
        UpdateCounterpoise();
    }
    FitGridIfParentIsNested(node);
}

namespace
{
void CalculateTotalRectImpl(UIControl* control, Rect& totalRect, Vector2& rootControlPosition, const UIGeometricData& gd)
{
    if (!control->GetVisibilityFlag())
    {
        return;
    }
    UIGeometricData tempGeometricData = control->GetLocalGeometricData();
    tempGeometricData.AddGeometricData(gd);
    Rect box = tempGeometricData.GetAABBox();

    if (totalRect.x > box.x)
    {
        float32 delta = totalRect.x - box.x;
        rootControlPosition.x += delta;
        totalRect.dx += delta;
        totalRect.x = box.x;
    }
    if (totalRect.y > box.y)
    {
        float32 delta = totalRect.y - box.y;
        rootControlPosition.y += delta;
        totalRect.dy += delta;
        totalRect.y = box.y;
    }
    if (totalRect.x + totalRect.dx < box.x + box.dx)
    {
        float32 nextRight = box.x + box.dx;
        totalRect.dx = nextRight - totalRect.x;
    }
    if (totalRect.y + totalRect.dy < box.y + box.dy)
    {
        float32 nextBottom = box.y + box.dy;
        totalRect.dy = nextBottom - totalRect.y;
    }

    for (const auto& child : control->GetChildren())
    {
        CalculateTotalRectImpl(child, totalRect, rootControlPosition, tempGeometricData);
    }
}
} //unnamed namespace

void BackgroundController::CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition) const
{
    rootControlPosition.SetZero();
    UIGeometricData gd = nestedControl->GetGeometricData();

    gd.position.SetZero();
    UIControl* scalableControl = gridControl->GetParent()->GetParent();
    DVASSERT_MSG(nullptr != scalableControl, "grid update without being attached to screen");
    gd.scale /= scalableControl->GetScale(); //grid->controlCanvas->scalableControl
    if (gd.scale.x != 0.0f || gd.scale.y != 0.0f)
    {
        totalRect = gd.GetAABBox();

        for (const auto& child : nestedControl->GetChildren())
        {
            CalculateTotalRectImpl(child, totalRect, rootControlPosition, gd);
        }
    }
}

void BackgroundController::AdjustToNestedControl()
{
    Rect rect;
    Vector2 pos;
    CalculateTotalRect(rect, pos);
    Vector2 size = rect.GetSize();
    positionHolderControl->SetPosition(pos);
    gridControl->SetSize(size);
    ContentSizeChanged.Emit();
    RootControlPosChanged.Emit(pos);
}

void BackgroundController::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    //check that we adjust removed node
    if (node->GetControl() == nestedControl)
    {
        return;
    }
    FitGridIfParentIsNested(from);
}

void BackgroundController::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int /*index*/)
{
    FitGridIfParentIsNested(destination);
}

void BackgroundController::UpdateCounterpoise()
{
    UIGeometricData gd = nestedControl->GetLocalGeometricData();
    Vector2 invScale;
    invScale.x = gd.scale.x != 0.0f ? 1.0f / gd.scale.x : 0.0f;
    invScale.y = gd.scale.y != 0.0f ? 1.0f / gd.scale.y : 0.0f;
    counterpoiseControl->SetScale(invScale);
    counterpoiseControl->SetSize(gd.size);
    gd.cosA = cosf(gd.angle);
    gd.sinA = sinf(gd.angle);
    counterpoiseControl->SetAngle(-gd.angle);
    Vector2 pos(gd.position * invScale);
    Vector2 angeledPosition(pos.x * gd.cosA + pos.y * gd.sinA,
                            pos.x * -gd.sinA + pos.y * gd.cosA);

    counterpoiseControl->SetPosition(-angeledPosition + gd.pivotPoint);
}

void BackgroundController::FitGridIfParentIsNested(PackageBaseNode* node)
{
    PackageBaseNode* parent = node;
    while (nullptr != parent)
    {
        if (parent->GetControl() == nestedControl) //we change child in the nested control
        {
            AdjustToNestedControl();
            return;
        }
        parent = parent->GetParent();
    }
}

bool BackgroundController::IsPropertyAffectBackground(AbstractProperty* property)
{
    DVASSERT(nullptr != property);
    FastName name(property->GetName());
    static FastName matchedNames[] = { FastName("Angle"), FastName("Size"), FastName("Scale"), FastName("Position"), FastName("Pivot"), FastName("Visible") };
    return std::find(std::begin(matchedNames), std::end(matchedNames), name) != std::end(matchedNames);
}

CanvasSystem::CanvasSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , controlsCanvas(new UIControl())
{
    controlsCanvas->SetName(FastName("controls canvas"));
    systemsManager->GetScalableControl()->AddControl(controlsCanvas.Get());

    systemsManager->EditingRootControlsChanged.Connect(this, &CanvasSystem::OnRootContolsChanged);
    systemsManager->PackageNodeChanged.Connect(this, &CanvasSystem::OnPackageNodeChanged);
    systemsManager->TransformStateChanged.Connect(this, &CanvasSystem::OnTransformStateChanged);
}

CanvasSystem::~CanvasSystem()
{
    systemsManager->GetScalableControl()->RemoveControl(controlsCanvas.Get());
}

void CanvasSystem::OnPackageNodeChanged(PackageNode* package_)
{
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    package = package_;
    if (nullptr != package)
    {
        package->AddListener(this);
    }
}

void CanvasSystem::OnTransformStateChanged(bool inTransformState_)
{
    inTransformState = inTransformState_;
    if (!inTransformState)
    {
        for (auto& node : transformedNodes)
        {
            for (auto& iter : gridControls)
            {
                iter->RecalculateBackgroundProperties(node);
            }
        }
    }
    transformedNodes.clear();
}

void CanvasSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasRemoved(node, from);
    }
}

void CanvasSystem::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasAdded(node, destination, index);
    }
}

void CanvasSystem::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != property);

    if (nullptr == controlsCanvas->GetParent()) //detached canvas
    {
        DVASSERT(false);
        return;
    }

    if (inTransformState)
    {
        transformedNodes.insert(node);
    }
    else
    {
        if (BackgroundController::IsPropertyAffectBackground(property))
        {
            for (auto& iter : gridControls)
            {
                iter->RecalculateBackgroundProperties(node);
            }
        }
    }
}

BackgroundController* CanvasSystem::CreateControlBackground(PackageBaseNode* node)
{
    BackgroundController* backgroundController(new BackgroundController(node->GetControl()));
    backgroundController->ContentSizeChanged.Connect(this, &CanvasSystem::LayoutCanvas);
    backgroundController->RootControlPosChanged.Connect(&systemsManager->RootControlPositionChanged, &Signal<const Vector2&>::Emit);
    gridControls.emplace_back(backgroundController);
    return backgroundController;
}

void CanvasSystem::AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos)
{
    UIControl* grid = backgroundController->GetGridControl();
    if (pos >= controlsCanvas->GetChildren().size())
    {
        controlsCanvas->AddControl(grid);
    }
    else
    {
        auto iterToInsert = controlsCanvas->GetChildren().begin();
        std::advance(iterToInsert, pos);
        controlsCanvas->InsertChildBelow(grid, *iterToInsert);
    }
    backgroundController->UpdateCounterpoise();
    backgroundController->AdjustToNestedControl();
}

uint32 CanvasSystem::GetIndexByPos(const Vector2& pos) const
{
    uint32 index = 0;
    for (auto& iter : gridControls)
    {
        auto grid = iter->GetGridControl();

        if (pos.y < (grid->GetPosition().y + grid->GetSize().y / 2.0f))
        {
            return index;
        }
        index++;
    }
    return index;
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    const List<UIControl*>& children = controlsCanvas->GetChildren();
    int childrenCount = children.size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }
    for (const UIControl* control : children)
    {
        maxWidth = Max(maxWidth, control->GetSize().dx);
        totalHeight += control->GetSize().dy;
    }

    float32 curY = 0.0f;
    for (UIControl* child : children)
    {
        Rect rect = child->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        child->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);
    systemsManager->GetScalableControl()->SetSize(size);
    systemsManager->GetRootControl()->SetSize(size);
    systemsManager->GetInputLayerControl()->SetSize(size);
    systemsManager->CanvasSizeChanged.Emit();
}

void CanvasSystem::OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls_)
{
    Set<PackageBaseNode*> sortedRootControls(rootControls_.begin(), rootControls_.end());
    Set<PackageBaseNode*> newNodes;
    Set<PackageBaseNode*> deletedNodes;
    if (!rootControls.empty())
    {
        std::set_difference(rootControls.begin(), rootControls.end(), sortedRootControls.begin(), sortedRootControls.end(), std::inserter(deletedNodes, deletedNodes.end()));
    }
    if (!sortedRootControls.empty())
    {
        std::set_difference(sortedRootControls.begin(), sortedRootControls.end(), rootControls.begin(), rootControls.end(), std::inserter(newNodes, newNodes.end()));
    }
    rootControls = sortedRootControls;

    for (auto iter = deletedNodes.begin(); iter != deletedNodes.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        UIControl* control = node->GetControl();
        auto findIt = std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
            return gridIter->IsNestedControl(control);
        });
        DVASSERT(findIt != gridControls.end());
        controlsCanvas->RemoveControl(findIt->get()->GetGridControl());
        gridControls.erase(findIt);
    }
    DVASSERT(rootControls_.size() == rootControls.size());
    for (auto iter = rootControls_.begin(); iter != rootControls_.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        if (newNodes.find(node) == newNodes.end())
        {
            continue;
        }
        UIControl* control = node->GetControl();
        DVASSERT(std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
                     return gridIter->IsNestedControl(control);
                 }) == gridControls.end());
        BackgroundController* backgroundController = CreateControlBackground(node);
        AddBackgroundControllerToCanvas(backgroundController, std::distance(rootControls_.begin(), iter));
    }
    LayoutCanvas();
}
