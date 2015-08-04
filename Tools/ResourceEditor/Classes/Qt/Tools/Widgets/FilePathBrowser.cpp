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


#include "FilePathBrowser.h"

#include <QAction>
#include <QAbstractButton>
#include <QDebug>
#include <QBoxLayout>
#include <QFileInfo>
#include <QKeyEvent>

#include "QtTools/FileDialog/FileDialog.h"

namespace
{
    const QSize cButtonSize = QSize(24, 24);
}


FilePathBrowser::FilePathBrowser(QWidget* parent)
    : LineEditEx(parent)
    , iconCache(2)
    , allowInvalidPath(false)
{
    InitButtons();

    QBoxLayout *l = qobject_cast<QBoxLayout *>( layout() );
    if ( l != NULL )
    {
        l->setContentsMargins( 2, 2, 2, 2 );
    }

    connect( this, SIGNAL( returnPressed() ), SLOT( OnReturnPressed() ) );
    connect( this, SIGNAL( textUpdated( const QString& ) ), SLOT( ValidatePath() ) );
}

FilePathBrowser::~FilePathBrowser()
{
}

void FilePathBrowser::SetHint(const QString& hint)
{
    hintText = hint;
    setPlaceholderText(hintText);
}

void FilePathBrowser::SetDefaultFolder(const QString& _path)
{
    defaultFolder = _path;
}

void FilePathBrowser::SetPath(const QString& _path)
{
    path = _path;
    setText(path);
    setToolTip(path);
}

void FilePathBrowser::SetFilter(const QString& _filter)
{
    filter = _filter;
}

QSize FilePathBrowser::sizeHint() const
{
    QSize hint = LineEditEx::sizeHint();
    const bool hasActions = !actions().isEmpty();

    if (hasActions)
    {
        hint.rheight() = cButtonSize.height();

        QBoxLayout *l = qobject_cast<QBoxLayout *>( layout() );
        if ( l != NULL )
        {
            int top, left, right, bottom;
            l->getContentsMargins( &left, &top, &right, &bottom );
            hint.rheight() += top + bottom;
        }
    }

    return hint;
}

QString FilePathBrowser::DefaultBrowsePath()
{
    const QFileInfo pathInfo(text());
    const QFileInfo defaultInfo(defaultFolder);

    if (pathInfo.isFile())
        return path;

    if (defaultInfo.isDir())
        return defaultFolder;

    return QString();
}

void FilePathBrowser::OnBrowse()
{
    const QString newPath = FileDialog::getOpenFileName( this, hintText, DefaultBrowsePath(), filter, NULL, 0 );
    TryToAcceptPath(newPath);
}

void FilePathBrowser::OnReturnPressed()
{
    TryToAcceptPath(text());
}

void FilePathBrowser::ValidatePath()
{
    const bool isValid = QFileInfo(text()).isFile();    

    // Icon
    QPixmap *pix = NULL;
    if ( iconCache.contains(isValid) )
    {
        pix = iconCache.object(isValid);
    }
    else
    {
        const QString uri = isValid ? ":/QtIcons/accept_button.png" : ":/QtIcons/prohibition_button.png";
        pix = new QPixmap(uri);
        iconCache.insert(isValid, pix);
    }
    validIcon->setPixmap(*pix);

    // Tooltip
    const QString toolTip = isValid ? "File exists" : "File doesn't exists";
    validIcon->setToolTip(toolTip);
}

void FilePathBrowser::InitButtons()
{
    validIcon = new QLabel();
    validIcon->setFixedSize(ButtonSizeHint(NULL));
    validIcon->setAlignment(Qt::AlignCenter);
    AddCustomWidget(validIcon);

    QAction *browse = new QAction(this);
    browse->setToolTip("Browse...");
    browse->setIcon(QIcon(":/QtIcons/openscene.png"));
    connect( browse, SIGNAL( triggered() ), SLOT( OnBrowse() ) );
    addAction(browse);
}

void FilePathBrowser::TryToAcceptPath(const QString& _path)
{
    QFileInfo newInfo( _path );
    
    if (allowInvalidPath || newInfo.isFile())
    {
        SetPath(_path);
        emit pathChanged(_path);
    }
}

QSize FilePathBrowser::ButtonSizeHint(const QAction * action) const
{
    Q_UNUSED(action);
    return cButtonSize;
}

void FilePathBrowser::keyPressEvent(QKeyEvent* event)
{
    LineEditEx::keyPressEvent(event);

    switch ( event->key() )
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        event->accept();
        break;

    default:
        break;
    }
}
