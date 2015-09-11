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


#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#include "DAVAEngine.h"
#include <QString>
#include <QColor>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>


#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

DAVA::FilePath PathnameToDAVAStyle(const QString &convertedPathname);

DAVA::FilePath GetOpenFileName(const DAVA::String &title, const DAVA::FilePath &pathname, const DAVA::String &filter);

void ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText);

DAVA::WideString SizeInBytesToWideString(DAVA::float32 size);
DAVA::String SizeInBytesToString(DAVA::float32 size);

DAVA::Image * CreateTopLevelImage(const DAVA::FilePath &imagePathname);

void ShowErrorDialog(const DAVA::Set<DAVA::String> &errors);
void ShowErrorDialog(const DAVA::String &errorMessage, const DAVA::String &title = "Error");

bool IsKeyModificatorPressed(DAVA::int32 key);
bool IsKeyModificatorsPressed();

QColor ColorToQColor(const DAVA::Color& color);
DAVA::Color QColorToColor(const QColor& qcolor);

enum eMessageBoxFlags
{
    MB_FLAG_YES = QMessageBox::Yes,
    MB_FLAG_NO = QMessageBox::No,
    MB_FLAG_CANCEL = QMessageBox::Cancel
};

int ShowQuestion(const DAVA::String &header, const DAVA::String &question, int buttons, int defaultButton);

#ifdef __DAVAENGINE_WIN32__
	const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Window;
#else
	const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Tool;
#endif

DAVA::String ReplaceInString(const DAVA::String & sourceString, const DAVA::String & what, const DAVA::String & on);

void ShowFileInExplorer( const QString& path );


// Method for debugging. Save image to file
void SaveSpriteToFile(DAVA::Sprite * sprite, const DAVA::FilePath & path);
void SaveTextureToFile(DAVA::Texture * texture, const DAVA::FilePath & path);
void SaveImageToFile(DAVA::Image * image, const DAVA::FilePath & path);


#endif // __QT_UTILS_H__
