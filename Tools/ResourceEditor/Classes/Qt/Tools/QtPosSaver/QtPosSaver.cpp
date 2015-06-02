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


#include "Tools/QtPosSaver/QtPosSaver.h"
#include <QHeaderView>

bool QtPosSaver::settingsArchiveIsLoaded = false;
DAVA::RefPtr<DAVA::KeyedArchive> QtPosSaver::settingsArchive( NULL );

QtPosSaver::QtPosSaver()
	: attachedWidget(NULL)
{
	if(!settingsArchiveIsLoaded)
	{
		settingsArchive.Set( new DAVA::KeyedArchive() );
		settingsArchive->Load("~doc:/ResourceEditorPos.archive");
		settingsArchiveIsLoaded = true;
	}
	else
	{
		settingsArchive->Retain();
	}
}

QtPosSaver::~QtPosSaver()
{
	if(settingsArchiveIsLoaded)	
	{
		if(NULL != attachedWidget && !attachedWidgetName.isEmpty())
		{
			SaveGeometry(attachedWidget);
		}

		if(1 == settingsArchive->GetRetainCount())
		{
			settingsArchive->Save("~doc:/ResourceEditorPos.archive");
			settingsArchive.Set( NULL );
			settingsArchiveIsLoaded = false;
		}
		else
		{
			settingsArchive->Release();
		}
	}
}

void QtPosSaver::Attach(QWidget *widget, const QString &name)
{
	attachedWidget = widget;

	if(NULL != attachedWidget)
	{
		if(name.isEmpty())
		{
			attachedWidgetName = attachedWidget->objectName();
		}
		else
		{
			attachedWidgetName = name;
		}

		LoadGeometry(attachedWidget);
	}
}

void QtPosSaver::SaveGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-geometry-" + widget->objectName();
		Save(key, widget->saveGeometry());

		key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState(1, (char) widget->isMaximized());
		Save(key, mState);
	}
}

void QtPosSaver::LoadGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState = Load(key);
		if(!mState.isEmpty() && mState.at(0))
		{
			if(widget->isVisible())
			{
				widget->showMaximized();
			}
			else
			{
				widget->setWindowState((widget->windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen)) | Qt::WindowMaximized);
			}
		}

		key = attachedWidgetName + "-geometry-" + widget->objectName();
		widget->restoreGeometry(Load(key));
	}
}

void QtPosSaver::SaveState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
			Save(key, splitter->saveState());
	}
}

void QtPosSaver::LoadState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
		splitter->restoreState(Load(key));
	}
}

void QtPosSaver::SaveState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		Save(key, mainwindow->saveState());
	}
}

void QtPosSaver::LoadState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		mainwindow->restoreState(Load(key));
	}
}

void QtPosSaver::SaveValue(const QString &key, const DAVA::VariantType &value)
{
	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		QString k = attachedWidgetName + "-" + key;
		settingsArchive->SetVariant(k.toStdString(), value);
	}
}

DAVA::VariantType QtPosSaver::LoadValue(const QString &key)
{
	DAVA::VariantType v;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		QString k = attachedWidgetName + "-" + key;
		DAVA::VariantType *val = settingsArchive->GetVariant(k.toStdString());
		if(NULL != val)
		{
			v = *val;
		}
	}

	return v;
}

void QtPosSaver::Save(const QString &key, const QByteArray &data)
{
	if(settingsArchiveIsLoaded && !key.isEmpty() && !data.isEmpty())
	{
		settingsArchive->SetByteArray(key.toStdString(), (const DAVA::uint8 *) data.constData(), data.size());
	}
}

QByteArray QtPosSaver::Load(const QString &key)
{
	QByteArray data;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		int sz = settingsArchive->GetByteArraySize(key.toStdString());
		const DAVA::uint8 *dt = settingsArchive->GetByteArray(key.toStdString());

		if(NULL != dt)
		{
			data.append((const char *) dt, sz);
		}
	}

	return data;
}