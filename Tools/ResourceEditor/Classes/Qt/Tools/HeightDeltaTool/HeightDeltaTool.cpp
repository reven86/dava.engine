#include "HeightDeltaTool.h"

#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>

#include "ui_HeightDeltaTool.h"

#include "Qt/Main/mainwindow.h"
#include "Tools/HeightDeltaTool/PaintHeightDelta.h"
#include "Tools/PathDescriptor/PathDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"

#include "QtTools/FileDialogs/FileDialog.h"

HeightDeltaTool::HeightDeltaTool(QWidget* p)
    : QWidget(p)
    , ui(new Ui::HeightDeltaTool())
{
    ui->setupUi(this);

    connect(ui->cancel, &QAbstractButton::clicked, this, &HeightDeltaTool::close);
    connect(ui->run, &QAbstractButton::clicked, this, &HeightDeltaTool::OnRun);
    connect(ui->angle, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &HeightDeltaTool::OnValueChanged);

    OnValueChanged();
}

HeightDeltaTool::~HeightDeltaTool() = default;

double HeightDeltaTool::GetThresholdInMeters(double unitSize)
{
    double angle = ui->angle->value();
    DAVA::float32 radAngle = DAVA::DegToRad((DAVA::float32)angle);

    double tangens = tan(radAngle);

    double delta = DAVA::Abs(unitSize * (DAVA::float32)tangens);
    return delta;
}

void HeightDeltaTool::OnRun()
{
    DVASSERT(!outputFilePath.isEmpty());

    SceneEditor2* scene = sceneholder.GetScene();
    DVASSERT(scene);
    DAVA::Landscape* landscapeRO = FindLandscape(scene);
    DVASSERT(landscapeRO);

    const DAVA::AABBox3& bbox = landscapeRO->GetBoundingBox();
    DAVA::Heightmap* heightmap = landscapeRO->GetHeightmap();
    DVASSERT(heightmap);

    DAVA::int32 heightmapSize = heightmap->Size();
    const double unitSize = (bbox.max.x - bbox.min.x) / heightmapSize;

    const double threshold = GetThresholdInMeters(unitSize);

    DAVA::Vector<DAVA::Color> colors;
    colors.resize(2);
    colors[0] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color0).AsColor();
    colors[1] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color1).AsColor();

    PaintHeightDelta::Execute(outputFilePath.toStdString(), (DAVA::float32)threshold, heightmap,
                              heightmapSize, heightmapSize, bbox.max.z - bbox.min.z, colors);
    QMessageBox::information(this, "Mask is ready", outputFilePath);

    QWidget::close();
}

void HeightDeltaTool::OnValueChanged(double /*v*/)
{
    ui->run->setEnabled(false);

    SceneEditor2* scene = sceneholder.GetScene();
    DVASSERT(scene != nullptr);
    DAVA::Landscape* landscape = FindLandscape(scene);
    if (landscape == nullptr)
    {
        ui->outputPath->setText(tr("Landscape not found"));
        return;
    }

    if (landscape->GetHeightmap() == nullptr)
    {
        ui->outputPath->setText(tr("Heightmap was not assigned"));
        return;
    }

    ui->run->setEnabled(true);

    DAVA::FilePath heightMapPath = landscape->GetHeightmapPathname();
    heightMapPath.ReplaceExtension("");

    QString angle = QString::number(ui->angle->value()).replace('.', '-').replace(',', '-');

    outputFilePath = QString("%1_delta_%2.png").arg(heightMapPath.GetAbsolutePathname().c_str()).arg(angle);
    ui->outputPath->setText(QFileInfo(outputFilePath).fileName());
}
