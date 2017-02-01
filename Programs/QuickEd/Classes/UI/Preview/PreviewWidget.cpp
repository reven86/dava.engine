#include "PreviewWidget.h"

#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlSystem.h"
#include "Engine/Engine.h"

#include "QtTools/Updaters/ContinuousUpdater.h"
#include "QtTools/InputDialogs/MultilineTextInputDialog.h"

#include "Document/Document.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "EditorSystems/EditorCanvas.h"
#include "Ruler/RulerWidget.h"
#include "Ruler/RulerController.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "UI/Package/PackageMimeData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortCut>
#include <QFileInfo>
#include <QInputDialog>
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

struct SystemsContext : WidgetContext
{
    SelectedNodes selection;
};
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
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

    verticalRuler->SetRulerOrientation(Qt::Vertical);

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
    continuousUpdater->Stop();
}

void PreviewWidget::SelectControl(const DAVA::String& path)
{
    if (document != nullptr)
    {
        PackageNode* package = document->GetPackage();
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
    InitEditorSystems();

    renderWidget->resized.Connect(this, &PreviewWidget::OnResized);

    renderWidget->SetClientDelegate(this);
    frame->layout()->addWidget(renderWidget);
    CreateActions();
}

void PreviewWidget::CreateActions()
{
    QAction* importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PreviewWidget::ImportRequested);
    renderWidget->addAction(importPackageAction);

    QAction* cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(cutAction, &QAction::triggered, this, &PreviewWidget::CutRequested);
    renderWidget->addAction(cutAction);

    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copyAction, &QAction::triggered, this, &PreviewWidget::CopyRequested);
    renderWidget->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteAction, &QAction::triggered, this, &PreviewWidget::PasteRequested);
    renderWidget->addAction(pasteAction);

    QAction* deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform

    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    renderWidget->addAction(deleteAction);

    selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(selectAllAction);

    focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(focusNextChildAction);

    focusPreviousChildAction = new QAction(tr("Focus frevious child"), this);
    focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
    focusPreviousChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(focusPreviousChildAction);
}

void PreviewWidget::OnDocumentChanged(Document* arg)
{
    SaveSystemsContextAndClear();

    DVASSERT(nullptr != systemsManager);
    continuousUpdater->Stop();
    SaveContext();
    document = arg;
    systemsManager->magnetLinesChanged.Emit({});
    systemsManager->ClearHighlight();
    if (document.isNull())
    {
        systemsManager->packageChanged.Emit(nullptr);
    }
    else
    {
        systemsManager->packageChanged.Emit(document->GetPackage());
        LoadContext();
    }

    LoadSystemsContext(arg);
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
            systemsManager->selectionChanged.Emit(selectionContainer.selectedNodes, SelectedNodes());
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

void PreviewWidget::InitEditorSystems()
{
    DVASSERT(nullptr == systemsManager);
    systemsManager.reset(new EditorSystemsManager(renderWidget));

    systemsManager->rootControlPositionChanged.Connect(this, &PreviewWidget::OnRootControlPositionChanged);
    systemsManager->selectionChanged.Connect(this, &PreviewWidget::OnSelectionInSystemsChanged);
    systemsManager->propertyChanged.Connect(this, &PreviewWidget::OnPropertyChanged);
    systemsManager->dragStateChanged.Connect(this, &PreviewWidget::OnDragStateChanged);

    connect(focusNextChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusNextChild, systemsManager.get()));
    connect(focusPreviousChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusPreviousChild, systemsManager.get()));
    connect(selectAllAction, &QAction::triggered, std::bind(&EditorSystemsManager::SelectAll, systemsManager.get()));
    editorCanvas = systemsManager->GetEditorCanvas();
    editorCanvas->sizeChanged.Connect(this, &PreviewWidget::UpdateScrollArea);
    editorCanvas->ositionChanged.Connect(this, &PreviewWidget::OnPositionChanged);
    editorCanvas->nestedControlPositionChanged.Connect(this, &PreviewWidget::OnNestedControlPositionChanged);
    editorCanvas->scaleChanged.Connect(this, &PreviewWidget::OnScaleChanged);
    //fastest way to apply displayed scale to the editorCanvas
    OnScaleByComboText();
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
        connect(action, &QAction::triggered, [this, node]() { ChangeControlText(node); });
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

void PreviewWidget::ChangeControlText(ControlNode* node)
{
    DVASSERT(node != nullptr);

    UIControl* control = node->GetControl();

    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    DVASSERT(staticText != nullptr);

    RootProperty* rootProperty = node->GetRootProperty();
    AbstractProperty* textProperty = rootProperty->FindPropertyByName("text");
    DVASSERT(textProperty != nullptr);

    String text = textProperty->GetValue().Get<String>();

    QString label = tr("Enter new text, please");
    bool ok;
    QString inputText = MultilineTextInputDialog::GetMultiLineText(this, label, label, QString::fromStdString(text), &ok);
    if (ok)
    {
        DVASSERT(document != nullptr);
        QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();
        executor->BeginMacro("change text by user");
        AbstractProperty* multilineProperty = rootProperty->FindPropertyByName("multiline");
        DVASSERT(multilineProperty != nullptr);
        UIStaticText::eMultiline multilineType = static_cast<UIStaticText::eMultiline>(multilineProperty->GetValue().Get<int32>());
        if (inputText.contains('\n') && multilineType == UIStaticText::MULTILINE_DISABLED)
        {
            executor->ChangeProperty(node, multilineProperty, UIStaticText::MULTILINE_ENABLED);
        }
        executor->ChangeProperty(node, textProperty, inputText.toStdString());
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
        Vector2 position(horizontalScrollBar->maximum() / 2.0f, verticalScrollBar->maximum() / 2.0f);
        editorCanvas->SetPosition(position);
    }
    else
    {
        editorCanvas->SetPosition(context->canvasPosition);
    }
}

void PreviewWidget::SaveContext()
{
    if (document.isNull())
    {
        return;
    }

    //check that we do not leave document in non valid state
    DVASSERT(document->GetPackage()->CanUpdateAll());
    PreviewContext* context = DynamicTypeCheck<PreviewContext*>(document->GetContext(this));
    context->canvasPosition = editorCanvas->GetPosition();
}

void PreviewWidget::OnWheel(QWheelEvent* event)
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
    if (document.isNull())
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
        ChangeControlText(nodeToChangeTextOnMouseRelease);
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
        DVASSERT(nullptr != document);
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
            node = DynamicTypeCheck<PackageBaseNode*>(document->GetPackage()->GetPackageControlsNode());
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
                ChangeControlText(node);
            }
        }
    }
}

void PreviewWidget::OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState)
{
    if (document == nullptr)
    {
        return;
    }
    QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();

    if (dragState == EditorSystemsManager::Transform)
    {
        document->SetCanClose(false);
        executor->BeginMacro("transformations");
    }
    else if (previousState == EditorSystemsManager::Transform)
    {
        document->SetCanClose(true);
        executor->EndMacro();
    }
}

void PreviewWidget::OnPropertyChanged(ControlNode* node, AbstractProperty* property, Any newValue)
{
    DVASSERT(!document.isNull());
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
