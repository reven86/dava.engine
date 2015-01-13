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



#ifndef __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__
#define __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__

#include <QProgressDialog>
#include <QTimer>

class QProgressBar;

class  QResourceEditorProgressDialog
	: public QProgressDialog
{

	Q_OBJECT

public:

	QResourceEditorProgressDialog( QWidget * parent = 0, Qt::WindowFlags f = 0, bool isCycled = false );

	bool getCycledFlag()
	{ 
		return isCycled;
	}

	unsigned int getTimeIntervalForParcent() 
	{ 
		return timeIntervalForPercent;
	}
	
	void getTimeIntervalForParcent(unsigned int value) 
	{ 
		timeIntervalForPercent = value;
	}

protected:

	virtual void	showEvent ( QShowEvent * e );

	bool				isCycled;
	unsigned int		timeIntervalForPercent;
	QTimer				timer;

private slots:
	
	void UpdateProgress();
};

#endif // __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__