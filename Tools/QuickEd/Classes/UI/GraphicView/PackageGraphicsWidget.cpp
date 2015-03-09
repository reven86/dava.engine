//
//  PackageGraphicsWidget.cpp
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#include "PackageGraphicsWidget.h"
#include "ui_PackageGraphicsWidget.h"
#include <QLineEdit>
#include "EditScreen.h"
#include "UI/PackageDocument.h"
#include "UI/GraphicView/GraphicsViewContext.h"


static const int SCALE_PERCENTAGES[] =
{
    10,  25,  50,  75,
	100, 125, 150, 175,
    200, 250, 400, 800,
};

static const int32 DEFAULT_SCALE_PERCENTAGE_INDEX = 4; // 100%
static const char* PERCENTAGE_FORMAT = "%1 %";

PackageGraphicsWidget::PackageGraphicsWidget(QWidget *parent)
: QWidget(parent)
, ui(new Ui::PackageGraphicsWidget())
, document(NULL)
, context(NULL)
{
    ui->setupUi(this);

    ScopedPtr<UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(EDIT_SCREEN, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(EDIT_SCREEN);

    ui->davaGLWidget->setMaximumSize(1, 1);
    ui->davaGLWidget->setFocusPolicy(Qt::StrongFocus);
    ui->davaGLWidget->installEventFilter(this);
    connect(ui->davaGLWidget, SIGNAL(resized()), this, SLOT(OnGLWidgetResized()));

    ui->horizontalRuler->hide();
    ui->verticalRuler->hide();

    // Setup the Scale Combo.
    int scalesCount = COUNT_OF(SCALE_PERCENTAGES);
    for (int i = 0; i < scalesCount; i ++)
    {
        ui->scaleCombo->addItem(QString(PERCENTAGE_FORMAT).arg(SCALE_PERCENTAGES[i]));
    }

    ui->scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
    ui->scaleCombo->lineEdit()->setMaxLength(6);
    ui->scaleCombo->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->scaleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnScaleByComboIndex(int)));
    connect(ui->scaleCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(OnScaleByComboText()));

    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnVScrollbarMoved(int)));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnHScrollbarMoved(int)));

    // Setup rulers.
    ui->horizontalRuler->SetOrientation(RulerWidget::Horizontal);
    ui->verticalRuler->SetOrientation(RulerWidget::Vertical);
}

PackageGraphicsWidget::~PackageGraphicsWidget()
{
    delete ui;
}

void PackageGraphicsWidget::SetDocument(PackageDocument *newDocument)
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

    if (!document)
    {
        ui->davaGLWidget->setMaximumSize(1, 1);
    }
    else
    {
        context = document->GetGraphicsContext();
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
    }
}

void PackageGraphicsWidget::OnScaleByZoom(int scaleDelta)
{
    //int newScale = controlWorkspace->GetScreenScale() + scaleDelta;
    //controlWorkspace->SetScreenScale(newScale);
}

void PackageGraphicsWidget::OnScaleByComboIndex(int index)
{
    if (index < 0 || index >= (int)COUNT_OF(SCALE_PERCENTAGES) || !context)
    {
        return;
    }

    int scaleValue = SCALE_PERCENTAGES[index];

    context->SetCanvasControlScale(scaleValue);
}

void PackageGraphicsWidget::OnScaleByComboText()
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

void PackageGraphicsWidget::OnZoomInRequested()
{
	OnScaleByZoom(10);
}

void PackageGraphicsWidget::OnZoomOutRequested()
{
	OnScaleByZoom(-10);
}

void PackageGraphicsWidget::OnCanvasScaleChanged(int newScale)
{
    QString newScaleText = QString(PERCENTAGE_FORMAT).arg(newScale);
    if (ui->scaleCombo->currentText() != newScaleText )
    {
        ui->scaleCombo->lineEdit()->blockSignals(true);
        ui->scaleCombo->setEditText(newScaleText);
        ui->scaleCombo->lineEdit()->blockSignals(false);
    }
}

void PackageGraphicsWidget::OnGLWidgetResized()
{
    const QSize &viewSize = GetGLViewSize();
    Vector2 screenSize((float32)viewSize.width(), (float32)viewSize.height());

    UIScreenManager::Instance()->GetScreen()->SetSize(screenSize);

    if (context)
        context->SetViewControlSize(viewSize);
}

void PackageGraphicsWidget::OnVScrollbarMoved(int vPosition)
{
    if (context)
    {
        QPoint canvasPosition = context->GetCanvasPosition();
        canvasPosition.setY(-vPosition);
        context->SetCanvasPosition(canvasPosition);
    }
}

void PackageGraphicsWidget::OnHScrollbarMoved(int hPosition)
{
    if (context)
    {
        QPoint canvasPosition = context->GetCanvasPosition();
        canvasPosition.setX(-hPosition);
        context->SetCanvasPosition(canvasPosition);
    }
}

void PackageGraphicsWidget::OnScrollPositionChanged(const QPoint &newPosition)
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

void PackageGraphicsWidget::OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize)
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

QSize PackageGraphicsWidget::GetGLViewSize() const
{
    return ui->davaGLWidget->size();
}
