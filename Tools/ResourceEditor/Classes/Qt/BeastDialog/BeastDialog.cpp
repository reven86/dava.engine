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


#include "BeastDialog.h"

#include <QEventLoop>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

#include "QtTools/FileDialog/FileDialog.h"

#include "Qt/Scene/SceneEditor2.h"


namespace
{
    const FastName settingsDefaultPath("Internal/Beast/LightmapsDefaultDir");
}

BeastDialog::BeastDialog(QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::BeastDialog())
    , scene(NULL)
    , result(false)
    , beastMode(BeastProxy::MODE_LIGHTMAPS)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(ui->start, SIGNAL( clicked() ), SLOT( OnStart() ));
    connect(ui->cancel, SIGNAL( clicked() ), SLOT( OnCancel() ));
    connect(ui->browse, SIGNAL( clicked() ), SLOT( OnBrowse() ));
    connect(ui->output, SIGNAL( textChanged(const QString&) ), SLOT( OnTextChanged() ));
    connect(ui->lightmapBeastModeButton, SIGNAL( clicked( bool ) ), SLOT( OnLightmapMode( bool ) ));
    connect(ui->shBeastModeButton, SIGNAL( clicked( bool ) ), SLOT( OnSHMode( bool ) ));
    connect(ui->previewButton, SIGNAL( clicked( bool ) ), SLOT( OnPreviewMode( bool ) ));
}

BeastDialog::~BeastDialog()
{
}

void BeastDialog::SetScene(SceneEditor2 *_scene)
{
    scene = _scene;
}

bool BeastDialog::Exec(QWidget *parent)
{
    DVASSERT(scene);

    ui->scenePath->setText(QDir::toNativeSeparators( GetDefaultPath() ));
    
    const String defaultPath = SettingsManager::Instance()->GetValue(settingsDefaultPath).AsString();
    ui->output->setText(QDir::toNativeSeparators(QString("%1").arg(defaultPath.c_str())));

    show();

    loop = new QEventLoop( this );
    loop->exec();

    hide();
    loop->deleteLater();

    return result;
}

void BeastDialog::OnStart()
{
    const QDir dir( ui->scenePath->text() );
    dir.mkpath( ui->output->text() );
    if ( !dir.exists( ui->output->text() ) )
    {
        result = false;
        QMessageBox::warning( this, QString(), "Specified path is invalid. Couldn't create output directory." );
        return;
    }

    SettingsManager::Instance()->SetValue(settingsDefaultPath,VariantType(QDir::toNativeSeparators(ui->output->text()).toStdString()));

    result = true;
    loop->quit();
}

void BeastDialog::OnCancel()
{
    result = false;
    loop->quit();
}

void BeastDialog::OnBrowse()
{
    const QString path = FileDialog::getExistingDirectory( this, QString(), GetPath(), FileDialog::ShowDirsOnly );
    if ( !path.isEmpty() )
    {
        SetPath( path );
    }
}

void BeastDialog::OnTextChanged()
{
    const QString text = QString("Output path: %1").arg(QDir::toNativeSeparators(GetPath()));
    ui->output->setToolTip( text );
}

void BeastDialog::OnLightmapMode(bool checked)
{
    if(checked)
    {
        ui->foldersWidget->setDisabled(false);
        beastMode = BeastProxy::MODE_LIGHTMAPS;
    }
}

void BeastDialog::OnSHMode(bool checked)
{
    if(checked)
    {
        ui->foldersWidget->setDisabled(true);
        beastMode = BeastProxy::MODE_SPHERICAL_HARMONICS;
    }
}

void BeastDialog::OnPreviewMode(bool checked)
{
    if(checked)
    {
        ui->foldersWidget->setDisabled(true);
        beastMode = BeastProxy::MODE_PREVIEW;
    }
}

void BeastDialog::closeEvent( QCloseEvent * event )
{
    Q_UNUSED(event);
    ui->cancel->click();
}

QString BeastDialog::GetPath() const
{
    const QString root = ui->scenePath->text();
    const QString output = ui->output->text();
    if (output.isEmpty())
        return QString();

    const QString path = QDir::toNativeSeparators(QString("%1/%2/").arg(root).arg(output));
    return path;
}

BeastProxy::eBeastMode BeastDialog::GetMode() const
{
    return beastMode;
}

QString BeastDialog::GetDefaultPath() const
{
    if ( !scene )
        return QString();

    const QString absoluteFilePath = scene->GetScenePath().GetAbsolutePathname().c_str();
    const QFileInfo fileInfo(absoluteFilePath);
    const QString dir = QDir::toNativeSeparators(fileInfo.absolutePath());

    return dir;
}

void BeastDialog::SetPath( const QString& path )
{
    const QString mandatory = QDir::fromNativeSeparators( GetDefaultPath() );
    const QString pathRoot = path.left(mandatory.length());
    if (QDir( mandatory + '/' ) != QDir( pathRoot + '/' ))
    {
        ui->output->setText( QString() );
        return;
    }

    const QString relative = path.mid(mandatory.length() + 1);
    ui->output->setText(QDir::toNativeSeparators( relative ));
}
