#include "UI/Preview/Guides/GuidesController.h"

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
}

void GuidesController::CreatePreviewGuide()
{
    DVASSERT(previewGuide == nullptr);
    previewGuide = new QWidget(container);
    previewGuide->setAttribute(Qt::WA_TransparentForMouseEvents);

    previewGuide->setStyleSheet("QWidget { background-color: rgba(255, 0, 0, 50%); }");
    previewGuide->hide();
}

void GuidesController::OnContainerGeometryChanged(const QPoint& bottomLeft, const QPoint& topRight, DAVA::float32 rulerRelativePos_)
{
    ProcessGeometryChanged(bottomLeft, topRight);
    rulerRelativePos = rulerRelativePos_;

    SyncGuidesWithValues();
}

void GuidesController::OnCanvasParametersChanged(DAVA::float32 min_, DAVA::float32 max_, DAVA::float32 scale_)
{
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
        CreateGuide(position);
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
        DVASSERT(valuePtr != nullptr);
        DragGuide(position);
    }
    else
    {
        if (GetNearestValuePtr(position) != nullptr)
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

    DisableDrag();
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

    DAVA::float32* valuePtr = GetNearestValuePtr(position);
    if (valuePtr != nullptr)
    {
        QAction* removeGuideAction = new QAction("Remove Guide", parent);
        connect(removeGuideAction, &QAction::triggered, std::bind(&GuidesController::RemoveGuide, this, *valuePtr));
        actions << removeGuideAction;
    }

    QAction* removeAllGuidesAction = new QAction(QString("Remove All %1 Guides").arg(GetOrientation() == DAVA::Vector2::AXIS_X ? "Horizontal" : "Vertical"), parent);
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
        previewGuide->hide();
        break;
    case DISPLAY_DRAG:
        container->unsetCursor();
        break;
    default:
        break;
    }

    displayState = state;

    switch (displayState)
    {
    case DISPLAY_PREVIEW:
        previewGuide->show();
        break;
    case DISPLAY_DRAG:
        container->setCursor(GetOrientation() == DAVA::Vector2::AXIS_X ? Qt::SplitHCursor : Qt::SplitVCursor);
    default:
        break;
    }
}

void GuidesController::EnableDrag(DAVA::float32 position)
{
    DVASSERT(IsEnabled());
    DVASSERT(dragState == NO_DRAG);

    DVASSERT(valuePtr == nullptr);
    valuePtr = GetNearestValuePtr(position);

    DVASSERT(valuePtr != nullptr);
    if (valuePtr != nullptr)
    {
        dragState = DRAG;

        DAVA::TArc::DataContext* active = accessor->GetActiveContext();
        DocumentData* data = active->GetData<DocumentData>();
        data->BeginBatch("Dragging guide");
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

    DVASSERT(valuePtr != nullptr);
    valuePtr = nullptr;
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
            guides.front()->deleteLater();
            guides.pop_front();
        }
        return;
    }

    PackageNode::AxisGuides values = GetValues();
    auto iter = std::lower_bound(values.begin(), values.end(), minValue);
    auto endIter = std::upper_bound(values.begin(), values.end(), maxValue);

    int size = std::distance(iter, endIter);
    while (guides.size() > size)
    {
        delete guides.takeLast();
    }
    while (guides.size() < size)
    {
        QWidget* guide = new QWidget(container);
        guide->setAttribute(Qt::WA_TransparentForMouseEvents);
        guide->setStyleSheet("QWidget { background-color: rgba(255, 0, 0, 100%); }");

        guide->show();

        guides.append(guide);
    }

    int index = 0;
    for (; iter != endIter; ++iter, ++index)
    {
        QWidget* guide = guides.at(index);
        ResizeGuide(guide);
        MoveGuide(*iter, guide);
    }
}

DAVA::float32* GuidesController::GetNearestValuePtr(DAVA::float32 position)
{
    const DAVA::float32 range = preferences.detectGuideDistance / scale;

    cachedValues = GetValues();

    if (cachedValues.empty())
    {
        return nullptr;
    }

    DAVA::float32 value = PositionToValue(position);

    PackageNode::AxisGuides::iterator iter = std::min_element(cachedValues.begin(), cachedValues.end(), [value](DAVA::float32 left, DAVA::float32 right)
                                                              {
                                                                  return std::abs(left - value) < std::abs(right - value);
                                                              });
    if (std::fabs(*iter - value) > range)
    {
        return nullptr;
    }
    return &(*iter);
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
    MoveGuide(value, previewGuide);
}

void GuidesController::CreateGuide(DAVA::float32 position)
{
    PackageNode::AxisGuides values = GetValues();

    values.push_back(PositionToValue(position));
    std::sort(values.begin(), values.end());

    SetValues(values);

    SyncGuidesWithValues();
}

void GuidesController::DragGuide(DAVA::float32 position)
{
    DVASSERT(IsEnabled());
    DVASSERT(valuePtr != nullptr);

    *valuePtr = PositionToValue(position);
    std::sort(cachedValues.begin(), cachedValues.end());
    SetValues(cachedValues);

    SyncGuidesWithValues();
}

void GuidesController::RemoveGuide(DAVA::float32 value)
{
    cachedValues = GetValues();
    cachedValues.erase(std::remove(cachedValues.begin(), cachedValues.end(), value));
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

DAVA::float32 GuidesController::PositionToValue(DAVA::float32 position) const
{
    return std::floor(minValue + position / scale);
}

DAVA::float32 GuidesController::ValueToPosition(DAVA::float32 value) const
{
    return rulerRelativePos + (value - minValue) * scale;
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

void HGuidesController::ResizeGuide(QWidget* guide) const
{
    guide->resize(1, size);
}

void HGuidesController::MoveGuide(DAVA::float32 value, QWidget* guide) const
{
    guide->move(ValueToPosition(value), guideStartPosition);
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

void VGuidesController::ResizeGuide(QWidget* guide) const
{
    guide->resize(size, 1);
}

void VGuidesController::MoveGuide(DAVA::float32 value, QWidget* guide) const
{
    guide->move(guideStartPosition, ValueToPosition(value));
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

REGISTER_PREFERENCES_ON_START(GuidesControllerPreferences,
                              PREF_ARG("detectGuideDistance", DAVA::float32(3.0f))
                              )
