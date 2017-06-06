#pragma once

#include "UI/Preview/Guides/IRulerListener.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/Any.h>
#include <Base/Introspection.h>
#include <Math/Vector.h>

#include <QWidget>
#include <QMap>
#include <QObject>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

class QWidget;
class QPropertyAnimation;

class GuidesControllerPreferences : public DAVA::InspBase
{
public:
    GuidesControllerPreferences();
    ~GuidesControllerPreferences();

    //preferences
    DAVA::float32 detectGuideDistance;

    INTROSPECTION(GuidesControllerPreferences,
                  MEMBER(detectGuideDistance, "Rulers/distance from guide to drag", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};

//this class realize Behavior pattern to have different behaviors for vertical and for horizontal guides
class GuidesController : public QObject, public IRulerListener
{
    Q_OBJECT

public:
    GuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container);

    void CreatePreviewGuide();
    void OnContainerGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight, DAVA::float32 rulerRelativePos);
    void OnCanvasParametersChanged(DAVA::float32 min, DAVA::float32 max, DAVA::float32 scale);
    void BindFields();

private:
    //IRulerListener
    void OnMousePress(DAVA::float32 position) override;
    void OnMouseMove(DAVA::float32 position) override;
    void OnMouseRelease(DAVA::float32 position) override;
    void OnMouseLeave() override;
    QList<QAction*> GetActions(DAVA::float32 position, QObject* parent) override;

    //cursor and preview guide states
    //NO_DISPLAY: normal cursor and no preview guide
    //DISPLAY_PREVIEW: normal cursor and preview guide are shown
    //DISPLAY_DRAG: drag cursor and no preview guide
    enum eDisplayState
    {
        NO_DISPLAY,
        DISPLAY_PREVIEW,
        DISPLAY_DRAG
    };
    eDisplayState displayState = NO_DISPLAY;
    void SetDisplayState(eDisplayState state);

    //controller state. Used only to wrap valuePtr pointer
    //NO_DRAG: do nothing
    //DRAG: store copy of current guides and pointer to a modified guide
    enum eDragState
    {
        NO_DRAG,
        DRAG
    };
    eDragState dragState = NO_DRAG;
    void EnableDrag(DAVA::float32 position);
    void DisableDrag();

    void OnValuesChanged(const DAVA::Any& values);
    void OnRootControlsChanged(const DAVA::Any& rootControls);
    void OnGuidesEnabledChanged(const DAVA::Any& guidesEnabled);

    void SyncGuidesWithValues();

    //descr: returns closest value to given position
    //returns nullptr if closest value is too far or if no values available
    DAVA::float32* GetNearestValuePtr(DAVA::float32 position);

    bool IsEnabled() const;
    PackageNode::AxisGuides GetValues() const;
    void SetValues(const PackageNode::AxisGuides& values);

    void SetupPreviewGuide(DAVA::float32 position);
    void CreateGuide(DAVA::float32 position);
    void DragGuide(DAVA::float32 position);

    void RemoveGuide(DAVA::float32 value);
    void RemoveAllGuides();

    bool IsGuidesEnabled() const;
    void SetGuidesEnabled(bool enabled);

    //behavior
    virtual void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) = 0;
    virtual void ResizeGuide(QWidget* guide) const = 0;
    virtual void MoveGuide(DAVA::float32 position, QWidget* guide) const = 0;
    virtual DAVA::Vector2::eAxis GetOrientation() const = 0;

protected:
    DAVA::float32 PositionToValue(DAVA::float32 position) const;
    DAVA::float32 ValueToPosition(DAVA::float32 value) const;

    //guide position in container coordinates
    DAVA::float32 guideStartPosition = 0.0f;
    DAVA::float32 size = 0.0f;

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::DataWrapper documentDataWrapper;
    DAVA::TArc::DataWrapper preferencesDataWrapper;

    QWidgetList guides;

    //use it only for drag-n-drop
    PackageNode::AxisGuides cachedValues;

    //guide value on ruler
    DAVA::float32 minValue = 0.0f;
    DAVA::float32 maxValue = 0.0f;

    //ruler widget position to show guide in correct point
    DAVA::float32 rulerRelativePos = 0.0f;

    DAVA::float32 scale = 0.0f;

    //parent widget of guides
    QWidget* container = nullptr;

    //pointer to currentGuide to modify it value on drag
    DAVA::float32* valuePtr = nullptr;

    //semi-transparent preview guide
    QWidget* previewGuide = nullptr;

    GuidesControllerPreferences preferences;
};

class HGuidesController : public GuidesController
{
public:
    HGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container);

private:
    void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) override;

    void ResizeGuide(QWidget* guide) const override;
    void MoveGuide(DAVA::float32 position, QWidget* guide) const override;

    DAVA::Vector2::eAxis GetOrientation() const override;
};

class VGuidesController : public GuidesController
{
public:
    VGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container);

private:
    void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) override;

    void ResizeGuide(QWidget* guide) const override;
    void MoveGuide(DAVA::float32 position, QWidget* guide) const override;

    DAVA::Vector2::eAxis GetOrientation() const override;
};
