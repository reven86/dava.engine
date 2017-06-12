#include "UI/Preview/Guides/GuidesController.h"
#include "UI/Preview/Guides/GuideLabel.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/PreferencesModule/PreferencesData.h"
#include "QECommands/SetGuidesCommand.h"

#include <TArc/Core/FieldBinder.h>

#include <Reflection/ReflectedTypeDB.h>
#include <Logger/Logger.h>
#include <Preferences/PreferencesStorage.h>
#include <Preferences/PreferencesRegistrator.h>

GuidesController::GuidesController(DAVA::TArc::ContextAccessor* accessor_, QWidget* container_)
    : accessor(accessor_)
    , fieldBinder(new DAVA::TArc::FieldBinder(accessor))
    , container(container_)
{
    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    preferencesDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreferencesData>());

    BindFields();

    preferences.guidesColorChanged.Connect(this, &GuidesController::OnGuidesColorChanged);
    preferences.previewGuideColorChanged.Connect(this, &GuidesController::OnPreviewGuideColorChanged);
}

void GuidesController::CreatePreviewGuide()
{
    DVASSERT(previewGuide.line == nullptr && previewGuide.text == nullptr);
    previewGuide = CreateGuide(preferences.GetPreviewGuideColor());
    previewGuide.Hide();
}

void GuidesController::OnContainerGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight, DAVA::float32 rulerRelativePos_)
{
    ProcessGeometryChanged(bottomLeft, topRight);
    rulerRelativePos = rulerRelativePos_;
    SyncGuidesWithValues();
}

void GuidesController::OnCanvasParametersChanged(DAVA::float32 scaledMinValue_, DAVA::float32 min_, DAVA::float32 max_, DAVA::float32 scale_)
{
    scaledMinValue = scaledMinValue_;
    minValue = min_;
    maxValue = max_;
    scale = scale_;

    SyncGuidesWithValues();
}

void GuidesController::OnMousePress(DAVA::float32 position)
{
    if (IsEnabled() == false || displayState == NO_DISPLAY)
    {
        return;
    }

    //if guides was disabled - enable guides and create new one
    if (IsGuidesEnabled() == false)
    {
        SetGuidesEnabled(true);
        SetDisplayState(DISPLAY_PREVIEW);
    }

    if (displayState == DISPLAY_PREVIEW)
    {
        PackageNode::AxisGuides values = GetValues();

        values.push_back(PositionToValue(position));

        SetValues(values);

        SyncGuidesWithValues();

        previewGuide.Raise();

        SetDisplayState(DISPLAY_DRAG);
    }

    EnableDrag(position);
}

void GuidesController::OnMouseMove(DAVA::float32 position)
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }

    if (dragState == DRAG)
    {
        DragGuide(position);
    }
    else
    {
        PackageNode::AxisGuides::iterator valuePtr = GetNearestValuePtr(position);
        if (valuePtr != cachedValues.end())
        {
            SetDisplayState(DISPLAY_DRAG);
        }
        else
        {
            SetDisplayState(DISPLAY_PREVIEW);
            SetupPreviewGuide(position);
        }
    }
}

void GuidesController::OnMouseRelease(DAVA::float32 position)
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }

    //copy  pointer to dragged item to remove it if we drag outside of screen
    DisableDrag();

    if (displayState == DISPLAY_REMOVE)
    {
        RemoveLastGuide();
        SetDisplayState(NO_DISPLAY);
    }
}

void GuidesController::OnMouseLeave()
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return;
    }
    SetDisplayState(NO_DISPLAY);
}

QList<QAction*> GuidesController::GetActions(DAVA::float32 position, QObject* parent)
{
    //we can call context menu when we dragging guide
    DisableDrag();

    QList<QAction*> actions;

    QAction* guidesEnabledAction = new QAction("Show Alignment Guides", parent);
    guidesEnabledAction->setCheckable(true);
    guidesEnabledAction->setChecked(IsGuidesEnabled());
    connect(guidesEnabledAction, &QAction::toggled, this, &GuidesController::SetGuidesEnabled);

    actions << guidesEnabledAction;

    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        return actions;
    }

    PackageNode::AxisGuides::iterator valuePtr = GetNearestValuePtr(position);
    if (valuePtr != cachedValues.end())
    {
        QAction* removeGuideAction = new QAction("Remove Guide", parent);
        connect(removeGuideAction, &QAction::triggered, std::bind(&GuidesController::RemoveGuide, this, *valuePtr));
        actions << removeGuideAction;
    }

    QAction* removeAllGuidesAction = new QAction(QString("Remove All %1 Guides").arg(GetOrientation() == DAVA::Vector2::AXIS_X ? "Vertical" : "Horizontal"), parent);
    connect(removeAllGuidesAction, &QAction::triggered, this, &GuidesController::RemoveAllGuides);
    if (GetValues().empty())
    {
        removeAllGuidesAction->setEnabled(false);
    }
    actions << removeAllGuidesAction;

    return actions;
}

void GuidesController::SetDisplayState(eDisplayState state)
{
    if (displayState == state)
    {
        return;
    }

    //leave current state
    switch (displayState)
    {
    case DISPLAY_PREVIEW:
        previewGuide.Hide();
        break;
    case DISPLAY_DRAG:
    case DISPLAY_REMOVE:
        container->unsetCursor();
        break;
    default:
        break;
    }

    displayState = state;

    switch (displayState)
    {
    case DISPLAY_PREVIEW:
        previewGuide.Show();
        previewGuide.Raise();
        break;
    case DISPLAY_DRAG:
        container->setCursor(GetOrientation() == DAVA::Vector2::AXIS_X ? Qt::SplitHCursor : Qt::SplitVCursor);
        break;
    case DISPLAY_REMOVE:
        container->setCursor(QCursor(QPixmap(":/Cursors/trashCursor.png")));
    default:
        break;
    }
}

void GuidesController::EnableDrag(DAVA::float32 position)
{
    DVASSERT(IsEnabled());
    if (dragState == DRAG)
    {
        return;
    }

    valuePtr = GetNearestValuePtr(position);

    DVASSERT(valuePtr != cachedValues.end());
    if (valuePtr != cachedValues.end())
    {
        dragState = DRAG;

        DAVA::TArc::DataContext* active = accessor->GetActiveContext();
        DocumentData* data = active->GetData<DocumentData>();
        data->BeginBatch("Dragging guide");

        //move dragged guide to the end of list to display it under all other guides
        cachedValues.splice(cachedValues.end(), cachedValues, valuePtr);
    }
}

void GuidesController::DisableDrag()
{
    if (dragState != DRAG)
    {
        return;
    }

    DAVA::TArc::DataContext* active = accessor->GetActiveContext();
    DocumentData* data = active->GetData<DocumentData>();
    data->EndBatch();

    dragState = NO_DRAG;
}

void GuidesController::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = DocumentData::guidesPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnValuesChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = DocumentData::displayedRootControlsPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnRootControlsChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<PreferencesData>();
        fieldDescr.fieldName = PreferencesData::guidesEnabledPropertyName;
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &GuidesController::OnGuidesEnabledChanged));
    }
}

void GuidesController::OnValuesChanged(const DAVA::Any&)
{
    SyncGuidesWithValues();
}

void GuidesController::OnRootControlsChanged(const DAVA::Any& rootControls)
{
    //this is not good situation, but we can reload or close document by shortcut while we dragging guide
    SetDisplayState(NO_DISPLAY);
    DisableDrag();
    SyncGuidesWithValues();
}

void GuidesController::OnGuidesEnabledChanged(const DAVA::Any&)
{
    SyncGuidesWithValues();
}

void GuidesController::SyncGuidesWithValues()
{
    if (IsEnabled() == false || IsGuidesEnabled() == false)
    {
        while (guides.empty() == false)
        {
            RemoveLastGuide();
        }
        return;
    }

    PackageNode::AxisGuides values = GetValues();
    PackageNode::AxisGuides visibleValues;
    for (DAVA::float32 value : values)
    {
        if (value > minValue && value < maxValue)
        {
            visibleValues.push_back(value);
        }
    }

    std::size_t size = visibleValues.size();

    while (guides.size() > size)
    {
        RemoveLastGuide();
    }
    while (guides.size() < size)
    {
        Guide guide = CreateGuide(preferences.GetGuidesColor());
        guides.append(guide);
    }

    DVASSERT(size == guides.size());
    int index = 0;
    for (DAVA::float32 value : visibleValues)
    {
        Guide& guide = guides[index++];
        ResizeGuide(guide);
        guide.text->SetValue(value);
        MoveGuide(value, guide);

        guide.Show();
        guide.Raise();
    }
}

PackageNode::AxisGuides::iterator GuidesController::GetNearestValuePtr(DAVA::float32 position)
{
    DAVA::float32 range = 1;
    if (scale < 1.0f)
    {
        range = 3 / scale;
    }

    cachedValues = GetValues();

    if (cachedValues.empty())
    {
        return cachedValues.end();
    }

    DAVA::float32 value = PositionToValue(position);

    PackageNode::AxisGuides::iterator iter = std::min_element(cachedValues.begin(), cachedValues.end(), [value](DAVA::float32 left, DAVA::float32 right)
                                                              {
                                                                  return std::abs(left - value) < std::abs(right - value);
                                                              });
    if (std::fabs(*iter - value) > range)
    {
        return cachedValues.end();
    }
    return iter;
}

bool GuidesController::IsEnabled() const
{
    if (documentDataWrapper.HasData())
    {
        SortedControlNodeSet displayedRootControls = documentDataWrapper.GetFieldValue(DocumentData::displayedRootControlsPropertyName).Cast<SortedControlNodeSet>(SortedControlNodeSet());
        if (displayedRootControls.size() == 1)
        {
            return true;
        }
    }
    return false;
}

PackageNode::AxisGuides GuidesController::GetValues() const
{
    PackageNode::Guides guides = documentDataWrapper.GetFieldValue(DocumentData::guidesPropertyName).Cast<PackageNode::Guides>(PackageNode::Guides());
    return guides[GetOrientation()];
}

void GuidesController::SetValues(const PackageNode::AxisGuides& values)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* data = activeContext->GetData<DocumentData>();
    const SortedControlNodeSet& rootControls = data->GetDisplayedRootControls();
    Q_ASSERT(rootControls.size() == 1);
    DAVA::String name = (*rootControls.begin())->GetName();

    data->ExecCommand<SetGuidesCommand>(name, GetOrientation(), values);
}

void GuidesController::SetupPreviewGuide(DAVA::float32 position)
{
    DAVA::float32 value = PositionToValue(position);
    ResizeGuide(previewGuide);
    previewGuide.text->SetValue(value);
    MoveGuide(value, previewGuide);
}

Guide GuidesController::CreateGuide(const DAVA::Color& color) const
{
    Guide guide;
    guide.line = new QWidget(container);
    guide.text = new GuideLabel(GetOrientation(), container);

    guide.line->setAttribute(Qt::WA_TransparentForMouseEvents);
    guide.text->setAttribute(Qt::WA_TransparentForMouseEvents);
    SetGuideColor(guide.line, color);
    return guide;
}

void GuidesController::DragGuide(DAVA::float32 position)
{
    DVASSERT(IsEnabled());
    DVASSERT(dragState == DRAG);

    DAVA::float32 value = PositionToValue(position);
    SetDisplayState((value < minValue || value > maxValue) ? DISPLAY_REMOVE : DISPLAY_DRAG);

    *valuePtr = PositionToValue(position);
    SetValues(cachedValues);

    SyncGuidesWithValues();
}

void GuidesController::RemoveGuide(DAVA::float32 value)
{
    cachedValues = GetValues();
    cachedValues.remove(value);
    SetValues(cachedValues);
}

void GuidesController::RemoveAllGuides()
{
    SetValues(PackageNode::AxisGuides());
}

bool GuidesController::IsGuidesEnabled() const
{
    return preferencesDataWrapper.GetFieldValue(PreferencesData::guidesEnabledPropertyName).Cast<bool>(true);
}

void GuidesController::SetGuidesEnabled(bool enabled)
{
    preferencesDataWrapper.SetFieldValue(PreferencesData::guidesEnabledPropertyName, enabled);
    if (enabled == false)
    {
        DisableDrag();
        SetDisplayState(NO_DISPLAY);
    }
}

void GuidesController::OnGuidesColorChanged(const DAVA::Color& color)
{
    for (Guide& guide : guides)
    {
        SetGuideColor(guide.line, color);
    }
}

void GuidesController::OnPreviewGuideColorChanged(const DAVA::Color& color)
{
    SetGuideColor(previewGuide.line, color);
}

void GuidesController::SetGuideColor(QWidget* guide, const DAVA::Color& color) const
{
    QString colorString = QString("rgba(%1, %2, %3, %4)")
                          .arg(color.r * 255.0f)
                          .arg(color.g * 255.0f)
                          .arg(color.b * 255.0f)
                          .arg(color.a * 255.0f);

    guide->setStyleSheet(QString("QWidget { background-color: %1; }").arg(colorString));
}

void GuidesController::RemoveLastGuide()
{
    Guide& guide = guides.last();
    delete guide.line;
    delete guide.text;
    guides.removeLast();
}

DAVA::float32 GuidesController::PositionToValue(DAVA::float32 position) const
{
    return std::round((scaledMinValue + position) / scale);
}

DAVA::float32 GuidesController::ValueToPosition(DAVA::float32 value) const
{
    return rulerRelativePos + (value)*scale - scaledMinValue;
}

HGuidesController::HGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container)
    : GuidesController(accessor, container)
{
}

void HGuidesController::ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight)
{
    int top = topRight.y();
    int bottom = bottomLeft.y();

    guideStartPosition = top;
    size = bottom - top;
}

void HGuidesController::ResizeGuide(Guide& guide) const
{
    guide.line->resize(1, size);
    guide.text->resize(30, 15);
}

void HGuidesController::MoveGuide(DAVA::float32 value, Guide& guide) const
{
    DAVA::float32 xPosition = ValueToPosition(value);
    guide.line->move(xPosition, guideStartPosition);
    guide.text->move(xPosition + 5, guideStartPosition);
}

DAVA::Vector2::eAxis HGuidesController::GetOrientation() const
{
    return DAVA::Vector2::AXIS_X;
}

VGuidesController::VGuidesController(DAVA::TArc::ContextAccessor* accessor, QWidget* container)
    : GuidesController(accessor, container)
{
}

void VGuidesController::ProcessGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight)
{
    int left = bottomLeft.x();
    int right = topRight.x();

    guideStartPosition = left;
    size = right - left;
}

void VGuidesController::ResizeGuide(Guide& guide) const
{
    guide.line->resize(size, 1);
    guide.text->resize(15, 30);
}

void VGuidesController::MoveGuide(DAVA::float32 value, Guide& guide) const
{
    DAVA::float32 yPosition = ValueToPosition(value);
    guide.line->move(guideStartPosition, yPosition);
    guide.text->move(guideStartPosition, yPosition - guide.text->height() - 5);
}

DAVA::Vector2::eAxis VGuidesController::GetOrientation() const
{
    return DAVA::Vector2::AXIS_Y;
}

GuidesControllerPreferences::GuidesControllerPreferences()
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

GuidesControllerPreferences::~GuidesControllerPreferences()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

const DAVA::Color& GuidesControllerPreferences::GetGuidesColor() const
{
    return guidesColor;
}

void GuidesControllerPreferences::SetGuidesColor(const DAVA::Color& color)
{
    guidesColor = color;
    guidesColorChanged.Emit(color);
}

const DAVA::Color& GuidesControllerPreferences::GetPreviewGuideColor() const
{
    return previewGuideColor;
}

void GuidesControllerPreferences::SetPreviewGuideColor(const DAVA::Color& color)
{
    previewGuideColor = color;
    previewGuideColorChanged.Emit(color);
}

REGISTER_PREFERENCES_ON_START(GuidesControllerPreferences,
                              PREF_ARG("detectGuideDistance", DAVA::float32(3.0f)),
                              PREF_ARG("guideColor", DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f)),
                              PREF_ARG("previewGuideColor", DAVA::Color(1.0f, 0.0f, 0.0f, 0.5f))
                              )
