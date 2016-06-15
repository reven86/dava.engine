#include "TextureListModel.h"
#include <QPainter>
#include <QFileInfo>

#include "Scene/SelectableGroup.h"
#include "Render/PixelFormatDescriptor.h"

TextureListModel::TextureListModel(QObject* parent /* = 0 */)
    : QAbstractListModel(parent)
    , curSortMode(TextureListModel::SortByName)
    , curFilterBySelectedNode(false)
    , activeScene(NULL)
{
}

TextureListModel::~TextureListModel()
{
    clear();
}

int TextureListModel::rowCount(const QModelIndex& /* parent */) const
{
    return textureDescriptorsFiltredSorted.size();
}

QVariant TextureListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const DAVA::TextureDescriptor* curTextureDescriptor = textureDescriptorsFiltredSorted[index.row()];

        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant(QFileInfo(curTextureDescriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str()).fileName());
            break;

        default:
            break;
        }
    }

    return QVariant();
}

DAVA::Texture* TextureListModel::getTexture(const QModelIndex& index) const
{
    DAVA::Texture* ret = NULL;
    DAVA::TextureDescriptor* desc = getDescriptor(index);

    if (index.isValid() && texturesAll.contains(desc))
    {
        ret = texturesAll[desc];
    }

    return ret;
}

DAVA::Texture* TextureListModel::getTexture(const DAVA::TextureDescriptor* descriptor) const
{
    DAVA::Texture* ret = NULL;

    if (texturesAll.contains(descriptor))
    {
        ret = texturesAll[descriptor];
    }

    return ret;
}

DAVA::TextureDescriptor* TextureListModel::getDescriptor(const QModelIndex& index) const
{
    DAVA::TextureDescriptor* ret = NULL;

    if (index.isValid() && textureDescriptorsFiltredSorted.size() > index.row())
    {
        ret = textureDescriptorsFiltredSorted[index.row()];
    }

    return ret;
}

bool TextureListModel::isHighlited(const QModelIndex& index) const
{
    bool ret = false;
    DAVA::TextureDescriptor* descriptor = getDescriptor(index);

    if (NULL != descriptor)
    {
        ret = textureDescriptorsHighlight.contains(descriptor);
    }

    return ret;
}

void TextureListModel::dataReady(const DAVA::TextureDescriptor* desc)
{
    int i = textureDescriptorsFiltredSorted.indexOf((DAVA::TextureDescriptor * const)desc);
    emit dataChanged(this->index(i), this->index(i));
}

QModelIndex TextureListModel::getIndex(const DAVA::TextureDescriptor* textureDescriptor)
{
    if (textureDescriptor == nullptr)
        return QModelIndex();

    auto iter = std::find(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), textureDescriptor);
    if (iter == textureDescriptorsFiltredSorted.end())
        return QModelIndex();

    return index(std::distance(textureDescriptorsFiltredSorted.begin(), iter));
}

void TextureListModel::setFilter(QString filter)
{
    beginResetModel();
    curFilter = filter;
    applyFilterAndSort();
    endResetModel();
}

void TextureListModel::setFilterBySelectedNode(bool enabled)
{
    beginResetModel();
    curFilterBySelectedNode = enabled;
    applyFilterAndSort();
    endResetModel();
}

void TextureListModel::setSortMode(TextureListModel::TextureListSortMode sortMode)
{
    beginResetModel();
    curSortMode = sortMode;
    applyFilterAndSort();
    endResetModel();
}

void TextureListModel::setScene(SceneEditor2* scene)
{
    beginResetModel();

    clear();

    activeScene = scene;

    SceneHelper::TextureCollector collector;
    SceneHelper::EnumerateSceneTextures(scene, collector);
    DAVA::TexturesMap& texturesInNode = collector.GetTextures();

    for (DAVA::TexturesMap::iterator t = texturesInNode.begin(); t != texturesInNode.end(); ++t)
    {
        DAVA::TextureDescriptor* descriptor = t->second->texDescriptor;
        if (NULL != descriptor && DAVA::FileSystem::Instance()->Exists(descriptor->pathname))
        {
            textureDescriptorsAll.push_back(descriptor);
            texturesAll[descriptor] = SafeRetain(t->second);
        }
    }

    applyFilterAndSort();

    endResetModel();

    if (activeScene != nullptr)
    {
        setHighlight(&activeScene->selectionSystem->GetSelection());
    }
}

void TextureListModel::setHighlight(const SelectableGroup* nodes)
{
    beginResetModel();

    textureDescriptorsHighlight.clear();

    if (nullptr != nodes)
    {
        SceneHelper::TextureCollector collector;
        for (auto entity : nodes->ObjectsOfType<DAVA::Entity>())
        {
            SceneHelper::EnumerateEntityTextures(activeScene, entity, collector);
        }
        DAVA::TexturesMap& nodeTextures = collector.GetTextures();

        const DAVA::uint32 descriptorsCount = static_cast<const DAVA::uint32>(textureDescriptorsAll.size());
        for (const auto& nTex : nodeTextures)
        {
            const DAVA::FilePath& path = nTex.first;
            for (DAVA::uint32 d = 0; d < descriptorsCount; ++d)
            {
                if (textureDescriptorsAll[d]->pathname == path)
                {
                    textureDescriptorsHighlight.push_back(textureDescriptorsAll[d]);
                }
            }
        }
    }

    if (curFilterBySelectedNode)
    {
        applyFilterAndSort();
    }

    endResetModel();
}

void TextureListModel::clear()
{
    activeScene = NULL;

    QMapIterator<const DAVA::TextureDescriptor*, DAVA::Texture*> it(texturesAll);
    while (it.hasNext())
    {
        it.next();
        it.value()->Release();
    }

    texturesAll.clear();
    textureDescriptorsHighlight.clear();
    textureDescriptorsFiltredSorted.clear();
    textureDescriptorsAll.clear();
}

void TextureListModel::applyFilterAndSort()
{
    textureDescriptorsFiltredSorted.clear();

    for (int i = 0; i < (int)textureDescriptorsAll.size(); ++i)
    {
        if ((curFilter.isEmpty() || DAVA::String::npos != textureDescriptorsAll[i]->pathname.GetAbsolutePathname().find(curFilter.toStdString())) && // text filter
            (!curFilterBySelectedNode || textureDescriptorsHighlight.contains(textureDescriptorsAll[i]))) // cur selected node filter
        {
            textureDescriptorsFiltredSorted.push_back(textureDescriptorsAll[i]);
        }
    }

    switch (curSortMode)
    {
    case SortByName:
    {
        SortFnByName fn;
        std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), fn);
    }
    break;
    case SortByFileSize:
    {
        SortFnByFileSize fn;
        std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), fn);
    }
    break;
    case SortByImageSize:
    {
        SortFnByImageSize fn(this);
        std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), fn);
    }
    break;
    case SortByDataSize:
    {
        SortFnByDataSize fn(this);
        std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), fn);
    }
    break;
    default:
        break;
    }
}

bool SortFnByName::operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
    return QFileInfo(t1->pathname.GetAbsolutePathname().c_str()).completeBaseName() < QFileInfo(t2->pathname.GetAbsolutePathname().c_str()).completeBaseName();
}

bool SortFnByFileSize::operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
    return QFileInfo(t1->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size() < QFileInfo(t2->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size();
}

bool SortFnByImageSize::operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
    DAVA::Texture* tx1 = model->getTexture(t1);
    DAVA::Texture* tx2 = model->getTexture(t2);

    return (tx1->width * tx1->height) < (tx2->width * tx2->height);
}

bool SortFnByDataSize::operator()(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
    DAVA::Texture* tx1 = model->getTexture(t1);
    DAVA::Texture* tx2 = model->getTexture(t2);

    return (tx1->width * tx1->height * DAVA::PixelFormatDescriptor::GetPixelFormatSizeInBytes(tx1->GetFormat())) < (tx2->width * tx2->height * DAVA::PixelFormatDescriptor::GetPixelFormatSizeInBytes(tx2->GetFormat()));
}
