#include "HeightDeltaTool.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>

#include "ui_HeightDeltaTool.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Project/ProjectManager.h"
#include "Commands2/PaintHeightDeltaAction.h"

#include "Tools/PathDescriptor/PathDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"


HeightDeltaTool::HeightDeltaTool( QWidget* p )
    : QWidget(p)
    , ui(new Ui::HeightDeltaTool())
    , outTemplate( "%1" )
{
    ui->setupUi(this);

    connect( ui->browse, SIGNAL(clicked()), SLOT(OnBrowse()));
    connect( ui->cancel, SIGNAL(clicked()), SLOT(close()));
    connect( ui->run, SIGNAL(clicked()), SLOT(OnRun()));
    connect( ui->suffix, SIGNAL(stateChanged(int)), SLOT(OnValueChanged()) );
    connect( ui->angle, SIGNAL(valueChanged(double)), SLOT(OnValueChanged()) );
    connect( ui->input, SIGNAL(textChanged( const QString& )), SLOT(OnValueChanged()) );

    const DAVA::FilePath defaultPath = ProjectManager::Instance()->CurProjectPath();
    SetDefaultDir(defaultPath.GetAbsolutePathname().c_str());

    OnValueChanged();
}

HeightDeltaTool::~HeightDeltaTool()
{
}

void HeightDeltaTool::SetDefaultDir( QString const& path )
{
    defaultDir = path;
}

void HeightDeltaTool::SetOutputTemplate( QString const& prefix, QString const& suffix )
{
    outTemplate = QString( "%1%2%3" ).arg(prefix).arg("%1").arg(suffix);
    OnValueChanged();
}

void HeightDeltaTool::OnBrowse()
{
    const QString path = QFileDialog::getOpenFileName( this, QString(), defaultDir, PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
    
    if ( path != NULL )
    {
        inPath = path;
        ui->input->setText(inPath);
        OnValueChanged();
    }
}

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
    const bool sourceExists = QFileInfo(inPath).exists();
    if ( !sourceExists )
    {
        QMessageBox::warning(this, "File doest not exists", QString("Input file could not be opened:\n\"%1\"").arg(inPath) );
        return ;
    }

    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    if (scene != NULL)
    {
        Landscape* landscapeRO = FindLandscape(scene);
        if (landscapeRO != NULL)
        {
            QtMainWindow::Instance()->WaitStart("Generating color height mask...", outPath);

            const DAVA::AABBox3& bbox = landscapeRO->GetBoundingBox();
            DAVA::Heightmap* heightmap = landscapeRO->GetHeightmap();
            
            if (heightmap != NULL)
            {
                const double unitSize = (bbox.max.x - bbox.min.x) / heightmap->Size();
                
                auto inputPathname = FilePath(inPath.toStdString());
                auto imInterface = DAVA::ImageSystem::Instance()->GetImageFormatInterface(inputPathname);
                DVASSERT(imInterface);
                auto imageSize = imInterface->GetImageSize(inputPathname);
                
                const double threshold = GetThresholdInMeters(unitSize);

                DAVA::Vector<DAVA::Color> colors;
                colors.resize(2);
                colors[0] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color0).AsColor();
                colors[1] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color1).AsColor();

                PaintHeightDeltaAction* action = new PaintHeightDeltaAction(
                        outPath.toStdString(),
                        (DAVA::float32)threshold,
                        heightmap,
                        imageSize.dx,
                        imageSize.dy,
                        bbox.max.z - bbox.min.z,
                        colors);
        
                action->Redo();
                DAVA::SafeDelete(action);
            }

            QtMainWindow::Instance()->WaitStop();

            if (heightmap != NULL)
            {
                QMessageBox::information(this, "Mask is ready", outPath);
            }
            else
            {
                QMessageBox::warning(this, "An error occured", "Please check if landscape has proper setup.");
            }
        }
    }
}

void HeightDeltaTool::OnValueChanged()
{
    inPath = ui->input->text();
    if ( inPath.isEmpty() )
    {
        return ;
    }

    const QString name = QFileInfo(inPath).completeBaseName();
    const QString ext = QFileInfo(inPath).suffix();
    const QString outNameTemplate = QString( outTemplate ).arg(name);
    QString angle;

    if (ui->suffix->isChecked())
    {
        angle = "_" + QString::number( ui->angle->value() ).replace( '.', '-' ).replace( ',', '-' );
    }

    outName = QString("%1%2.%3").arg(outNameTemplate).arg(angle).arg(ext);
    outPath = QString("%1/%2").arg(QFileInfo(inPath).absoluteDir().absolutePath()).arg(outName);
        
    // ui->input->setText(inPath);
    ui->output->setText(outName);
}
