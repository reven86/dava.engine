#include "FilePathBrowser.h"

#include <QAction>
#include <QAbstractButton>
#include <QDebug>
#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>


namespace
{
    const QSize cButtonSize = QSize(24, 24);
}


FilePathBrowser::FilePathBrowser(QWidget* parent)
    : LineEditEx(parent)
{
    SetUseDelayedUpdate(false);
    InitActions();

    QBoxLayout *l = qobject_cast<QBoxLayout *>( layout() );
    if ( l != NULL )
    {
        l->setContentsMargins( 2, 2, 2, 2 );
    }

    connect( this, SIGNAL( returnPressed() ), SLOT( OnReturnPressed() ) );
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
    const QString newPath = QFileDialog::getOpenFileName( this, hintText, DefaultBrowsePath(), filter, NULL, 0 );
    TryToAcceptPath(newPath);
}

void FilePathBrowser::OnReturnPressed()
{
    TryToAcceptPath(text());
}

void FilePathBrowser::InitActions()
{
    QAction *browse = new QAction(this);
    browse->setToolTip("Browse...");
    browse->setIcon(QIcon(":/QtIcons/openscene.png"));
    connect( browse, SIGNAL( triggered() ), SLOT( OnBrowse() ) );

    addAction(browse);
}

void FilePathBrowser::TryToAcceptPath(const QString& _path)
{
    QFileInfo newInfo( _path );
    
    if (newInfo.isFile())
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
