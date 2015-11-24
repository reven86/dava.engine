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


#include "HeightDeltaTool.h"

#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>

#include "ui_HeightDeltaTool.h"

#include "Qt/Main/mainwindow.h"
#include "Project/ProjectManager.h"
#include "Commands2/PaintHeightDeltaAction.h"

#include "Tools/PathDescriptor/PathDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"

#include "QtTools/FileDialog/FileDialog.h"


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

    const DAVA::FilePath defaultPath = ProjectManager::Instance()->GetProjectPath();
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
    const QString path = FileDialog::getOpenFileName( this, QString(), defaultDir, PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
    
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
            const DAVA::AABBox3& bbox = landscapeRO->GetBoundingBox();
            DAVA::Heightmap* heightmap = landscapeRO->GetHeightmap();
            
            if (heightmap != NULL)
            {
                const double unitSize = (bbox.max.x - bbox.min.x) / heightmap->Size();
                
                auto inputPathname = FilePath(inPath.toStdString());
                auto imInterface = DAVA::ImageSystem::Instance()->GetImageFormatInterface(inputPathname);
                DVASSERT(imInterface);
                auto imageInfo = imInterface->GetImageInfo(inputPathname);
                
                const double threshold = GetThresholdInMeters(unitSize);

                DAVA::Vector<DAVA::Color> colors;
                colors.resize(2);
                colors[0] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color0).AsColor();
                colors[1] = SettingsManager::GetValue(Settings::General_HeighMaskTool_Color1).AsColor();

                PaintHeightDeltaAction* action = new PaintHeightDeltaAction(
                        outPath.toStdString(),
                        (DAVA::float32)threshold,
                        heightmap,
                        imageInfo.width,
                        imageInfo.height,
                        bbox.max.z - bbox.min.z,
                        colors);
        
                action->Redo();
                DAVA::SafeDelete(action);
            }

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
