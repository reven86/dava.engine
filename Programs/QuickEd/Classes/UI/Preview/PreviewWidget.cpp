#include "PreviewWidget.h"

#include "EditorSystems/EditorSystemsManager.h"

#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/CursorSystem.h"

#include "Ruler/RulerWidget.h"
#include "Ruler/RulerController.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "UI/Package/PackageMimeData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

#include "Modules/DocumentsModule/Document.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/WidgetsData.h"

#include <TArc/Controls/SceneTabbar.h>
#include <TArc/Models/SceneTabsModel.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <UI/UIControl.h>
#include <UI/UIStaticText.h>
#include <UI/UIControlSystem.h>
#include <Engine/Engine.h>

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortCut>
#include <QFileInfo>
#include <QInputDialog>
#include <QComboBox>
#include <QScrollBar>
#include <QGridLayout>

#include <QApplication>
#include <QTimer>

using namespace DAVA;

namespace
{
QString ScaleStringFromReal(float scale)
{
    return QString("%1 %").arg(static_cast<int>(scale * 100.0f + 0.5f));
}

struct PreviewContext : WidgetContext
{
    Vector2 canvasPosition;
};
}

PreviewWidget::PreviewWidget(DAVA::TArc::ContextAccessor* accessor_, DAVA::RenderWidget* renderWidget, EditorSystemsManager* systemsManager)
    : QFrame()
    , accessor(accessor_)
    , rulerController(new RulerController(this))
    , continuousUpdater(new ContinuousUpdater(MakeFunction(this, &PreviewWidget::NotifySelectionChanged), this, 300))
{
    qRegisterMetaType<SelectedNodes>("SelectedNodes");
    percentages << 0.25f << 0.33f << 0.50f << 0.67f << 0.75f << 0.90f
                << 1.00f << 1.10f << 1.25f << 1.50f << 1.75f << 2.00f
                << 2.50f << 3.00f << 4.00f << 5.00f << 6.00f << 7.00f << 8.00f;
    InjectRenderWidget(renderWidget);

    InitUI(accessor);

    InitFromSystemsManager(systemsManager);

    connect(rulerController, &RulerController::HorisontalRulerSettingsChanged, horizontalRuler, &RulerWidget::OnRulerSettingsChanged);
    connect(rulerController, &RulerController::VerticalRulerSettingsChanged, verticalRuler, &RulerWidget::OnRulerSettingsChanged);

    connect(rulerController, &RulerController::HorisontalRulerMarkPositionChanged, horizontalRuler, &RulerWidget::OnMarkerPositionChanged);
    connect(rulerController, &RulerController::VerticalRulerMarkPositionChanged, verticalRuler, &RulerWidget::OnMarkerPositionChanged);

    // Setup the Scale Combo.
    for (auto percentage : percentages)
    {
        scaleCombo->addItem(ScaleStringFromReal(percentage));
    }

    connect(scaleCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    QRegExp regEx("[0-8]?([0-9]|[0-9]){0,2}\\s?\\%?");
    scaleCombo->setValidator(new QRegExpValidator(regEx));
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    OnScaleChanged(1.0f);
    UpdateScrollArea();
}

PreviewWidget::~PreviewWidget()
{
    renderWidget->setParent(nullptr);
    continuousUpdater->Stop();
}

void PreviewWidget::SelectControl(const DAVA::String& path)
{
    using namespace DAVA::TArc;
    DataContext* dataContext = accessor->GetActiveContext();
    if (dataContext != nullptr)
    {
        DocumentData* documentData = dataContext->GetData<DocumentData>();
        DVASSERT(nullptr != documentData);
        PackageNode* package = documentData->package.Get();
        ControlNode* node = package->GetPrototypes()->FindControlNodeByPath(path);
        if (!node)
        {
            node = package->GetPackageControlsNode()->FindControlNodeByPath(path);
        }
        if (node != nullptr)
        {
            systemsManager->ClearSelection();
            systemsManager->SelectNode(node);
        }
    }
}

float PreviewWidget::GetScaleFromComboboxText() const
{
    // Firstly verify whether the value is already set.
    QString curTextValue = scaleCombo->currentText();
    curTextValue.remove('%');
    curTextValue.remove(' ');
    bool ok;
    float scaleValue = curTextValue.toFloat(&ok);
    DVASSERT(ok, "can not parse text to float");
    return scaleValue / 100.0f;
}

void PreviewWidget::InjectRenderWidget(DAVA::RenderWidget* renderWidget_)
{
    DVASSERT(renderWidget_ != nullptr);
    renderWidget = renderWidget_;
    CreateActions();


    renderWidget->resized.Connect(this, &PreviewWidget::OnResized);

    renderWidget->SetClientDelegate(this);
}

void PreviewWidget::CreateActions()
{
    QAction* importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PreviewWidget::ImportRequested);
    addAction(importPackageAction);

    QAction* cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(cutAction, &QAction::triggered, this, &PreviewWidget::CutRequested);
    addAction(cutAction);

    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copyAction, &QAction::triggered, this, &PreviewWidget::CopyRequested);
    addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteAction, &QAction::triggered, this, &PreviewWidget::PasteRequested);
    addAction(pasteAction);

    QAction* deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform

    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    addAction(deleteAction);

    selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAllAction);

    focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(focusNextChildAction);

    focusPreviousChildAction = new QAction(tr("Focus frevious child"), this);
    focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
    focusPreviousChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(focusPreviousChildAction);
}

void PreviewWidget::OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
    continuousUpdater->Stop();
    if (oldOne != nullptr)
    {
        DocumentData* documentData = oldOne->GetData<DocumentData>();
        DVASSERT(nullptr != documentData);
        //check that we do not leave document in non valid state
        DVASSERT(documentData->package->CanUpdateAll());

        WidgetsData* widgetsData = oldOne->GetData<WidgetsData>();
        DVASSERT(widgetsData != nullptr);

        PreviewContext* context = DynamicTypeCheck<PreviewContext*>(widgetsData->GetContext(this));
        context->canvasPosition = editorCanvas->GetPosition();
    };
    if (current != nullptr)
    {
        WidgetsData* widgetsData = current->GetData<WidgetsData>();
        DVASSERT(widgetsData != nullptr);

        PreviewContext* context = DynamicTypeCheck<PreviewContext*>(widgetsData->GetContext(this));
        if (nullptr == context)
        {
            context = new PreviewContext();
            widgetsData->SetContext(this, context);
            Vector2 position(horizontalScrollBar->maximum() / 2.0f, verticalScrollBar->maximum() / 2.0f);
            editorCanvas->SetPosition(position);
        }
        else
        {
            editorCanvas->SetPosition(context->canvasPosition);
        }
    }
}

void PreviewWidget::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    systemsManager->selectionChanged.Emit(selected, deselected);
}

void PreviewWidget::OnRootControlPositionChanged(const Vector2& pos)
{
    rootControlPos = QPoint(static_cast<int>(pos.x), static_cast<int>(pos.y));
    ApplyPosChanges();
}

void PreviewWidget::OnNestedControlPositionChanged(const Vector2& pos)
{
    canvasPos = QPoint(static_cast<int>(pos.x), static_cast<int>(pos.y));
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
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetScale(percentages.at(nextIndex));
    }
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
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetScale(percentages.at(nextIndex));
    }
}

void PreviewWidget::SetActualScale()
{
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetScale(1.0f); //1.0f is a 100% scale
    }
}

void PreviewWidget::ApplyPosChanges()
{
    QPoint viewPos = canvasPos + rootControlPos;
    rulerController->SetViewPos(-viewPos);
}

void PreviewWidget::UpdateScrollArea(const DAVA::Vector2& /*size*/)
{
    if (editorCanvas == nullptr)
    {
        verticalScrollBar->setPageStep(0);
        horizontalScrollBar->setPageStep(0);

        verticalScrollBar->setRange(0, 0);
        horizontalScrollBar->setRange(0, 0);
    }
    else
    {
        Vector2 areaSize = editorCanvas->GetViewSize();

        verticalScrollBar->setPageStep(areaSize.dy);
        horizontalScrollBar->setPageStep(areaSize.dx);

        Vector2 minPos = editorCanvas->GetMinimumPos();
        Vector2 maxPos = editorCanvas->GetMaximumPos();
        horizontalScrollBar->setRange(minPos.x, maxPos.x);
        verticalScrollBar->setRange(minPos.y, maxPos.y);
    }
}

void PreviewWidget::OnPositionChanged(const Vector2& position)
{
    horizontalScrollBar->setSliderPosition(position.x);
    verticalScrollBar->setSliderPosition(position.y);
}

void PreviewWidget::OnResized(DAVA::uint32 width, DAVA::uint32 height)
{
    systemsManager->viewSizeChanged.Emit(width, height);

    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    vcs->UnregisterAllAvailableResourceSizes();
    vcs->SetVirtualScreenSize(width, height);
    vcs->RegisterAvailableResourceSize(width, height, "Gfx");
    vcs->RegisterAvailableResourceSize(width, height, "Gfx2");

    UpdateScrollArea();
}

void PreviewWidget::InitFromSystemsManager(EditorSystemsManager* systemsManager_)
{
    DVASSERT(nullptr == systemsManager);
    systemsManager = systemsManager_;

    systemsManager->rootControlPositionChanged.Connect(this, &PreviewWidget::OnRootControlPositionChanged);
    systemsManager->selectionChanged.Connect(this, &PreviewWidget::OnSelectionInSystemsChanged);
    systemsManager->propertyChanged.Connect(this, &PreviewWidget::OnPropertyChanged);
    systemsManager->dragStateChanged.Connect(this, &PreviewWidget::OnDragStateChanged);

    connect(focusNextChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusNextChild, systemsManager));
    connect(focusPreviousChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusPreviousChild, systemsManager));
    connect(selectAllAction, &QAction::triggered, std::bind(&EditorSystemsManager::SelectAll, systemsManager));

    editorCanvas = new EditorCanvas(systemsManager);
    editorCanvas->sizeChanged.Connect(this, &PreviewWidget::UpdateScrollArea);
    editorCanvas->ositionChanged.Connect(this, &PreviewWidget::OnPositionChanged);
    editorCanvas->nestedControlPositionChanged.Connect(this, &PreviewWidget::OnNestedControlPositionChanged);
    editorCanvas->scaleChanged.Connect(this, &PreviewWidget::OnScaleChanged);
    systemsManager->AddEditorSystem(editorCanvas);

    CursorSystem* cursorSystem = new CursorSystem(renderWidget, systemsManager);
    systemsManager->AddEditorSystem(cursorSystem);
}

void PreviewWidget::OnScaleChanged(float32 scale)
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
}

void PreviewWidget::OnScaleByComboIndex(int index)
{
    DVASSERT(index >= 0);
    float scale = static_cast<float>(percentages.at(index));
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetScale(scale);
    }
}

void PreviewWidget::OnScaleByComboText()
{
    float scale = GetScaleFromComboboxText();
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetScale(scale);
    }
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    Vector2 canvasPosition = editorCanvas->GetPosition();
    canvasPosition.y = vPosition;
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetPosition(canvasPosition);
    }
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    Vector2 canvasPosition = editorCanvas->GetPosition();
    canvasPosition.x = hPosition;
    if (editorCanvas != nullptr)
    {
        editorCanvas->SetPosition(canvasPosition);
    }
}

void PreviewWidget::InitUI(DAVA::TArc::ContextAccessor* accessor)
{
    gridLayout = new QGridLayout(this);

    DAVA::TArc::DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<SceneTabsModel>());
    DAVA::TArc::SceneTabbar* tabBar = new DAVA::TArc::SceneTabbar(accessor, DAVA::Reflection::Create(ctx->GetData<SceneTabsModel>()), this);
    tabBar->closeTab.Connect(&requestCloseTab, &DAVA::Signal<DAVA::uint64>::Emit);
    tabBar->setElideMode(Qt::ElideNone);
    tabBar->setTabsClosable(true);
    tabBar->setUsesScrollButtons(true);
    gridLayout->addWidget(tabBar, 0, 0, 1, 3);

    horizontalRuler = new RulerWidget(this);
    horizontalRuler->SetRulerOrientation(Qt::Horizontal);

    gridLayout->addWidget(horizontalRuler, 1, 1, 1, 2);

    verticalRuler = new RulerWidget(this);
    verticalRuler->SetRulerOrientation(Qt::Vertical);
    gridLayout->addWidget(verticalRuler, 2, 0, 1, 1);

    QSizePolicy expandingPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    renderWidget->setSizePolicy(expandingPolicy);
    gridLayout->addWidget(renderWidget, 2, 1, 1, 2);

    verticalScrollBar = new QScrollBar(this);
    verticalScrollBar->setOrientation(Qt::Vertical);

    gridLayout->addWidget(verticalScrollBar, 2, 3, 1, 1);

    scaleCombo = new QComboBox(this);
    scaleCombo->setEditable(true);

    gridLayout->addWidget(scaleCombo, 3, 1, 1, 1);

    horizontalScrollBar = new QScrollBar(this);
    horizontalScrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    horizontalScrollBar->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(horizontalScrollBar, 3, 2, 1, 1);
}

void PreviewWidget::ShowMenu(const QMouseEvent* mouseEvent)
{
    QMenu menu;
    //separator must be added by the client code, which call AddSelectionMenuSection function
    QPoint localPos = mouseEvent->pos();
    if (AddSelectionMenuSection(&menu, localPos))
    {
        menu.addSeparator();
    }
    Vector2 davaPoint(localPos.x(), localPos.y());
    ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPoint);
    if (CanChangeTextInControl(node))
    {
        QString name = QString::fromStdString(node->GetName());
        QAction* action = menu.addAction(tr("Change text in %1").arg(name));
        connect(action, &QAction::triggered, [this, node]() { requestChangeTextInNode.Emit(node); });
    }
    if (!menu.actions().isEmpty())
    {
        menu.exec(mouseEvent->globalPos());
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

void PreviewWidget::OnWheel(QWheelEvent* event)
{
    if (accessor->GetActiveContext() == nullptr)
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
        editorCanvas->AdjustScale(scale, Vector2(pos.x(), pos.y()));
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
        Vector2 position = editorCanvas->GetPosition();
        Vector2 additionalPos((delta.x() * horizontalScrollBar->pageStep()) * wheelDelta,
                              (delta.y() * verticalScrollBar->pageStep()) * wheelDelta);
        editorCanvas->SetPosition(position - additionalPos);
    }
}

void PreviewWidget::OnNativeGuesture(QNativeGestureEvent* event)
{
    if (accessor->GetActiveContext() == nullptr)
    {
        return;
    }
    const float normalScale = 1.0f;
    const float expandedScale = 1.5f;
    float scale = editorCanvas->GetScale();
    QPoint qtPos = event->pos();
    Vector2 pos(qtPos.x(), qtPos.y());
    switch (event->gestureType())
    {
    case Qt::ZoomNativeGesture:
        editorCanvas->AdjustScale(scale + event->value(), pos);
        break;
    case Qt::SmartZoomNativeGesture:
        //event->value() returns 1.0f or 0.0f
        editorCanvas->AdjustScale((event->value() == 0.0f ? normalScale : expandedScale), pos);
        break;
    default:
        break;
    }
}

void PreviewWidget::OnMouseReleased(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        ShowMenu(event);
    }
    if (nodeToChangeTextOnMouseRelease != nullptr)
    {
        requestChangeTextInNode.Emit(nodeToChangeTextOnMouseRelease);
        nodeToChangeTextOnMouseRelease = nullptr;
    }
}

void PreviewWidget::OnMouseDBClick(QMouseEvent* event)
{
    QPoint point = event->pos();

    Vector2 davaPoint(point.x(), point.y());
    ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPoint);
    if (!CanChangeTextInControl(node))
    {
        return;
    }

    // call "change text" after release event will pass
    nodeToChangeTextOnMouseRelease = node;
}

void PreviewWidget::OnMouseMove(QMouseEvent* event)
{
    DVASSERT(nullptr != event);
    rulerController->UpdateRulerMarkers(event->pos());
}

void PreviewWidget::OnDragEntered(QDragEnterEvent* event)
{
    event->accept();
}

void PreviewWidget::OnDragMoved(QDragMoveEvent* event)
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
        QPoint pos = event->pos();
        DAVA::Vector2 davaPos(pos.x(), pos.y());
        ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPos);
        systemsManager->HighlightNode(node);

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

void PreviewWidget::OnDragLeaved(QDragLeaveEvent*)
{
    systemsManager->ClearHighlight();
}

void PreviewWidget::OnDrop(QDropEvent* event)
{
    systemsManager->ClearHighlight();
    DVASSERT(nullptr != event);
    auto mimeData = event->mimeData();
    if (mimeData->hasFormat("text/plain") || mimeData->hasFormat(PackageMimeData::MIME_TYPE))
    {
        Vector2 pos(event->pos().x(), event->pos().y());
        PackageBaseNode* node = systemsManager->GetControlNodeAtPoint(pos);
        String string = mimeData->text().toStdString();
        auto action = event->dropAction();
        uint32 index = 0;
        if (node == nullptr)
        {
            DAVA::TArc::DataContext* active = accessor->GetActiveContext();
            DVASSERT(active != nullptr);
            DocumentData* data = active->GetData<DocumentData>();
            node = DynamicTypeCheck<PackageBaseNode*>(data->package->GetPackageControlsNode());
            index = systemsManager->GetIndexOfNearestRootControl(pos);
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
    renderWidget->setFocus();
}

void PreviewWidget::OnKeyPressed(QKeyEvent* event)
{
    if (event->isAutoRepeat())
    {
        return;
    }
    int key = event->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        SelectedNodes selectedNodes = selectionContainer.selectedNodes;
        if (selectedNodes.size() == 1)
        {
            ControlNode* node = dynamic_cast<ControlNode*>(*selectedNodes.begin());
            if (CanChangeTextInControl(node))
            {
                requestChangeTextInNode.Emit(node);
            }
        }
    }
}

void PreviewWidget::OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState)
{
    using namespace DAVA::TArc;
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(nullptr != documentData);
    Document* document = activeContext->GetData<Document>();
    DVASSERT(nullptr != document);
    //TODO: move this code to the TransformSystem when systems will be moved to the TArc
    QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();
    if (dragState == EditorSystemsManager::Transform)
    {
        documentData->canClose = false;
        executor->BeginMacro("transformations");
    }
    else if (previousState == EditorSystemsManager::Transform)
    {
        documentData->canClose = true;
        executor->EndMacro();
    }
}

void PreviewWidget::OnPropertyChanged(ControlNode* node, AbstractProperty* property, VariantType newValue)
{
    using namespace DAVA::TArc;
    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    //TODO: move this code to the TransformSystem when systems will be moved to the TArc
    Document* document = activeContext->GetData<Document>();
    DVASSERT(nullptr != document);
    QtModelPackageCommandExecutor* commandExecutor = document->GetCommandExecutor();
    commandExecutor->ChangeProperty(node, property, newValue);
}

float PreviewWidget::GetScaleFromWheelEvent(int ticksCount) const
{
    float scale = editorCanvas->GetScale();
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
