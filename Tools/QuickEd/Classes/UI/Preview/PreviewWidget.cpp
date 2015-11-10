/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "PreviewWidget.h"

#include "ScrollAreaController.h"

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortCut>
#include "UI/UIControl.h"
#include "UI/UIScreenManager.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

#include "Document.h"

using namespace DAVA;

namespace
{
QString ScaleStringFromInt(int scale)
{
    return QString("%1 %").arg(scale);
}

struct PreviewContext : WidgetContext
{
    ~PreviewContext() override = default;
    QPoint canvasPosition;
};
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , scrollAreaController(new ScrollAreaController(this))
{
    percentages << 10 << 25 << 50 << 75 << 100 << 125
                << 150 << 175 << 200 << 250 << 400 << 800;
    setupUi(this);
    davaGLWidget = new DavaGLWidget();
    frame->layout()->addWidget(davaGLWidget);
    davaGLWidget->GetGLView()->installEventFilter(this);

    davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    connect( davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized );

    // Setup the Scale Combo.
    for (auto percentage : percentages)
    {
        scaleCombo->addItem(ScaleStringFromInt(percentage));
    }
    connect(scrollAreaController, &ScrollAreaController::ViewSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::CanvasSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::PositionChanged, this, &PreviewWidget::OnPositionChanged);
    connect(scrollAreaController, &ScrollAreaController::ScaleChanged, this, &PreviewWidget::OnScaleChanged);
    
    connect(scaleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    connect(davaGLWidget, &DavaGLWidget::ScreenChanged, this, &PreviewWidget::OnMonitorChanged);

    scaleCombo->setCurrentIndex(percentages.indexOf(100)); //100%
    scaleCombo->lineEdit()->setMaxLength(6); //3 digits + whitespace + % ?
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    UpdateScrollArea();

    QAction* deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WindowShortcut); //widget shortcut is not working for davaGLWidget
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    davaGLWidget->addAction(deleteAction);

    QAction* selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WindowShortcut);
    connect(selectAllAction, &QAction::triggered, this, &PreviewWidget::SelectAllRequested);
    davaGLWidget->addAction(selectAllAction);

    QAction* focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WindowShortcut);
    connect(focusNextChildAction, &QAction::triggered, this, &PreviewWidget::FocusNextChild);
    davaGLWidget->addAction(focusNextChildAction);

    QAction* focusPreviousChildAction = new QAction(tr("Focus frevious child"), this);
#if defined Q_OS_WIN
    focusPreviousChildAction->setShortcut(Qt::Key_Backtab);
#elif defined Q_OS_MAC
    focusPreviousChildAction->setShortcut(Qt::ShiftModifier + Qt::Key_Tab);
#endif //Q_OS_WIN Q_OS_MAC
    focusPreviousChildAction->setShortcutContext(Qt::WindowShortcut);
    connect(focusPreviousChildAction, &QAction::triggered, this, &PreviewWidget::FocusPreviousChild);
    davaGLWidget->addAction(focusPreviousChildAction);
}

ScrollAreaController* PreviewWidget::GetScrollAreaController()
{
    return scrollAreaController;
}

float PreviewWidget::GetScale() const
{
    // Firstly verify whether the value is already set.
    QString curTextValue = scaleCombo->currentText().trimmed();
    int scaleValue = 0;
    if (curTextValue.endsWith(" %"))
    {
        int endCharPos = curTextValue.lastIndexOf(" %");
        QString remainderNumber = curTextValue.left(endCharPos);
        scaleValue = remainderNumber.toInt();
    }
    else
    {
        // Try to parse the value.
        scaleValue = curTextValue.toFloat();
    }
    return scaleValue;
}

qreal PreviewWidget::GetDPR() const
{
    return davaGLWidget->devicePixelRatio();
}

ControlNode* PreviewWidget::OnSelectControlByMenu(const Vector<ControlNode*>& nodesUnderPoint, const Vector2& point)
{
    QPoint globalPos = davaGLWidget->mapToGlobal(QPoint(point.x, point.y) / davaGLWidget->devicePixelRatio());
    QMenu menu;
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode* controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QAction* action = new QAction(QString::fromStdString(controlNode->GetName()), &menu);
        action->setCheckable(true);
        menu.addAction(action);
        void* ptr = static_cast<void*>(controlNode);
        action->setData(QVariant::fromValue(ptr));
        if (selectionContainer.IsSelected(controlNode))
        {
            action->setChecked(true);
        }
    }
    QAction* selectedAction = menu.exec(globalPos);
    if (nullptr != selectedAction)
    {
        void* ptr = selectedAction->data().value<void*>();
        return static_cast<ControlNode*>(ptr);
    }
    return nullptr;
}

void PreviewWidget::OnDocumentChanged(Document* arg)
{
    document = arg;
    if (nullptr != document)
    {
        EditorSystemsManager* systemManager = document->GetSystemManager();
        UIControl* root = systemManager->GetRootControl();
        DVASSERT(nullptr != root);
        scrollAreaController->SetNestedControl(root);
    }
    else
    {
        scrollAreaController->SetNestedControl(nullptr);
    }
}

void PreviewWidget::OnDocumentActivated(Document* document)
{
    PreviewContext* context = static_cast<PreviewContext*>(document->GetContext(this));
    if (nullptr == context)
    {
        context = new PreviewContext();
        document->SetContext(this, context);
        QPoint position(horizontalScrollBar->maximum() / 2.0f, verticalScrollBar->maximum() / 2.0f);
        scrollAreaController->SetPosition(position);
        document->GetSystemManager()->GetControlByMenu = std::bind(&PreviewWidget::OnSelectControlByMenu, this, _1, _2);
    }
    else
    {
        scrollAreaController->SetPosition(context->canvasPosition);
    }
}

void PreviewWidget::OnDocumentDeactivated(Document* document)
{
    PreviewContext* context = static_cast<PreviewContext*>(document->GetContext(this));
    context->canvasPosition = scrollAreaController->GetPosition();
}

void PreviewWidget::SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
}

void PreviewWidget::OnMonitorChanged()
{
    SetDPR(davaGLWidget->devicePixelRatio());
}

void PreviewWidget::UpdateScrollArea()
{
    QSize areaSize = scrollAreaController->GetViewSize();
    QSize contentSize = scrollAreaController->GetCanvasSize();

    verticalScrollBar->setPageStep(areaSize.height());
    horizontalScrollBar->setPageStep(areaSize.width());

    verticalScrollBar->setRange(0, contentSize.height() - areaSize.height());
    horizontalScrollBar->setRange(0, contentSize.width() - areaSize.width());
}

void PreviewWidget::OnPositionChanged(const QPoint& position)
{
    horizontalScrollBar->setSliderPosition(position.x());
    verticalScrollBar->setSliderPosition(position.y());
}

void PreviewWidget::OnScaleChanged(qreal scale)
{
    scaleCombo->lineEdit()->setText(ScaleStringFromInt((scale + EPSILON) * 100.0f / davaGLWidget->devicePixelRatio()));
    emit ScaleChanged(scale);
}

void PreviewWidget::OnScaleByComboIndex(int index)
{
    DVASSERT(index >= 0);
    float scale = static_cast<float>(percentages.at(index));
    scrollAreaController->SetScale(scale / 100.0f * davaGLWidget->devicePixelRatio());
}

void PreviewWidget::OnScaleByComboText()
{
    float scale = GetScale();
    scrollAreaController->SetScale(scale / 100.0f * davaGLWidget->devicePixelRatio());
}

void PreviewWidget::OnGLWidgetResized(int width, int height, int dpr)
{
    scrollAreaController->SetViewSize(QSize(width * dpr, height * dpr));
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

bool PreviewWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == davaGLWidget->GetGLView())
    {
        switch(event->type())
        {
            case QEvent::Wheel:
                DVASSERT(nullptr != dynamic_cast<QWheelEvent*>(event));
                OnWheelEvent(static_cast<QWheelEvent*>(event));
                break;
            case QEvent::NativeGesture:
                DVASSERT(nullptr != dynamic_cast<QNativeGestureEvent*>(event));
                OnNativeGuestureEvent(static_cast<QNativeGestureEvent*>(event));
                break;
            case QEvent::MouseMove:
                DVASSERT(nullptr != dynamic_cast<QMouseEvent*>(event));
                OnMoveEvent(static_cast<QMouseEvent*>(event));
            case QEvent::MouseButtonPress:
                DVASSERT(nullptr != dynamic_cast<QMouseEvent*>(event));
                lastMousePos = static_cast<QMouseEvent*>(event)->pos(); 
            default:
                break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PreviewWidget::OnWheelEvent(QWheelEvent* event)
{
    static const qreal wheelDelta = 0.002;
#if defined Q_OS_MAC
    int horizontalScrollBarValue = horizontalScrollBar->value();
    horizontalScrollBarValue -= event->pixelDelta().x() * horizontalScrollBar->pageStep() * wheelDelta;
    horizontalScrollBar->setValue(horizontalScrollBarValue);
    
    int verticalScrollBarValue = verticalScrollBar->value();
    verticalScrollBarValue -= event->pixelDelta().y() * verticalScrollBar->pageStep() * wheelDelta;
    verticalScrollBar->setValue(verticalScrollBarValue);
#elif defined Q_OS_WIN
    qreal scale = scrollAreaController->GetScale();
    qreal eventValue = event->angleDelta().y() * wheelDelta;
    scale *= 1.0f + eventValue / 2.0f; //so funny divide random parameters with the random values
    QPoint pos = event->pos() * davaGLWidget->devicePixelRatio();
    scrollAreaController->AdjustScale(scale, pos);
#endif //Q_OS_MAC Q_OS_WIN
}

void PreviewWidget::OnNativeGuestureEvent(QNativeGestureEvent* event)
{
    const int normalScale = 1.0f;
    const int expandedScale = 1.4f;
    qreal scale = scrollAreaController->GetScale();
    QPoint pos = event->pos() * davaGLWidget->devicePixelRatio();
    switch(event->gestureType())
    {
        case Qt::ZoomNativeGesture:
            scrollAreaController->AdjustScale(scale + event->value() * davaGLWidget->devicePixelRatio(), pos);
                        break;
        case Qt::SmartZoomNativeGesture:
            scrollAreaController->AdjustScale((event->value() == 0 ? normalScale : expandedScale) * davaGLWidget->devicePixelRatio(), pos);
            //event->value() returns 1 or 0
            break;
        default:
            break;
    }
}

void PreviewWidget::OnMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton) 
    {
        QPoint delta(event->pos() - lastMousePos);
        delta *= davaGLWidget->devicePixelRatio();
        lastMousePos = event->pos();

        int horizontalScrollBarValue = horizontalScrollBar->value();
        horizontalScrollBarValue -= delta.x();
        horizontalScrollBar->setValue(horizontalScrollBarValue);

        int verticalScrollBarValue = verticalScrollBar->value();
        verticalScrollBarValue -= delta.y();
        verticalScrollBar->setValue(verticalScrollBarValue);
    }
}

void PreviewWidget::SetDPR(qreal arg)
{
    if (dpr != arg)
    {
        dpr = arg;
        DPRChanged(dpr);
    }
}
