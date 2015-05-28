#include "GrassEditorPanel.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Tools/SliderWidget/SliderWidget.h"
#include "Constants.h"

#define LAYERS_ROW_HEIGHT 32
#define LAYERS_PREVIEW_WIDTH 48
#define LAYERS_CHECKBOX_WIDTH 24

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QHeaderView>
#include <QPainter>
#include <QFileDialog>


GrassEditorPanel::GrassEditorPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
{
	InitUI();
	ConnectToSignals();
}

GrassEditorPanel::~GrassEditorPanel()
{
}

bool GrassEditorPanel::GetEditorEnabled()
{
	return GetActiveScene()->grassEditorSystem->IsLandscapeEditingEnabled();
}

void GrassEditorPanel::OnEditorEnabled()
{
}

void GrassEditorPanel::SetWidgetsState(bool enabled)
{
}

void GrassEditorPanel::BlockAllSignals(bool block)
{
}

void GrassEditorPanel::InitUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // sliders
    QGroupBox *brushBox = new QGroupBox("Brush", this);
    QGridLayout *brushLayout = new QGridLayout(brushBox);

    grassHeight = new QSlider(Qt::Horizontal, brushBox);
    grassHeight->setRange(0, 15);
    grassHeight->setSingleStep(1);
    grassHeight->setPageStep(4);
    grassHeight->setTickPosition(QSlider::TicksBothSides);
    grassHeight->setMaximumHeight(21);

    grassHeightAffect = new QToolButton(brushBox);
    grassHeightAffect->setCheckable(true);
    grassHeightAffect->setIcon(QIcon(":/QtIcons/attach.png"));

    grassHeightAdd = new QToolButton(brushBox);
    grassHeightAdd->setCheckable(true);
    grassHeightAdd->setIcon(QIcon(":/QtIcons/pencil_add.png"));

    grassDensityAffect = new QToolButton(brushBox);
    grassDensityAffect->setCheckable(true);
    grassDensityAffect->setIcon(QIcon(":/QtIcons/attach.png"));

    grassDensityAdd = new QToolButton(brushBox);
    grassDensityAdd->setCheckable(true);
    grassDensityAdd->setIcon(QIcon(":/QtIcons/pencil_add.png"));

    grassDensity = new QSlider(Qt::Horizontal, brushBox);
    grassDensity->setRange(0, 15);
    grassDensity->setSingleStep(1);
    grassDensity->setPageStep(4);
    grassDensity->setTickPosition(QSlider::TicksBothSides);
    grassDensity->setMaximumHeight(21);

    brushLayout->addWidget(new QLabel("Height:"), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    brushLayout->addWidget(grassHeight, 0, 1);
    brushLayout->addWidget(grassHeightAffect, 0, 2);
    brushLayout->addWidget(grassHeightAdd, 0, 3);

    brushLayout->addWidget(new QLabel("Density:"), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
    brushLayout->addWidget(grassDensity, 1, 1);
    brushLayout->addWidget(grassDensityAffect, 1, 2);
    brushLayout->addWidget(grassDensityAdd, 1, 3);

    layout->addWidget(brushBox);

    // layers
    layersList = new QTableWidget(GRASS_EDITOR_LAYERS_COUNT, 3, this);
    //layersList->setShowGrid(false);
    layersList->horizontalHeader()->setVisible(false);
    layersList->horizontalHeader()->resizeSection(0, LAYERS_CHECKBOX_WIDTH);
    layersList->horizontalHeader()->resizeSection(1, LAYERS_PREVIEW_WIDTH);
    layersList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    layersList->verticalHeader()->setVisible(false);
    layersList->setSelectionBehavior(QAbstractItemView::SelectRows);
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layersList->setFocusPolicy(Qt::NoFocus);
    layersList->setIconSize(QSize(LAYERS_ROW_HEIGHT - 2, LAYERS_ROW_HEIGHT - 2));
    layout->addWidget(layersList);

    QString layerCBStyle = "QCheckBox::indicator { width: 16px; height: 16px; } QCheckBox::indicator:checked {image: url(:/QtIcons/layer_visible.png); } QCheckBox::indicator:unchecked {image: url(:/QtIcons/layer_invisible.png); }";

    for(int row = 0; row < GRASS_EDITOR_LAYERS_COUNT; ++row)
    {
        QString layerName;
        layerName.sprintf("Layer %d", row);
        
        QTableWidgetItem *layerVisibilityItem = new QTableWidgetItem();
        layerVisibilityItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerVisibilityItem->setBackground(qApp->palette().button());

        QTableWidgetItem *layerIconItem = new QTableWidgetItem();
        layerIconItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerIconItem->setBackground(qApp->palette().button());

        QTableWidgetItem *layerNameItem = new QTableWidgetItem(layerName);
        layerNameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerNameItem->setBackground(qApp->palette().button());

        QHBoxLayout *wrapperLayout = new QHBoxLayout();
        wrapperLayout->setMargin((LAYERS_CHECKBOX_WIDTH - 16) / 2);
        wrapperLayout->setSpacing(0);

        QFrame *layerCheckBoxWrapper = new QFrame();
        layerCheckBoxWrapper->setLayout(wrapperLayout);
        layerCheckBoxWrapper->setAutoFillBackground(true);
        layerCheckBoxWrapper->setFrameStyle(QFrame::Raised);
        layerCheckBoxWrapper->setFrameShape(QFrame::Panel);
        layerCheckBoxWrapper->setStyleSheet("background-color: palette(button)");

        QCheckBox * layerCheckBox = new QCheckBox(layerCheckBoxWrapper);
        layerCheckBox->setStyleSheet(layerCBStyle);
        wrapperLayout->addWidget(layerCheckBox, 0, Qt::AlignCenter);

        layersList->setItem(row, 0, layerVisibilityItem);
        layersList->setItem(row, 1, layerIconItem);
        layersList->setItem(row, 2, layerNameItem);
        layersList->setCellWidget(row, 0, layerCheckBoxWrapper);
        layersList->setRowHeight(row, LAYERS_ROW_HEIGHT);
        layersList->setStyleSheet("QTableWidget::item:selected{ background-color: palette(highlight) }");

        layerCheckBoxes[row] = layerCheckBox;
        layerCheckBoxes[row]->setProperty("#layer_index", row);
    }

    // lod preview
    //lodPreview = new DistanceSlider(this);
    //layout->addWidget(lodPreview);
}

void GrassEditorPanel::ConnectToSignals()
{ 
    QObject::connect(layersList, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(OnLayerSelected(int, int, int, int)));
    QObject::connect(grassHeight, SIGNAL(valueChanged(int)), this, SLOT(OnHeightChanged(int)));
    QObject::connect(grassDensity, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
    QObject::connect(grassHeightAffect, SIGNAL(toggled(bool)), this, SLOT(OnHightAffectToggled(bool)));
    QObject::connect(grassDensityAffect, SIGNAL(toggled(bool)), this, SLOT(OnDensityAffectToggled(bool)));
    QObject::connect(grassHeightAdd, SIGNAL(toggled(bool)), this, SLOT(OnHightAddToggled(bool)));
    QObject::connect(grassDensityAdd, SIGNAL(toggled(bool)), this, SLOT(OnDensityAddToggled(bool)));

    for(int row = 0; row < GRASS_EDITOR_LAYERS_COUNT; ++row)
    {
        QObject::connect(layerCheckBoxes[row], SIGNAL(stateChanged(int)), this, SLOT(OnLayerChecked(int)));
    }
}

void GrassEditorPanel::StoreState()
{ }

void GrassEditorPanel::RestoreState()
{ 
    SceneEditor2* sceneEditor = GetActiveScene();
    
    if(NULL != sceneEditor)
    {
        DAVA::VegetationRenderObject *vegetationRObj = sceneEditor->grassEditorSystem->GetCurrentVegetationObject();
        if(NULL != vegetationRObj)
        {
            QPixmap txpix;

            DAVA::FilePath texturePath = vegetationRObj->GetVegetationTexture();
            texturePath.ReplaceExtension(".png");

            if(texturePath.Exists())
            {
                 txpix = QPixmap(texturePath.GetAbsolutePathname().c_str());
            }

            for(int i = 0; i < GRASS_EDITOR_LAYERS_COUNT; ++i)
            {
                QTableWidgetItem *item = layersList->item(i, 1);

                // fill visible/invisible state for layer
                if(sceneEditor->grassEditorSystem->IsLayerVisible(i))
                {
                    layerCheckBoxes[i]->setCheckState(Qt::Checked);
                }
                else
                {
                    layerCheckBoxes[i]->setCheckState(Qt::Unchecked);
                }

                // fill texture preview for layer
                bool ok = false;
                if(!txpix.isNull())
                {
                    if(NULL != item)
                    {
#if RHI_COMPLETE_EDITOR
                        DAVA::Rect2i r = MapTexCoord(sheet.cells[i], txpix.width(), txpix.height());

                        QPixmap pix(24, 24);
                        QPainter painter(&pix);
                        painter.setPen(QColor(0, 0, 0));
                        painter.setBrush(QBrush(QColor(255, 255, 255)));
                        painter.fillRect(0, 0, pix.width() - 1, pix.height() - 1, Qt::SolidPattern);
                        painter.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
                        painter.drawPixmap(1, 1, pix.width() - 2, pix.height() - 2, txpix, r.x, r.y, r.dx, r.dy);

                        item->setIcon(QIcon(pix));
#endif

                        ok = true;
                    }
                }

                if(!ok)
                {
                    item->setIcon(QIcon());
                }
            }

            int curHeight = sceneEditor->grassEditorSystem->GetBrushHeight();
            int curDensity = sceneEditor->grassEditorSystem->GetBrushDensity();
            int curLayer = sceneEditor->grassEditorSystem->GetCurrentLayer();
            int curMode = sceneEditor->grassEditorSystem->GetBrushMode();
            int curAffect = sceneEditor->grassEditorSystem->GetBrushAffect();

            layersList->setCurrentItem(layersList->item(curLayer, 0));
            grassHeight->setValue(curHeight);
            grassDensity->setValue(curDensity);

            grassHeightAffect->setChecked((bool) (curAffect & GrassEditorSystem::AFFECT_HEIGHT));
            grassHeightAdd->setChecked((bool) (curMode & GrassEditorSystem::BRUSH_ADD_HEIGHT));
            grassDensityAffect->setChecked((bool) (curAffect & GrassEditorSystem::AFFECT_DENSITY));
            grassDensityAdd->setChecked((bool) (curMode & GrassEditorSystem::BRUSH_ADD_DENSITY));
        }
    }
}

void GrassEditorPanel::ConnectToShortcuts()
{ }

void GrassEditorPanel::DisconnectFromShortcuts()
{ }

void GrassEditorPanel::OnLayerSelected(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetCurrentLayer(currentRow);
    }
}

void GrassEditorPanel::OnLayerChecked(int state)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        QVariant index = QObject::sender()->property("#layer_index");
        if(index.isValid())
        {
            int layer = index.toInt();
            sceneEditor->grassEditorSystem->SetLayerVisible(layer, state == Qt::Checked);
        }
    }
}

void GrassEditorPanel::OnHeightChanged(int value)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetBrushHeight(value);
    }
}

void GrassEditorPanel::OnDensityChanged(int value)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetBrushDensity(value);
    }
}

void GrassEditorPanel::OnDensityAffectToggled(bool checked)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        int cur = sceneEditor->grassEditorSystem->GetBrushAffect();
        if(checked)
        {
            cur |= GrassEditorSystem::AFFECT_DENSITY;
        }
        else
        {
            cur &= (~GrassEditorSystem::AFFECT_DENSITY);
        }

        sceneEditor->grassEditorSystem->SetBrushAffect(cur);
    }
}

void GrassEditorPanel::OnDensityAddToggled(bool checked)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        int cur = sceneEditor->grassEditorSystem->GetBrushMode();
        if(checked)
        {
            cur |= GrassEditorSystem::BRUSH_ADD_DENSITY;
        }
        else
        {
            cur &= (~GrassEditorSystem::BRUSH_ADD_DENSITY);
        }

        sceneEditor->grassEditorSystem->SetBrushMode(cur);
    }
}

void GrassEditorPanel::OnHightAffectToggled(bool checked)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        int cur = sceneEditor->grassEditorSystem->GetBrushAffect();
        if(checked)
        {
            cur |= GrassEditorSystem::AFFECT_HEIGHT;
        }
        else
        {
            cur &= (~GrassEditorSystem::AFFECT_HEIGHT);
        }

        sceneEditor->grassEditorSystem->SetBrushAffect(cur);
    }
}

void GrassEditorPanel::OnHightAddToggled(bool checked)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        int cur = sceneEditor->grassEditorSystem->GetBrushMode();
        if(checked)
        {
            cur |= GrassEditorSystem::BRUSH_ADD_HEIGHT;
        }
        else
        {
            cur &= (~GrassEditorSystem::BRUSH_ADD_HEIGHT);
        }

        sceneEditor->grassEditorSystem->SetBrushMode(cur);
    }
}
#if RHI_COMPLETE_EDITOR
DAVA::Rect2i GrassEditorPanel::MapTexCoord(const DAVA::TextureSheetCell &cell, DAVA::uint32 w, DAVA::uint32 h) const
{
    DAVA::AABBox2 area;

    for(int i = 0; i < 4; ++i)
    {
        DAVA::Vector2 point;
        DAVA::Vector2 cellCoords = cell.coords[i];

        point.x = cellCoords.x * w;
        point.y = cellCoords.y * h;

        area.AddPoint(point);
    }

    return GrassEditorSystem::GetAffectedImageRect(area);
}
#endif