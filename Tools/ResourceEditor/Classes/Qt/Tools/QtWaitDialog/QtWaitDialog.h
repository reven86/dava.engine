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



#ifndef __RESOURCEEDITORQT__QTWAITDIALOG__
#define __RESOURCEEDITORQT__QTWAITDIALOG__

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QEventLoop>

namespace Ui
{
	class QtWaitDialog;
}

class QtWaitDialog
    : public QWidget
{
	Q_OBJECT

public:
	QtWaitDialog(QWidget *parent = 0);
	~QtWaitDialog();

	void Exec(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel);
	void Show(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel);
	void Reset();

	void SetMessage(const QString &message);

	void SetRange(int min, int max);
	void SetRangeMin(int min);
	void SetRangeMax(int max);
	void SetValue(int value);
	void EnableCancel(bool enable);

	bool WasCanceled() const;

signals:
	void canceled();

protected slots:
	void CancelPressed();
	void WaitCanceled();

private:
    void processEvents();
    
	void Setup(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel);
	Ui::QtWaitDialog *ui;

	bool wasCanceled;
    bool isRunnedFromExec;
    QEventLoop loop;
};

#endif // __RESOURCEEDITORQT__MAINWAITDIALOG__
