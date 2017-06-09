#pragma once

#include "UI/Preview/Guides/IRulerListener.h"
#include "UI/Preview/Guides/Guide.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/Any.h>
#include <Base/Introspection.h>
#include <Math/Vector.h>
#include <Math/Color.h>

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

    const DAVA::Color& GetGuidesColor() const;
    void SetGuidesColor(const DAVA::Color& color);

    const DAVA::Color& GetPreviewGuideColor() const;
    void SetPreviewGuideColor(const DAVA::Color& color);

    DAVA::Signal<const DAVA::Color&> previewGuideColorChanged;
    DAVA::Signal<const DAVA::Color&> guidesColorChanged;

private:
    DAVA::Color guidesColor;
    DAVA::Color previewGuideColor;

public:
    INTROSPECTION(GuidesControllerPreferences,
                  PROPERTY("guideColor", "Rulers/guide color", GetGuidesColor, SetGuidesColor, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("previewGuideColor", "Rulers/preview guide color", GetPreviewGuideColor, SetPreviewGuideColor, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
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
    void OnCanvasParametersChanged(DAVA::float32 scaledMinValue, DAVA::float32 min, DAVA::float32 max, DAVA::float32 scale);
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
    //DISPLAY_PREVIEW: normal cursor and preview guide is visible
    //DISPLAY_DRAG: drag cursor and no preview guide
    //DISPLAY_REMOVE: cursor changed to RemoveCursor and no preview guide
    enum eDisplayState
    {
        NO_DISPLAY,
        DISPLAY_PREVIEW,
        DISPLAY_DRAG,
        DISPLAY_REMOVE
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
    void DragGuide(DAVA::float32 position);

    void RemoveGuide(DAVA::float32 value);
    void RemoveAllGuides();

    bool IsGuidesEnabled() const;
    void SetGuidesEnabled(bool enabled);

    void OnGuidesColorChanged(const DAVA::Color& color);
    void OnPreviewGuideColorChanged(const DAVA::Color& color);

    Guide CreateGuide(const DAVA::Color& color) const;
    void SetGuideColor(QWidget* guide, const DAVA::Color& color) const;
    void RemoveLastGuide();

    //behavior
    virtual void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) = 0;
    virtual void ResizeGuide(Guide& guide) const = 0;
    virtual void MoveGuide(DAVA::float32 position, Guide& guide) const = 0;
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

    //use it only for drag-n-drop
    PackageNode::AxisGuides cachedValues;

    //this variable used to convert between position in value
    //in big scale values is very differ from minValue, because minValue can be 100 but scaledMinValue might be 108
    DAVA::float32 scaledMinValue = 0;

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
    Guide previewGuide;
    QList<Guide> guides;

    GuidesControllerPreferences preferences;
};

class HGuidesController : public GuidesController
{
public:
    HGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container);

private:
    void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) override;

    void ResizeGuide(Guide& guide) const override;
    void MoveGuide(DAVA::float32 position, Guide& guide) const override;

    DAVA::Vector2::eAxis GetOrientation() const override;
};

class VGuidesController : public GuidesController
{
public:
    VGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container);

private:
    void ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight) override;

    void ResizeGuide(Guide& guide) const override;
    void MoveGuide(DAVA::float32 position, Guide& guide) const override;

    DAVA::Vector2::eAxis GetOrientation() const override;
};
