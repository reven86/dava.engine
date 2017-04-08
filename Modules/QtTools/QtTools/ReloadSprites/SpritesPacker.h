#ifndef __SPRITES_PACKER_H__
#define __SPRITES_PACKER_H__

#include <Tools/TextureCompression/TextureConverter.h>
#include <Tools/TexturePacker/ResourcePacker2D.h>

#include <Render/RenderBase.h>

#include <QObject>
#include <QDir>
#include <atomic>

namespace DAVA
{
class ResourcePacker2D;
class AssetCacheClient;
}

class SpritesPacker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ IsRunning WRITE SetRunning NOTIFY RunningStateChanged);

public:
    SpritesPacker(QObject* parent = nullptr);

    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, const DAVA::String& comment);
    bool IsUsingCache() const;

    void AddTask(const QDir& inputDir, const QDir& outputDir);
    void ClearTasks();
    Q_INVOKABLE void ReloadSprites(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);

    const DAVA::ResourcePacker2D& GetResourcePacker() const;

public slots:
    void Cancel();
signals:
    void Finished();

private:
    DAVA::ResourcePacker2D resourcePacker2D;
    QList<QPair<QDir, QDir>> tasks;

    //properties section
public:
    bool IsRunning() const;
public slots:
    void SetRunning(bool arg);
signals:
    void RunningStateChanged(bool arg);

private:
    std::atomic<bool> running;
};

#endif //__SPRITES_PACKER_H__
