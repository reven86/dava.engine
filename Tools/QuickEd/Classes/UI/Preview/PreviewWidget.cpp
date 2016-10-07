#include "PreviewWidget.h"

#include "ScrollAreaController.h"

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortCut>
#include <QFileInfo>
#include <QInputDialog>
#include <QTimer>

#include "UI/UIControl.h"
#include "UI/UIScreenManager.h"
#include "UI/QtModelPackageCommandExecutor.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/Updaters/ContinuousUpdater.h"
#include "QtTools/InputDialogs/MultilineTextInputDialog.h"

#include "Document/Document.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "EditorSystems/CanvasSystem.h"
#include "EditorSystems/HUDSystem.h"
#include "Ruler/RulerWidget.h"
#include "Ruler/RulerController.h"
#include "UI/Package/PackageMimeData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

using namespace DAVA;

namespace
{
QString ScaleStringFromReal(float scale)
{
    return QString("%1 %").arg(static_cast<int>(scale * 100.0f + 0.5f));
}

struct PreviewContext : WidgetContext
{
    QPoint canvasPosition;
};

struct SystemsContext : WidgetContext
{
    SelectedNodes selection;
};
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , davaGLWidget(new DavaGLWidget(this))
    , scrollAreaController(new ScrollAreaController(this))
    , rulerController(new RulerController(this))
    , continuousUpdater(new ContinuousUpdater(MakeFunction(this, &PreviewWidget::NotifySelectionChanged), this, 300))
{
    qRegisterMetaType<SelectedNodes>("SelectedNodes");
    percentages << 0.25f << 0.33f << 0.50f << 0.67f << 0.75f << 0.90f
                << 1.00f << 1.10f << 1.25f << 1.50f << 1.75f << 2.00f
                << 2.50f << 3.00f << 4.00f << 5.00f << 6.00f << 7.00f << 8.00f;
    setupUi(this);

    connect(rulerController, &RulerController::HorisontalRulerSettingsChanged, horizontalRuler, &RulerWidget::OnRulerSettingsChanged);
    connect(rulerController, &RulerController::VerticalRulerSettingsChanged, verticalRuler, &RulerWidget::OnRulerSettingsChanged);

    connect(rulerController, &RulerController::HorisontalRulerMarkPositionChanged, horizontalRuler, &RulerWidget::OnMarkerPositionChanged);
    connect(rulerController, &RulerController::VerticalRulerMarkPositionChanged, verticalRuler, &RulerWidget::OnMarkerPositionChanged);

    connect(scrollAreaController, &ScrollAreaController::NestedControlPositionChanged, this, &PreviewWidget::OnNestedControlPositionChanged);

    verticalRuler->SetRulerOrientation(Qt::Vertical);
    frame->layout()->addWidget(davaGLWidget);
    davaGLWidget->GetGLView()->installEventFilter(this);

    davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    connect(davaGLWidget, &DavaGLWidget::Initialized, this, &PreviewWidget::OnGLInitialized);
    connect(davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized);
    // Setup the Scale Combo.
    for (auto percentage : percentages)
    {
        scaleCombo->addItem(ScaleStringFromReal(percentage));
    }
    connect(scrollAreaController, &ScrollAreaController::ViewSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::CanvasSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::PositionChanged, this, &PreviewWidget::OnPositionChanged);
    connect(scrollAreaController, &ScrollAreaController::ScaleChanged, this, &PreviewWidget::OnScaleChanged);

    connect(scaleCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    SetActualScale();
    QRegExp regEx("[0-8]?([0-9]|[0-9]){0,2}\\s?\\%?");
    scaleCombo->setValidator(new QRegExpValidator(regEx));
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    UpdateScrollArea();
    CreateActions();
}

PreviewWidget::~PreviewWidget()
{
    continuousUpdater->Stop();
}

ScrollAreaController* PreviewWidget::GetScrollAreaController()
{
    return scrollAreaController;
}

RulerController* PreviewWidget::GetRulerController()
{
    return rulerController;
}

float PreviewWidget::GetScaleFromComboboxText() const
{
    // Firstly verify whether the value is already set.
    QString curTextValue = scaleCombo->currentText();
    curTextValue.remove('%');
    curTextValue.remove(' ');
    bool ok;
    float scaleValue = curTextValue.toFloat(&ok);
    DVASSERT_MSG(ok, "can not parse text to float");
    return scaleValue / 100.0f;
}

ControlNode* PreviewWidget::HighlightNodeUnderPoint(const QPoint& localPos)
{
    //GL is not initialized yet
    if (!systemsManager)
    {
        return nullptr;
    }

    Vector2 davaPos(localPos.x(), localPos.y());
    return systemsManager->HighlightNodeUnderPoint(davaPos);
}

void PreviewWidget::ClearHightlight()
{
    //GL is not initialized yet
    if (!systemsManager)
    {
        return;
    }
    systemsManager->NodesHovered.Emit({ nullptr });
}

void PreviewWidget::CreateActions()
{
    QAction* importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WindowShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PreviewWidget::ImportRequested);
    davaGLWidget->addAction(importPackageAction);

    QAction* cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WindowShortcut);
    connect(cutAction, &QAction::triggered, this, &PreviewWidget::CutRequested);
    davaGLWidget->addAction(cutAction);

    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WindowShortcut);
    connect(copyAction, &QAction::triggered, this, &PreviewWidget::CopyRequested);
    davaGLWidget->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WindowShortcut);
    connect(pasteAction, &QAction::triggered, this, &PreviewWidget::PasteRequested);
    davaGLWidget->addAction(pasteAction);

    QAction* deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform

    deleteAction->setShortcutContext(Qt::WindowShortcut); //widget shortcut is not working for davaGLWidget
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    davaGLWidget->addAction(deleteAction);

    selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(selectAllAction);

    focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(focusNextChildAction);

    focusPreviousChildAction = new QAction(tr("Focus frevious child"), this);
    focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
    focusPreviousChildAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(focusPreviousChildAction);
}

void PreviewWidget::OnDocumentChanged(Document* arg)
{
    DVASSERT(nullptr != systemsManager);
    continuousUpdater->Stop();
    SaveContext();
    document = arg;
    systemsManager->MagnetLinesChanged.Emit({});
    systemsManager->NodesHovered.Emit({});
    if (document.isNull())
    {
        systemsManager->PackageNodeChanged.Emit(nullptr);
    }
    else
    {
        systemsManager->PackageNodeChanged.Emit(document->GetPackage());
        LoadContext();
    }
}

void PreviewWidget::SaveSystemsContextAndClear()
{
    if (!document.isNull())
    {
        SystemsContext* systemsContext = DynamicTypeCheck<SystemsContext*>(document->GetContext(systemsManager.get()));
        systemsContext->selection = selectionContainer.selectedNodes;
    }
    if (!selectionContainer.selectedNodes.empty())
    {
        DVASSERT(!document.isNull());
        systemsManager->ClearSelection();
        continuousUpdater->Stop();
        DVASSERT(selectionContainer.selectedNodes.empty());
    }
}

void PreviewWidget::LoadSystemsContext(Document* arg)
{
    DVASSERT(arg == document.data());
    if (document.isNull())
    {
        return;
    }
    SystemsContext* context = DynamicTypeCheck<SystemsContext*>(document->GetContext(systemsManager.get()));
    if (nullptr == context)
    {
        document->SetContext(systemsManager.get(), new SystemsContext());
    }
    else
    {
        selectionContainer.selectedNodes = context->selection;
        if (!selectionContainer.selectedNodes.empty())
        {
            systemsManager->SelectionChanged.Emit(selectionContainer.selectedNodes, SelectedNodes());
        }
    }
}

void PreviewWidget::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    systemsManager->SelectionChanged.Emit(selected, deselected);
}

void PreviewWidget::OnRootControlPositionChanged(const Vector2& pos)
{
    rootControlPos = QPoint(static_cast<int>(pos.x), static_cast<int>(pos.y));
    ApplyPosChanges();
}

void PreviewWidget::OnNestedControlPositionChanged(const QPoint& pos)
{
    canvasPos = pos;
    ApplyPosChanges();
}

void PreviewWidget::OnEmulationModeChanged(bool emulationMode)
{
    systemsManager->SetEmulationMode(emulationMode);

    if (emulationMode)
    {
        focusNextChildAction->setShortcut(0);
        focusNextChildAction->setEnabled(false);

        focusPreviousChildAction->setShortcut(0);
        focusPreviousChildAction->setEnabled(false);
    }
    else
    {
        focusNextChildAction->setShortcut(Qt::Key_Tab);
        focusNextChildAction->setEnabled(true);

        focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
        focusPreviousChildAction->setEnabled(true);
    }
}

void PreviewWidget::OnIncrementScale()
{
    int nextIndex = -1;
    float actualScale = GetScaleFromComboboxText();
    if (actualScale >= percentages.last())
    {
        return;
    }
    if (percentages.contains(actualScale))
    {
        int currentIndex = scaleCombo->currentIndex();
        DVASSERT(currentIndex < scaleCombo->count() - 1);
        nextIndex = currentIndex + 1;
    }
    else
    {
        QList<float>::iterator iter = std::upper_bound(percentages.begin(), percentages.end(), actualScale);
        nextIndex = std::distance(percentages.begin(), iter);
    }
    DVASSERT(nextIndex >= 0 && nextIndex < percentages.size());
    scrollAreaController->SetScale(percentages.at(nextIndex));
}

void PreviewWidget::OnDecrementScale()
{
    int nextIndex = -1;
    float actualScale = GetScaleFromComboboxText();
    if (actualScale <= percentages.first())
    {
        return;
    }
    if (percentages.contains(actualScale))
    {
        int currentIndex = scaleCombo->currentIndex();
        DVASSERT(currentIndex > 0);
        nextIndex = currentIndex - 1;
    }
    else
    {
        QList<float>::iterator iter = std::lower_bound(percentages.begin(), percentages.end(), actualScale);
        nextIndex = std::distance(percentages.begin(), iter) - 1; //lower bound returns first largest element, but we need smaller;
    }
    DVASSERT(nextIndex >= 0 && nextIndex < percentages.size());
    scrollAreaController->SetScale(percentages.at(nextIndex));
}

void PreviewWidget::SetActualScale()
{
    scrollAreaController->SetScale(1.0f); //1.0f is a 100% scale
}

void PreviewWidget::ApplyPosChanges()
{
    QPoint viewPos = canvasPos + rootControlPos;
    rulerController->SetViewPos(-viewPos);
}

void PreviewWidget::UpdateScrollArea()
{
    QSize areaSize = scrollAreaController->GetViewSize();

    verticalScrollBar->setPageStep(areaSize.height());
    horizontalScrollBar->setPageStep(areaSize.width());

    QPoint minPos = scrollAreaController->GetMinimumPos();
    QPoint maxPos = scrollAreaController->GetMaximumPos();
    horizontalScrollBar->setRange(minPos.x(), maxPos.x());
    verticalScrollBar->setRange(minPos.y(), maxPos.y());
}

void PreviewWidget::OnPositionChanged(const QPoint& position)
{
    horizontalScrollBar->setSliderPosition(position.x());
    verticalScrollBar->setSliderPosition(position.y());
}

void PreviewWidget::OnGLInitialized()
{
    DVASSERT(nullptr == systemsManager);
    systemsManager.reset(new EditorSystemsManager());
    scrollAreaController->SetNestedControl(systemsManager->GetRootControl());
    scrollAreaController->SetMovableControl(systemsManager->GetScalableControl());
    systemsManager->CanvasSizeChanged.Connect(scrollAreaController, &ScrollAreaController::UpdateCanvasContentSize);
    systemsManager->RootControlPositionChanged.Connect(this, &PreviewWidget::OnRootControlPositionChanged);
    systemsManager->SelectionChanged.Connect(this, &PreviewWidget::OnSelectionInSystemsChanged);
    systemsManager->PropertyChanged.Connect(this, &PreviewWidget::OnPropertyChanged);
    systemsManager->TransformStateChanged.Connect(this, &PreviewWidget::OnTransformStateChanged);
    connect(focusNextChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusNextChild, systemsManager.get()));
    connect(focusPreviousChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusPreviousChild, systemsManager.get()));
    connect(selectAllAction, &QAction::triggered, std::bind(&EditorSystemsManager::SelectAll, systemsManager.get()));
}

void PreviewWidget::OnScaleChanged(float scale)
{
    bool wasBlocked = scaleCombo->blockSignals(true);
    int scaleIndex = percentages.indexOf(scale);
    if (scaleIndex != -1)
    {
        scaleCombo->setCurrentIndex(scaleIndex);
    }
    scaleCombo->lineEdit()->setText(ScaleStringFromReal(scale));
    scaleCombo->blockSignals(wasBlocked);

    rulerController->SetScale(scale);
    float32 realScale = static_cast<float32>(scale);
    if (systemsManager)
    {
        systemsManager->GetScalableControl()->SetScale(Vector2(realScale, realScale));
    }
}

void PreviewWidget::OnScaleByComboIndex(int index)
{
    DVASSERT(index >= 0);
    float scale = static_cast<float>(percentages.at(index));
    scrollAreaController->SetScale(scale);
}

void PreviewWidget::OnScaleByComboText()
{
    float scale = GetScaleFromComboboxText();
    scrollAreaController->SetScale(scale);
}

void PreviewWidget::OnGLWidgetResized(int width, int height)
{
    scrollAreaController->SetViewSize(QSize(width, height));
    UpdateScrollArea();
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setY(vPosition);
    scrollAreaController->SetPosition(canvasPosition);
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setX(hPosition);
    scrollAreaController->SetPosition(canvasPosition);
}

bool PreviewWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == davaGLWidget->GetGLView())
    {
        switch (event->type())
        {
        case QEvent::Wheel:
            OnWheelEvent(static_cast<QWheelEvent*>(event));
            break;
        case QEvent::NativeGesture:
            OnNativeGuestureEvent(static_cast<QNativeGestureEvent*>(event));
            break;
        case QEvent::MouseMove:
            OnMoveEvent(static_cast<QMouseEvent*>(event));
            return CanDragScreen();
        case QEvent::MouseButtonPress:
            OnPressEvent(static_cast<QMouseEvent*>(event));
            return CanDragScreen();
        case QEvent::MouseButtonRelease:
            OnReleaseEvent(static_cast<QMouseEvent*>(event));
            break;
        case QEvent::MouseButtonDblClick:
            OnDoubleClickEvent(static_cast<QMouseEvent*>(event));
            break;
        case QEvent::DragEnter:
            return true;
        case QEvent::DragMove:
            OnDragMoveEvent(static_cast<QDragMoveEvent*>(event));
            return true;
        case QEvent::DragLeave:
            OnDragLeaveEvent(static_cast<QDragLeaveEvent*>(event));
            return true;
        case QEvent::Drop:
            OnDropEvent(static_cast<QDropEvent*>(event));
            davaGLWidget->GetGLView()->requestActivate();
            break;
        case QEvent::KeyPress:
            OnKeyPressed(static_cast<QKeyEvent*>(event));
            break;
        case QEvent::KeyRelease:
            OnKeyReleased(static_cast<QKeyEvent*>(event));
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PreviewWidget::ShowMenu(const QPoint& pos)
{
    QMenu menu;
    //separator must be added by the client code, which call AddSelectionMenuSection function
    if (AddSelectionMenuSection(&menu, pos))
    {
        menu.addSeparator();
    }
    Vector2 davaPoint(pos.x(), pos.y());
    ControlNode* node = systemsManager->GetControlNodeUnderPoint(davaPoint);
    if (CanChangeTextInControl(node))
    {
        QString name = QString::fromStdString(node->GetName());
        QAction* action = menu.addAction(tr("Change text in %1").arg(name));
        connect(action, &QAction::triggered, [this, node]() { ChangeControlText(node); });
    }
    if (!menu.actions().isEmpty())
    {
        QPoint globalPos = davaGLWidget->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}

bool PreviewWidget::AddSelectionMenuSection(QMenu* menu, const QPoint& pos)
{
    Vector<ControlNode*> nodesUnderPoint;
    Vector2 davaPos(pos.x(), pos.y());
    auto predicateForMenu = [davaPos](const ControlNode* node) -> bool
    {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        const VisibleValueProperty* visibleProp = node->GetRootProperty()->GetVisibleProperty();
        return visibleProp->GetVisibleInEditor() && control->IsPointInside(davaPos);
    };
    auto stopPredicate = [](const ControlNode* node) -> bool {
        const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
        return !visibleProp->GetVisibleInEditor();
    };
    systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicateForMenu, stopPredicate);

    //create list of item to select
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode* controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QAction* action = new QAction(QString::fromStdString(controlNode->GetName()), menu);
        action->setCheckable(true);
        menu->addAction(action);
        if (selectionContainer.IsSelected(controlNode))
        {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, [this, controlNode]() {
            systemsManager->SelectNode(controlNode);
        });
    }
    return !nodesUnderPoint.empty();
}

bool PreviewWidget::CanChangeTextInControl(const ControlNode* node) const
{
    if (node == nullptr)
    {
        return false;
    }

    UIControl* control = node->GetControl();

    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    return staticText != nullptr;
}

void PreviewWidget::ChangeControlText(ControlNode* node)
{
    DVASSERT(node != nullptr);

    UIControl* control = node->GetControl();

    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    DVASSERT(staticText != nullptr);

    RootProperty* rootProperty = node->GetRootProperty();
    AbstractProperty* textProperty = rootProperty->FindPropertyByName("Text");
    DVASSERT(textProperty != nullptr);

    String text = textProperty->GetValue().AsString();

    QString label = tr("Enter new text, please");
    bool ok;
    QString inputText = MultilineTextInputDialog::GetMultiLineText(this, label, label, QString::fromStdString(text), &ok);
    if (ok)
    {
        DVASSERT(document != nullptr);
        QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();
        executor->BeginMacro("change text by user");
        AbstractProperty* multilineProperty = rootProperty->FindPropertyByName("Multi Line");
        DVASSERT(multilineProperty != nullptr);
        UIStaticText::eMultiline multilineType = static_cast<UIStaticText::eMultiline>(multilineProperty->GetValue().AsInt32());
        if (inputText.contains('\n') && multilineType == UIStaticText::MULTILINE_DISABLED)
        {
            executor->ChangeProperty(node, multilineProperty, VariantType(UIStaticText::MULTILINE_ENABLED));
        }
        executor->ChangeProperty(node, textProperty, VariantType(inputText.toStdString()));
        executor->EndMacro();
    }
}

void PreviewWidget::LoadContext()
{
    PreviewContext* context = DynamicTypeCheck<PreviewContext*>(document->GetContext(this));
    if (nullptr == context)
    {
        context = new PreviewContext();
        document->SetContext(this, context);
        QPoint position(horizontalScrollBar->maximum() / 2.0f, verticalScrollBar->maximum() / 2.0f);
        scrollAreaController->SetPosition(position);
    }
    else
    {
        scrollAreaController->SetPosition(context->canvasPosition);
    }
}

void PreviewWidget::SaveContext()
{
    if (document.isNull())
    {
        return;
    }
    PreviewContext* context = DynamicTypeCheck<PreviewContext*>(document->GetContext(this));
    context->canvasPosition = scrollAreaController->GetPosition();
}

void PreviewWidget::OnWheelEvent(QWheelEvent* event)
{
    if (document.isNull())
    {
        return;
    }
    bool shouldZoom = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
#if defined(Q_OS_WIN)
    ;
#elif defined(Q_OS_MAC)
    && event->source() == Qt::MouseEventNotSynthesized;
#else
#error "wrong platform"
#endif //platform
    if (shouldZoom)
    {
        //resize view
        int tickSize = 120;
        int ticksCount = event->angleDelta().y() / tickSize;
        if (ticksCount == 0)
        {
            return;
        }
        float scale = GetScaleFromWheelEvent(ticksCount);
        QPoint pos = event->pos();
        scrollAreaController->AdjustScale(scale, pos);
    }
    else
    {
#if defined(Q_OS_WIN)
        QPoint delta = event->angleDelta();
#else //Q_OS_MAC
        QPoint delta = event->pixelDelta();
#endif //platform
        //scroll view up and down
        static const float wheelDelta = 0.002f;
        QPoint position = scrollAreaController->GetPosition();
        QPoint additionalPos((delta.x() * horizontalScrollBar->pageStep()) * wheelDelta,
                             (delta.y() * verticalScrollBar->pageStep()) * wheelDelta);
        scrollAreaController->SetPosition(position - additionalPos);
    }
}

void PreviewWidget::OnNativeGuestureEvent(QNativeGestureEvent* event)
{
    if (document.isNull())
    {
        return;
    }
    const float normalScale = 1.0f;
    const float expandedScale = 1.5f;
    float scale = scrollAreaController->GetScale();
    QPoint pos = event->pos();
    switch (event->gestureType())
    {
    case Qt::ZoomNativeGesture:
        scrollAreaController->AdjustScale(scale + event->value(), pos);
        break;
    case Qt::SmartZoomNativeGesture:
        scrollAreaController->AdjustScale((event->value() == 0.0f ? normalScale : expandedScale), pos);
        //event->value() returns 1 or 0
        break;
    default:
        break;
    }
}

void PreviewWidget::OnPressEvent(QMouseEvent* event)
{
    Qt::MouseButtons buttons = event->buttons();
    if (buttons & Qt::LeftButton)
    {
        isMouseLeftButtonPressed = true;
    }
    if (buttons & Qt::MidButton)
    {
        isMouseMidButtonPressed = true;
    }
    if (buttons & Qt::RightButton)
    {
        ShowMenu(event->pos());
    }
    UpdateDragScreenState();
    if (CanDragScreen())
    {
        lastMousePos = event->pos();
    }
}

void PreviewWidget::OnReleaseEvent(QMouseEvent* event)
{
    Qt::MouseButtons buttons = event->buttons();
    if ((buttons & Qt::LeftButton) == false)
    {
        isMouseLeftButtonPressed = false;
    }
    if ((buttons & Qt::MidButton) == false)
    {
        isMouseMidButtonPressed = false;
    }

    UpdateDragScreenState();
}

void PreviewWidget::OnDoubleClickEvent(QMouseEvent* event)
{
    QPoint point = event->pos();

    Vector2 davaPoint(point.x(), point.y());
    ControlNode* node = systemsManager->GetControlNodeUnderPoint(davaPoint);
    if (!CanChangeTextInControl(node))
    {
        return;
    }
    //send mouse release event manually to clear InTransform state
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, event->pos(), event->button(), event->buttons(), event->modifiers());
    qApp->sendEvent(davaGLWidget->GetGLView(), &mouseEvent);
    // call "change text" after release event will pass
    QTimer::singleShot(0, [node, this]() { ChangeControlText(node); });
}

void PreviewWidget::OnMoveEvent(QMouseEvent* event)
{
    DVASSERT(nullptr != event);
    rulerController->UpdateRulerMarkers(event->pos());
    if (CanDragScreen())
    {
        QPoint delta(event->pos() - lastMousePos);
        lastMousePos = event->pos();

        int horizontalScrollBarValue = horizontalScrollBar->value();
        horizontalScrollBarValue -= delta.x();
        horizontalScrollBar->setValue(horizontalScrollBarValue);

        int verticalScrollBarValue = verticalScrollBar->value();
        verticalScrollBarValue -= delta.y();
        verticalScrollBar->setValue(verticalScrollBarValue);
    }
}

void PreviewWidget::OnDragMoveEvent(QDragMoveEvent* event)
{
    DVASSERT(nullptr != event);
    ProcessDragMoveEvent(event) ? event->accept() : event->ignore();
}

bool PreviewWidget::ProcessDragMoveEvent(QDropEvent* event)
{
    DVASSERT(nullptr != event);
    auto mimeData = event->mimeData();
    if (mimeData->hasFormat("text/uri-list"))
    {
        QStringList strList = mimeData->text().split("\n");
        for (const auto& str : strList)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                QString path = url.toLocalFile();
                QFileInfo fileInfo(path);
                return fileInfo.isFile() && fileInfo.suffix() == "yaml";
            }
        }
    }
    else if (mimeData->hasFormat("text/plain") || mimeData->hasFormat(PackageMimeData::MIME_TYPE))
    {
        DVASSERT(nullptr != document);
        ControlNode* node = HighlightNodeUnderPoint(event->pos());

        if (nullptr != node)
        {
            if (node->IsReadOnly())
            {
                return false;
            }
            else
            {
                if (mimeData->hasFormat(PackageMimeData::MIME_TYPE))
                {
                    const PackageMimeData* controlMimeData = DynamicTypeCheck<const PackageMimeData*>(mimeData);
                    const Vector<ControlNode*>& srcControls = controlMimeData->GetControls();
                    for (const auto& srcNode : srcControls)
                    {
                        if (srcNode == node)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
        }
        else
        {
            //root node will be added
            return true;
        }
    }
    return false;
}

void PreviewWidget::OnDragLeaveEvent(QDragLeaveEvent*)
{
    ClearHightlight();
}

void PreviewWidget::OnDropEvent(QDropEvent* event)
{
    ClearHightlight();
    DVASSERT(nullptr != event);
    auto mimeData = event->mimeData();
    if (mimeData->hasFormat("text/plain") || mimeData->hasFormat(PackageMimeData::MIME_TYPE))
    {
        Vector2 pos(event->pos().x(), event->pos().y());
        PackageBaseNode* node = systemsManager->GetControlNodeUnderPoint(pos);
        String string = mimeData->text().toStdString();
        auto action = event->dropAction();
        uint32 index = 0;
        if (node == nullptr)
        {
            node = DynamicTypeCheck<PackageBaseNode*>(document->GetPackage()->GetPackageControlsNode());
            index = systemsManager->GetIndexOfNearestControl(pos - systemsManager->GetScalableControl()->GetPosition());
        }
        else
        {
            index = node->GetCount();
        }
        emit DropRequested(mimeData, action, node, index, &pos);
    }
    else if (mimeData->hasFormat("text/uri-list"))
    {
        QStringList list = mimeData->text().split("\n");
        Vector<FilePath> packages;
        for (const QString& str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                emit OpenPackageFile(url.toLocalFile());
            }
        }
    }
}

void PreviewWidget::OnKeyPressed(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Space)
    {
        isSpacePressed = true;
        UpdateDragScreenState();
    }
    if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        SelectedNodes selectedNodes = selectionContainer.selectedNodes;
        if (selectedNodes.size() == 1)
        {
            ControlNode* node = dynamic_cast<ControlNode*>(*selectedNodes.begin());
            if (CanChangeTextInControl(node))
            {
                ChangeControlText(node);
            }
        }
    }
}

void PreviewWidget::OnKeyReleased(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
    {
        isSpacePressed = false;
        UpdateDragScreenState();
    }
}

void PreviewWidget::OnTransformStateChanged(bool inTransformState)
{
    if (document == nullptr)
    {
        return;
    }
    QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();
    if (inTransformState)
    {
        executor->BeginMacro("transformations");
    }
    else
    {
        executor->EndMacro();
    }
}

void PreviewWidget::OnPropertyChanged(ControlNode* node, AbstractProperty* property, VariantType newValue)
{
    DVASSERT(!document.isNull());
    QtModelPackageCommandExecutor* commandExecutor = document->GetCommandExecutor();
    commandExecutor->ChangeProperty(node, property, newValue);
}

float PreviewWidget::GetScaleFromWheelEvent(int ticksCount) const
{
    float scale = scrollAreaController->GetScale();
    if (ticksCount > 0)
    {
        scale = GetNextScale(scale, ticksCount);
    }
    else if (ticksCount < 0)
    {
        scale = GetPreviousScale(scale, ticksCount);
    }
    return scale;
}

float PreviewWidget::GetNextScale(float currentScale, int ticksCount) const
{
    auto iter = std::upper_bound(percentages.begin(), percentages.end(), currentScale);
    if (iter == percentages.end())
    {
        return currentScale;
    }
    ticksCount--;
    int distance = std::distance(iter, percentages.end());
    ticksCount = std::min(distance, ticksCount);
    std::advance(iter, ticksCount);
    return iter != percentages.end() ? *iter : percentages.last();
}

void PreviewWidget::OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    for (const auto& node : deselected)
    {
        tmpSelected.erase(node);
        tmpDeselected.insert(node);
    }
    for (const auto& node : selected)
    {
        tmpSelected.insert(node);
        tmpDeselected.erase(node);
    }
    selectionContainer.MergeSelection(selected, deselected);
    continuousUpdater->Update();
}

void PreviewWidget::NotifySelectionChanged()
{
    if (!tmpSelected.empty() || !tmpDeselected.empty())
    {
        emit SelectionChanged(tmpSelected, tmpDeselected);
    }
    tmpSelected.clear();
    tmpDeselected.clear();
}

bool PreviewWidget::CanDragScreen() const
{
    return inDragScreenState;
}

void PreviewWidget::UpdateDragScreenState()
{
    bool inDragScreenState_ = isMouseMidButtonPressed || (isMouseLeftButtonPressed && isSpacePressed);
    if (inDragScreenState == inDragScreenState_)
    {
        return;
    }
    inDragScreenState = inDragScreenState_;
    if (inDragScreenState)
    {
        lastCursor = davaGLWidget->GetCursor();
        davaGLWidget->SetCursor(Qt::OpenHandCursor);
    }
    else
    {
        davaGLWidget->SetCursor(lastCursor);
    }
}

float PreviewWidget::GetPreviousScale(float currentScale, int ticksCount) const
{
    auto iter = std::lower_bound(percentages.begin(), percentages.end(), currentScale);
    if (iter == percentages.end())
    {
        return currentScale;
    }
    int distance = std::distance(iter, percentages.begin());
    ticksCount = std::max(ticksCount, distance);
    std::advance(iter, ticksCount);
    return *iter;
}
