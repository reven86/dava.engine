#ifndef __SPRITES_PACKER_MODULE_H__
#define __SPRITES_PACKER_MODULE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

#include "TextureCompression/TextureConverter.h"

#include <QObject>

namespace DAVA
{
class AssetCacheClient;
}

class SpritesPacker;
class QAction;
class QDialog;

class SpritesPackerModule final : public QObject
{
    Q_OBJECT

public:
    SpritesPackerModule();
    ~SpritesPackerModule() override;

    QAction* GetReloadAction() const;
    void SetAction(QAction* reloadSpritesAction);

    void RepackImmediately(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu);

    bool IsRunning() const;

signals:
    void SpritesReloaded();

private slots:

    void RepackWithDialog();

private:
    void SetupSpritesPacker(const DAVA::FilePath& projectPath);
    void ShowPackerDialog();
    void ReloadObjects();

    void ConnectCacheClient();
    void DisconnectCacheClient();
    void DisconnectCacheClientInternal(DAVA::AssetCacheClient* cacheClient);

    void ProcessSilentPacking(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);

    void CreateWaitDialog(const DAVA::FilePath& projectPath);
    void CloseWaitDialog();

private:
    DAVA::AssetCacheClient* cacheClient = nullptr;

    std::unique_ptr<SpritesPacker> spritesPacker;
    QAction* reloadSpritesAction = nullptr;

    QDialog* waitDialog = nullptr;
};

#endif // __SPRITES_PACKER_MODULE_H__
