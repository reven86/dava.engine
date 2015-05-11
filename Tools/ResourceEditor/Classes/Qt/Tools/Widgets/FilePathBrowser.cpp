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
