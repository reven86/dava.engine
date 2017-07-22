#include "SelectEntityPathWidget.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Scene/SceneEditor2.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QStyle>

#define MIME_ENTITY_NAME "application/dava.entity"
#define MIME_EMITER_NAME "application/dava.emitter"
#define MIME_URI_LIST_NAME "text/uri-list"

SelectEntityPathWidget::SelectEntityPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
    : SelectPathWidgetBase(_parent, false, _openDialogDefualtPath, _relativPath, "Open Scene File", "Scene File (*.sc2)")
{
    allowedFormatsList.push_back(".sc2");
}

SelectEntityPathWidget::~SelectEntityPathWidget()
{
    Q_FOREACH (DAVA::Entity* item, entitiesToHold)
    {
        SafeRelease(item);
    }
}

void SelectEntityPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
    {
        return;
    }

    event->setDropAction(Qt::LinkAction);

    bool isFormatSupported = true;

    if (event->mimeData()->hasFormat(MIME_URI_LIST_NAME))
    {
        isFormatSupported = false;
        DAVA::FilePath path(event->mimeData()->urls().first().toLocalFile().toStdString());
        Q_FOREACH (DAVA::String item, allowedFormatsList)
        {
            if (path.IsEqualToExtension(item))
            {
                isFormatSupported = true;
                break;
            }
        }
    }
    if (isFormatSupported)
    {
        event->accept();
    }
}

DAVA::Entity* SelectEntityPathWidget::GetOutputEntity(SceneEditor2* editor)
{
    DAVA::List<DAVA::Entity*> retList;
    ConvertFromMimeData(&mimeData, retList, editor);
    DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin() : NULL;
    return retEntity;
}

void SelectEntityPathWidget::ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor)
{
    if (mimeData->hasFormat(MIME_ENTITY_NAME) || mimeData->hasFormat(MIME_EMITER_NAME))
    {
        ConvertQMimeDataFromSceneTree(mimeData, retList);
    }
    else if (mimeData->hasFormat(MIME_URI_LIST_NAME))
    {
        ConvertQMimeDataFromFilePath(mimeData, retList, sceneEditor);
    }
}

void SelectEntityPathWidget::ConvertQMimeDataFromSceneTree(const QMimeData* mimeData,
                                                           DAVA::List<DAVA::Entity*>& retList)
{
    retList = DAVA::MimeDataHelper::GetPointersFromSceneTreeMime(mimeData);
    SetEntities(retList, true);
}

void SelectEntityPathWidget::ConvertQMimeDataFromFilePath(const QMimeData* mimeData,
                                                          DAVA::List<DAVA::Entity*>& retList,
                                                          SceneEditor2* sceneEditor)
{
    if (mimeData == NULL || sceneEditor == NULL || !mimeData->hasUrls())
    {
        return;
    }

    retList.clear();

    QList<QUrl> droppedUrls = mimeData->urls();

    Q_FOREACH (QUrl url, droppedUrls)
    {
        DAVA::FilePath filePath(url.toLocalFile().toStdString());
        if (!(DAVA::FileSystem::Instance()->Exists(filePath) && filePath.GetExtension() == ".sc2"))
        {
            continue;
        }

        DAVA::Entity* entity = sceneEditor->structureSystem->Load(filePath, false);

        if (NULL != entity)
        {
            retList.push_back(entity);
        }
    }
    // for just created entities no need to increase refCouner
    // it will be released in ~SelectEntityPathWidget()
    SetEntities(retList, false);
}

void SelectEntityPathWidget::SetEntities(const DAVA::List<DAVA::Entity*>& list, bool perfromRertain)
{
    Q_FOREACH (DAVA::Entity* item, entitiesToHold)
    {
        SafeRelease(item);
    }
    entitiesToHold = list;
    if (perfromRertain)
    {
        Q_FOREACH (DAVA::Entity* item, entitiesToHold)
        {
            SafeRetain(item);
        }
    }
}
