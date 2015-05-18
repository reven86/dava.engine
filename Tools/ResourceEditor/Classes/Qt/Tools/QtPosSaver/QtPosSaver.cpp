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

#include <QDebug>


#include "QtTools/WidgetHelpers/WidgetStateHelper.h"


bool QtPosSaver::settingsArchiveIsLoaded = false;
DAVA::RefPtr<DAVA::KeyedArchive> QtPosSaver::settingsArchive( nullptr );

QtPosSaver::QtPosSaver()
	: attachedWidget(nullptr)
{
	if (!settingsArchiveIsLoaded)
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
	if (settingsArchiveIsLoaded)	
	{
		if (nullptr != attachedWidget && !attachedWidgetName.isEmpty())
		{
			SaveGeometry(attachedWidget);
		}

		if (1 == settingsArchive->GetRetainCount())
		{
			settingsArchive->Save("~doc:/ResourceEditorPos.archive");
			settingsArchive.Set( nullptr );
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

	if (nullptr != attachedWidget)
	{
		if (name.isEmpty())
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
	if (nullptr != widget && !attachedWidgetName.isEmpty())
	{
        const auto normalKey = QString( "%1-geometry-%2" ).arg( attachedWidgetName ).arg( widget->objectName() );
        Save( normalKey, widget->saveGeometry() );

        const auto isMaximizedKey = QString( "%1-maximized-%2" ).arg( attachedWidgetName ).arg( widget->objectName() );
		QByteArray mState(1, static_cast<char>(widget->isMaximized()));
        Save( isMaximizedKey, mState );
	}
}

void QtPosSaver::LoadGeometry(QWidget *widget)
{
	if (nullptr != widget && !attachedWidgetName.isEmpty())
	{
        auto helper = WidgetStateHelper::create( widget );
        helper->setTrackedEvents( WidgetStateHelper::ScaleOnDisplayChange );

        const auto normalKey = QString( "%1-geometry-%2" ).arg( attachedWidgetName ).arg( widget->objectName() );
        const auto isMaximizedKey = QString( "%1-maximized-%2" ).arg( attachedWidgetName ).arg( widget->objectName() );

        const auto geometry = Load( normalKey );
        widget->restoreGeometry( geometry );

        const auto mState = Load( isMaximizedKey );
		if (!mState.isEmpty() && mState.at(0) != 0)
		{
			if (widget->isVisible())
			{
				widget->showMaximized();
			}
			else
			{
                const auto f = helper->getTrackedEvents() | WidgetStateHelper::MaximizeOnShowOnce;
                helper->setTrackedEvents( f );
			}
		}
	}
}

void QtPosSaver::SaveState(QSplitter *splitter)
{
    if (nullptr != splitter && !attachedWidgetName.isEmpty())
    {
        const auto splitterKey = QString( "%1-splitter-%2" ).arg( attachedWidgetName ).arg( splitter->objectName() );
        Save( splitterKey, splitter->saveState() );
    }
}

void QtPosSaver::LoadState(QSplitter *splitter)
{
	if (nullptr != splitter && !attachedWidgetName.isEmpty())
	{
        const auto splitterKey = QString( "%1-splitter-%2" ).arg( attachedWidgetName ).arg( splitter->objectName() );
        splitter->restoreState( Load( splitterKey ) );
	}
}

void QtPosSaver::SaveState(QMainWindow *mainwindow)
{
    if (nullptr != mainwindow && !attachedWidgetName.isEmpty())
	{
        const auto mainWindowKey = QString( "%1-mainwindow-%2" ).arg( attachedWidgetName ).arg( mainwindow->objectName() );
        Save( mainWindowKey, mainwindow->saveState() );
	}

    SaveGeometry( mainwindow );
}

void QtPosSaver::LoadState(QMainWindow *mainwindow)
{
	if (nullptr != mainwindow && !attachedWidgetName.isEmpty())
	{
        const auto mainWindowKey = QString( "%1-mainwindow-%2" ).arg( attachedWidgetName ).arg( mainwindow->objectName() );
        mainwindow->restoreState( Load( mainWindowKey ) );
	}

    LoadGeometry( mainwindow );
}

void QtPosSaver::SaveValue(const QString &key, const DAVA::VariantType &value)
{
	if (settingsArchiveIsLoaded && !key.isEmpty())
	{
        const auto valueKey = QString( "%1-%2" ).arg( attachedWidgetName ).arg( key );
        settingsArchive->SetVariant( valueKey.toStdString(), value );
	}
}

DAVA::VariantType QtPosSaver::LoadValue(const QString &key)
{
	DAVA::VariantType v;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
        const auto valueKey = QString( "%1-%2" ).arg( attachedWidgetName ).arg( key );
	    const auto val = settingsArchive->GetVariant( valueKey.toStdString() );
		if (nullptr != val)
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
		settingsArchive->SetByteArray(key.toStdString(), reinterpret_cast<const DAVA::uint8 *>(data.constData()), data.size());
	}
}

QByteArray QtPosSaver::Load(const QString &key)
{
	QByteArray data;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		const auto sz = settingsArchive->GetByteArraySize(key.toStdString());
	    const auto dt = settingsArchive->GetByteArray(key.toStdString());

		if (nullptr != dt)
		{
			data.append(reinterpret_cast<const char *>(dt), sz);
		}
	}

	return data;
}
