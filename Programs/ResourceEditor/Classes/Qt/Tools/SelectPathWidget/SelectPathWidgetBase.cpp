#include "SelectPathWidgetBase.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Qt/Settings/SettingsManager.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"

#include "TArc/DataProcessing/DataContext.h"
#include "QtTools/FileDialogs/FileDialog.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QStyle>
#include <QMessageBox>

SelectPathWidgetBase::SelectPathWidgetBase(QWidget* _parent, bool _checkForProjectPath, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath, DAVA::String _openFileDialogTitle, DAVA::String _fileFormatDescriotion)
    : QLineEdit(_parent)
    ,
    checkForProjectPath(_checkForProjectPath)
{
    Init(_openDialogDefualtPath, _relativPath, _openFileDialogTitle, _fileFormatDescriotion);
}

SelectPathWidgetBase::~SelectPathWidgetBase()
{
    delete openButton;
    delete clearButton;
}

void SelectPathWidgetBase::Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath, DAVA::String _openFileDialogTitle, DAVA::String _fileFormatFilter)
{
    setAcceptDrops(true);

    relativePath = DAVA::FilePath(_relativPath);
    openDialogDefaultPath = _openDialogDefualtPath;
    openFileDialogTitle = _openFileDialogTitle;
    fileFormatFilter = _fileFormatFilter;

    clearButton = CreateToolButton(":/QtIcons/ccancel.png");
    openButton = CreateToolButton(":/QtIcons/openscene.png");

    connect(clearButton, SIGNAL(clicked()), this, SLOT(EraseClicked()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(OpenClicked()));
    connect(this, SIGNAL(editingFinished()), this, SLOT(acceptEditing()));
}

void SelectPathWidgetBase::resizeEvent(QResizeEvent*)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height()) / 2);

    QSize szOpenBtn = openButton->sizeHint();
    int offsetFromRight = clearButton->isVisible() ? sz.width() : 0;
    openButton->move(rect().right() - offsetFromRight - frameWidth - szOpenBtn.width(), (rect().bottom() + 1 - szOpenBtn.height()) / 2);
}

QToolButton* SelectPathWidgetBase::CreateToolButton(const DAVA::String& iconPath)
{
    QToolButton* retButton;

    retButton = new QToolButton(this);
    QIcon icon(iconPath.c_str());
    retButton->setIcon(icon);
    retButton->setCursor(Qt::ArrowCursor);
    retButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QSize msz = minimumSizeHint();
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(retButton->sizeHint().width() * 2 + frameWidth));
    setMinimumSize(qMax(msz.width(), retButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), retButton->sizeHint().height() + frameWidth * 2 + 2));

    return retButton;
}

void SelectPathWidgetBase::EraseWidget()
{
    setText(QString(""));
    mimeData.clear();
}

void SelectPathWidgetBase::EraseClicked()
{
    EraseWidget();
}

void SelectPathWidgetBase::acceptEditing()
{
    this->setText(getText());
}

void SelectPathWidgetBase::setVisible(bool value)
{
    QLineEdit::setVisible(value);
    SelectPathWidgetBase::resizeEvent(NULL);
}

void SelectPathWidgetBase::OpenClicked()
{
    DAVA::FilePath presentPath(text().toStdString());
    DAVA::FilePath dialogString(openDialogDefaultPath);
    if (DAVA::FileSystem::Instance()->Exists(presentPath.GetDirectory())) //check if file text box clean
    {
        dialogString = presentPath.GetDirectory();
    }
    this->blockSignals(true);
    DAVA::String retString = FileDialog::getOpenFileName(this, openFileDialogTitle.c_str(), QString(dialogString.GetAbsolutePathname().c_str()), fileFormatFilter.c_str()).toStdString();
    this->blockSignals(false);

    if (retString.empty())
    {
        return;
    }

    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::String projectPath = data->GetProjectPath().GetAbsolutePathname();

    if (checkForProjectPath && DAVA::String::npos == retString.find(projectPath))
    {
        QMessageBox::warning(NULL, "Wrong file selected", QString(DAVA::Format("Path %s doesn't belong to project.", retString.c_str()).c_str()), QMessageBox::Ok);
        return;
    }

    HandlePathSelected(retString);
}

void SelectPathWidgetBase::HandlePathSelected(DAVA::String name)
{
    DAVA::FilePath fullPath(name);

    DVASSERT(DAVA::FileSystem::Instance()->Exists(fullPath));

    setText(name);

    DAVA::List<DAVA::FilePath> urls;
    urls.push_back(fullPath);
    DAVA::MimeDataHelper::ConvertToMimeData(urls, &mimeData);
}

void SelectPathWidgetBase::setText(const QString& filePath)
{
    QLineEdit::setText(filePath);
    setToolTip(filePath);
    emit PathSelected(filePath.toStdString());
}

void SelectPathWidgetBase::setText(const DAVA::String& filePath)
{
    SelectPathWidgetBase::setText(QString(filePath.c_str()));
}

DAVA::String SelectPathWidgetBase::getText()
{
    return text().toStdString();
}

DAVA::String SelectPathWidgetBase::ConvertToRelativPath(const DAVA::String& path)
{
    DAVA::FilePath fullPath(path);
    if (DAVA::FileSystem::Instance()->Exists(fullPath))
    {
        return fullPath.GetRelativePathname(relativePath);
    }
    else
    {
        return path;
    }
}

void SelectPathWidgetBase::dropEvent(QDropEvent* event)
{
    const QMimeData* sendedMimeData = event->mimeData();

    DAVA::List<DAVA::String> nameList;

    DAVA::MimeDataHelper::GetItemNamesFromMimeData(sendedMimeData, nameList);
    if (nameList.size() == 0)
    {
        return;
    }

    mimeData.clear();

    foreach (const QString& format, event->mimeData()->formats())
    {
        if (DAVA::MimeDataHelper::IsMimeDataTypeSupported(format.toStdString()))
        {
            mimeData.setData(format, event->mimeData()->data(format));
        }
    }

    DAVA::String itemName = *nameList.begin();
    DAVA::FilePath filePath(itemName);
    if (DAVA::FileSystem::Instance()->Exists(filePath)) // check is it item form scene tree or file system
    {
        setText(filePath.GetAbsolutePathname());
    }
    else
    {
        setText(itemName);
    }

    event->setDropAction(Qt::LinkAction);
    event->accept();
}

void SelectPathWidgetBase::dragEnterEvent(QDragEnterEvent* event)
{
    event->setDropAction(Qt::LinkAction);
    if (DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
    {
        event->accept();
    }
}

void SelectPathWidgetBase::dragMoveEvent(QDragMoveEvent* event)
{
    event->setDropAction(Qt::LinkAction);
    event->accept();
}

bool SelectPathWidgetBase::IsOpenButtonVisible() const
{
    return openButton->isVisible();
}

void SelectPathWidgetBase::SetOpenButtonVisible(bool value)
{
    openButton->setVisible(value);
}

bool SelectPathWidgetBase::IsClearButtonVisible() const
{
    return clearButton->isVisible();
}

void SelectPathWidgetBase::SetClearButtonVisible(bool value)
{
    clearButton->setVisible(value);
}

DAVA::String SelectPathWidgetBase::GetOpenDialogDefaultPath() const
{
    return openDialogDefaultPath;
}

void SelectPathWidgetBase::SetOpenDialogDefaultPath(const DAVA::FilePath& path)
{
    openDialogDefaultPath = path.GetAbsolutePathname();
}

DAVA::String SelectPathWidgetBase::GetFileFormatFilter() const
{
    return fileFormatFilter;
}

void SelectPathWidgetBase::SetFileFormatFilter(const DAVA::String& filter)
{
    fileFormatFilter = filter;
}
