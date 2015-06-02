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


#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QScopedPointer>


namespace Ui
{
    class LogWidget;
};


class QTimer;
class LogModel;
class LogFilterModel;


class LogWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();

    void SetRegisterLoggerAsLocal(bool isLocal);
    LogModel *Model();

private slots:
    void OnFilterChanged();
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void DetectAutoScroll();
    void DoAutoScroll();
    void LoadSettings();

private:
    void FillFiltersCombo();
    void SaveSettings();
    bool eventFilter( QObject* watched, QEvent* event );

    QScopedPointer<Ui::LogWidget> ui;
    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    QPointer<QTimer> eventSkipper;
    bool doAutoScroll;
    bool scrollStateDetected;
    bool registerLoggerAsLocal;
};


#endif // __LOGWIDGET_H__
