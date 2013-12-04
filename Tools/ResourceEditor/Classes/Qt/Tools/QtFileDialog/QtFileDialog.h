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



#ifndef __QT_FILE_DIALOG_H__
#define __QT_FILE_DIALOG_H__

#include <QFileDialog>
#include <QCoreApplication>

// This class calls QtCoreApplication::processEvents after native OS openFile dialog
// execution to prevent OpenGL context replacement on MacOS
// see DF-2322

class QtFileDialog : public QFileDialog
{
public:
	static QString getOpenFileName(QWidget *parent = 0,
		const QString &caption = QString(),
		const QString &dir = QString(),
		const QString &filter = QString(),
		QString *selectedFilter = 0,
		Options options = 0)
	{
		QString ret = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
		QCoreApplication::processEvents();
		return ret;
	}

	static QString getSaveFileName(QWidget *parent = 0,
		const QString &caption = QString(),
		const QString &dir = QString(),
		const QString &filter = QString(),
		QString *selectedFilter = 0,
		Options options = 0)
	{
		QString ret = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
		QCoreApplication::processEvents();
		return ret;
	}

	static QString getExistingDirectory(QWidget *parent = 0,
		const QString &caption = QString(),
		const QString &dir = QString(),
		Options options = ShowDirsOnly)
	{
		QString ret = QFileDialog::getExistingDirectory(parent, caption, dir, options);
		QCoreApplication::processEvents();
		return ret;
	}

	static QStringList getOpenFileNames(QWidget *parent = 0,
		const QString &caption = QString(),
		const QString &dir = QString(),
		const QString &filter = QString(),
		QString *selectedFilter = 0,
		Options options = 0)
	{
		QStringList ret = QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
		QCoreApplication::processEvents();
		return ret;
	}

	virtual void closeEvent(QCloseEvent * e)
	{
		QFileDialog::closeEvent(e);
		QCoreApplication::processEvents();
	}
};

#endif // __QT_FILE_DIALOG_H__
