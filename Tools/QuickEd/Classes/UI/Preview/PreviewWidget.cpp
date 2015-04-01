#include "PreviewWidget.h"

#include "PreviewModel.h"

#include "ui_PreviewWidget.h"
#include "EditScreen.h"
#include "UI/WidgetContext.h"

#include <QLineEdit>
#include <QScreen>

#include "Model/PackageHierarchy/ControlNode.h"

#include <QMessageBox>

using namespace DAVA;

static const int SCALE_PERCENTAGES[] =
{
    10,  25,  50,  75,
	100, 125, 150, 175,
    200, 250, 400, 800,
};

static const int32 DEFAULT_SCALE_PERCENTAGE_INDEX = 4; // 100%
static const char* PERCENTAGE_FORMAT = "%1 %";

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreviewWidget())
    , widgetContext(nullptr)
    , model(new PreviewModel(this))
{
    ui->setupUi(this);

    ScopedPtr<UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(EDIT_SCREEN, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(EDIT_SCREEN);
    connect( ui->davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized );

    // Setup the Scale Combo.
    int scalesCount = COUNT_OF(SCALE_PERCENTAGES);
    for (int i = 0; i < scalesCount; i ++)
    {
        ui->scaleCombo->addItem(QString(PERCENTAGE_FORMAT).arg(SCALE_PERCENTAGES[i]));
    }

    connect(ui->scaleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(ui->scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(ui->verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    ui->scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
    ui->scaleCombo->lineEdit()->setMaxLength(6);
    ui->scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    
   
    connect(ui->davaGLWidget->GetGLWindow(), &QWindow::screenChanged, this, &PreviewWidget::OnMonitorChanged);

    connect(model, &PreviewModel::CanvasOrViewChanged, this, &PreviewWidget::OnScrollAreaChanged);
    connect(model, &PreviewModel::CanvasPositionChanged, this, &PreviewWidget::OnScrollPositionChanged);
    connect(model, &PreviewModel::CanvasScaleChanged, this, &PreviewWidget::OnCanvasScaleChanged);

    connect(model, &PreviewModel::ControlNodeSelected, this, &PreviewWidget::OnControlNodeSelected);
    connect(model, &PreviewModel::AllControlsDeselected, this, &PreviewWidget::OnAllControlsDeselected);
    connect(model, &PreviewModel::ErrorOccurred, this, &PreviewWidget::OnError);
}

PreviewWidget::~PreviewWidget()
{
    delete ui;
}

void PreviewWidget::OnContextChanged(WidgetContext *context)
{
    if (widgetContext == context)
    {
        return;
    }
    UIScreenManager::Instance()->GetScreen()->RemoveAllControls();
    widgetContext = context;

    UpdateRootControls();
    if (nullptr != widgetContext)
    {
        ui->davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        UIScreenManager::Instance()->GetScreen()->AddControl(model->GetViewControl());
        model->SetViewControlSize(ui->davaGLWidget->size());

        OnScrollAreaChanged(model->GetViewSize(), model->GetScaledCanvasSize());
        OnScrollPositionChanged(model->GetCanvasPosition());
        OnCanvasScaleChanged(model->GetCanvasScale());
        OnMonitorChanged();
    }
}

void PreviewWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "activeRootControls")
    {
        UpdateRootControls();
    }
    if (role == "deactivatedControls")
    {
        model->ControlsDeactivated(widgetContext->GetData("deactivatedControls").value<QList<ControlNode*> >());
    }
    if (role == "activatedControls")
    {
        model->ControlsActivated(widgetContext->GetData("activatedControls").value<QList<ControlNode*> >());
    }
}

void PreviewWidget::UpdateRootControls()
{
    if (nullptr == widgetContext)
    {
        model->SetActiveRootControls(QList<ControlNode*>());
    }
    else
    {
        model->SetActiveRootControls(widgetContext->GetData("activeRootControls").value<QList<ControlNode*> >());
    }
    
}


DavaGLWidget* PreviewWidget::GetGLWidget() const
{
    return ui->davaGLWidget;
}

void PreviewWidget::OnMonitorChanged()
{
    const auto index = ui->scaleCombo->currentIndex();
    ui->scaleCombo->setCurrentIndex(-1);
    ui->scaleCombo->setCurrentIndex(index);
}

void PreviewWidget::OnControlNodeSelected(ControlNode *node)
{
    widgetContext->SetData(QVariant::fromValue(node), "selectedNode");
}

void PreviewWidget::OnAllControlsDeselected()
{
    widgetContext->SetData(!widgetContext->GetData("controlDeselected").toBool(), "controlDeselected");
}

void PreviewWidget::OnError(QString errorText)
{
    QMessageBox::warning(qApp->activeWindow(), tr("Error occurred!"), errorText);
}

void PreviewWidget::OnScaleByZoom(int scaleDelta)
{
    //int newScale = controlWorkspace->GetScreenScale() + scaleDelta;
    //controlWorkspace->SetScreenScale(newScale);
}

void PreviewWidget::OnScaleByComboIndex(int index)
{   
    if (index < 0 || index >= static_cast<int>( COUNT_OF(SCALE_PERCENTAGES)))
    {
        return;
    }

    auto dpr = static_cast<int>( ui->davaGLWidget->GetGLWindow()->devicePixelRatio() );
    auto scaleValue = SCALE_PERCENTAGES[index] * dpr;
    model->SetCanvasControlScale(scaleValue);
}

void PreviewWidget::OnScaleByComboText()
{
	// Firstly verify whether the value is already set.
	QString curTextValue = ui->scaleCombo->currentText().trimmed();
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
    
	//controlWorkspace->SetScreenScale(scaleValue);
}

void PreviewWidget::OnZoomInRequested()
{
	OnScaleByZoom(10);
}

void PreviewWidget::OnZoomOutRequested()
{
	OnScaleByZoom(-10);
}

void PreviewWidget::OnCanvasScaleChanged(int newScale)
{
    auto dpr = static_cast<int>( ui->davaGLWidget->GetGLWindow()->devicePixelRatio() );
    newScale /= dpr;

    QString newScaleText = QString(PERCENTAGE_FORMAT).arg(newScale);
    if (ui->scaleCombo->currentText() != newScaleText )
    {
        ui->scaleCombo->lineEdit()->blockSignals(true);
        ui->scaleCombo->setEditText(newScaleText);
        ui->scaleCombo->lineEdit()->blockSignals(false);
    }
}

void PreviewWidget::OnGLWidgetResized(int width, int height, int dpr)
{
    Vector2 screenSize((float32)width * dpr, (float32)height * dpr);

    UIScreenManager::Instance()->GetScreen()->SetSize(screenSize);

    model->SetViewControlSize(QSize(width * dpr, height * dpr));
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    QPoint canvasPosition = model->GetCanvasPosition();
    canvasPosition.setY(-vPosition);
    model->SetCanvasPosition(canvasPosition);
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    QPoint canvasPosition = model->GetCanvasPosition();
    canvasPosition.setX(-hPosition);
    model->SetCanvasPosition(canvasPosition);
}

void PreviewWidget::OnScrollPositionChanged(const QPoint &newPosition)
{
    QPoint scrollbarPosition(-newPosition.x(), -newPosition.y());

    if (ui->verticalScrollBar->sliderPosition() != scrollbarPosition.y())
    {
        ui->verticalScrollBar->blockSignals(true);
        ui->verticalScrollBar->setSliderPosition(scrollbarPosition.y());
        ui->verticalScrollBar->blockSignals(false);
    }

    if (ui->horizontalScrollBar->sliderPosition() != scrollbarPosition.x())
    {
        ui->horizontalScrollBar->blockSignals(true);
        ui->horizontalScrollBar->setSliderPosition(scrollbarPosition.x());
        ui->horizontalScrollBar->blockSignals(false);
    }
}

void PreviewWidget::OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize)
{
    QSize maximumPosition(contentSize - viewSize);

    //ui->verticalScrollBar->setEnabled(maximumPosition.height() > 0);
    //ui->horizontalScrollBar->setEnabled(maximumPosition.width() > 0);

    if (maximumPosition.height() < 0)
    {
        maximumPosition.setHeight(0);
    }

    if (maximumPosition.width() < 0)
    {
        maximumPosition.setWidth(0);
    }

    if (ui->verticalScrollBar->maximum() != maximumPosition.height())
    {
        ui->verticalScrollBar->blockSignals(true);
        ui->verticalScrollBar->setMinimum(0);
        ui->verticalScrollBar->setMaximum(maximumPosition.height());
        ui->verticalScrollBar->setPageStep(viewSize.height());
        ui->verticalScrollBar->blockSignals(false);
    }

    if (ui->horizontalScrollBar->maximum() != maximumPosition.width())
    {
        ui->horizontalScrollBar->blockSignals(true);
        ui->horizontalScrollBar->setMinimum(0);
        ui->horizontalScrollBar->setMaximum(maximumPosition.width());
        ui->horizontalScrollBar->setPageStep(viewSize.width());
        ui->horizontalScrollBar->blockSignals(false);
    }
}