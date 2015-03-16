#include "PreviewWidget.h"

#include "UI/Document.h"
#include "UI/PreviewContext.h"

#include "ui_PreviewWidget.h"
#include "EditScreen.h"

#include <QLineEdit>
#include <QScreen>


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
    , document(nullptr)
    , context(nullptr)
{
    ui->setupUi(this);

    ScopedPtr<UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(EDIT_SCREEN, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(EDIT_SCREEN);

    connect( ui->davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized );

    ui->horizontalRuler->hide();
    ui->verticalRuler->hide();

    // Setup the Scale Combo.
    int scalesCount = COUNT_OF(SCALE_PERCENTAGES);
    for (int i = 0; i < scalesCount; i ++)
    {
        ui->scaleCombo->addItem(QString(PERCENTAGE_FORMAT).arg(SCALE_PERCENTAGES[i]));
    }

    connect(ui->scaleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnScaleByComboIndex(int)));
    connect(ui->scaleCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(OnScaleByComboText()));

    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnVScrollbarMoved(int)));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnHScrollbarMoved(int)));

    ui->scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
    ui->scaleCombo->lineEdit()->setMaxLength(6);
    ui->scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    
    // Setup rulers.
    ui->horizontalRuler->SetOrientation(RulerWidget::Horizontal);
    ui->verticalRuler->SetOrientation(RulerWidget::Vertical);
    
    connect(ui->davaGLWidget->GetGLWindow(), &QWindow::screenChanged, this, &PreviewWidget::OnMonitorChanged);
}

PreviewWidget::~PreviewWidget()
{
    delete ui;
}

void PreviewWidget::SetDocument(Document *newDocument)
{
    if (document)
    {
        disconnect(context, SIGNAL(CanvasOrViewChanged(const QSize &, const QSize &)), this, SLOT(OnScrollAreaChanged(const QSize &, const QSize &)));
        disconnect(context, SIGNAL(CanvasPositionChanged(const QPoint &)), this, SLOT(OnScrollPositionChanged(const QPoint &)));
        disconnect(context, SIGNAL(CanvasScaleChanged(int)), this, SLOT(OnCanvasScaleChanged(int)));
        
        UIScreenManager::Instance()->GetScreen()->RemoveAllControls();
        context = NULL;
    }

    document = newDocument;

    if (document != nullptr)
    {
        context = document->GetPreviewContext();
        ui->davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        UIScreenManager::Instance()->GetScreen()->AddControl(context->GetViewControl());
        context->SetViewControlSize(GetGLViewSize());

        connect(context, SIGNAL(CanvasOrViewChanged(const QSize &, const QSize &)), this, SLOT(OnScrollAreaChanged(const QSize &, const QSize &)));
        connect(context, SIGNAL(CanvasPositionChanged(const QPoint &)), this, SLOT(OnScrollPositionChanged(const QPoint &)));
        connect(context, SIGNAL(CanvasScaleChanged(int)), this, SLOT(OnCanvasScaleChanged(int)));

        //Init
        OnScrollAreaChanged(context->GetViewSize(), context->GetScaledCanvasSize());
        OnScrollPositionChanged(context->GetCanvasPosition());
        OnCanvasScaleChanged(context->GetCanvasScale());
        OnMonitorChanged();
    }
}

DavaGLWidget* PreviewWidget::GetGLWidget() const
{
    return ui->davaGLWidget;
}

void PreviewWidget::OnMonitorChanged()
{
    if (context == nullptr)
        return;
    
    const auto index = ui->scaleCombo->currentIndex();
    ui->scaleCombo->setCurrentIndex(-1);
    ui->scaleCombo->setCurrentIndex(index);
}

void PreviewWidget::OnScaleByZoom(int scaleDelta)
{
    //int newScale = controlWorkspace->GetScreenScale() + scaleDelta;
    //controlWorkspace->SetScreenScale(newScale);
}

void PreviewWidget::OnScaleByComboIndex(int index)
{
    if (index < 0 || index >= static_cast<int>( COUNT_OF(SCALE_PERCENTAGES)) || !context)
    {
        return;
    }

    auto dpr = static_cast<int>( ui->davaGLWidget->GetGLWindow()->devicePixelRatio() );
    auto scaleValue = SCALE_PERCENTAGES[index] * dpr;
    context->SetCanvasControlScale(scaleValue);
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

    if (context)
    {
        context->SetViewControlSize(QSize(width * dpr, height * dpr));
    }
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    if (context)
    {
        QPoint canvasPosition = context->GetCanvasPosition();
        canvasPosition.setY(-vPosition);
        context->SetCanvasPosition(canvasPosition);
    }
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    if (context)
    {
        QPoint canvasPosition = context->GetCanvasPosition();
        canvasPosition.setX(-hPosition);
        context->SetCanvasPosition(canvasPosition);
    }
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

QSize PreviewWidget::GetGLViewSize() const
{
    return ui->davaGLWidget->size();
}
