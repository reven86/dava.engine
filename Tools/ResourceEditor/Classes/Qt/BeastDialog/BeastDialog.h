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


#ifndef BEAST_DIALOG
#define BEAST_DIALOG


#include <QWidget>
#include <QScopedPointer>
#include <QPointer>

#include "ui_BeastDialog.h"

#include "Beast/BeastProxy.h"


class SceneEditor2;
class QEventLoop;


class BeastDialog
    : public QWidget
{
    Q_OBJECT

public:
    BeastDialog(QWidget *parent = 0);
    ~BeastDialog();

    void SetScene(SceneEditor2 *scene);
    bool Exec(QWidget *parent = 0);
    QString GetPath() const;
    BeastProxy::eBeastMode GetMode() const;

private slots:
    void OnStart();
    void OnCancel();
    void OnBrowse();
    void OnTextChanged();
    void OnLightmapMode(bool checked);
    void OnSHMode(bool checked);
    void OnPreviewMode(bool checked);

private:
    void closeEvent( QCloseEvent * event );
    QString GetDefaultPath() const;
    void SetPath( const QString& path );

    QScopedPointer<Ui::BeastDialog> ui;
    QPointer< QEventLoop > loop;
    SceneEditor2 *scene;
    bool result;

    BeastProxy::eBeastMode beastMode;
};


#endif // BEAST_DIALOG
