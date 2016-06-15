#ifndef __SPRITE_PACKER_HELPER_H__
#define __SPRITE_PACKER_HELPER_H__

#include "DAVAEngine.h"
#include <QFutureWatcher>

class SceneData;
namespace DAVA
{
// Sprite Packer Helper for Particles Editor.
class SpritePackerHelper : public QObject, public DAVA::StaticSingleton<SpritePackerHelper>
{
    Q_OBJECT

public:
    SpritePackerHelper();
    void UpdateParticleSprites(DAVA::eGPUFamily gpu);

signals:
    void readyAll();

private:
    void EnumerateSpritesForReloading(Scene* scene, Map<String, Sprite*>& sprites);
    void EnumerateSpritesForParticleEmitter(ParticleEmitter* emitter, Map<String, Sprite*>& sprites);

    void Pack(DAVA::eGPUFamily gpu);
    void Reload();

    QFuture<void>* future;
    QFutureWatcher<void> watcher;

private slots:
    void threadRepackAllFinished();
};
};
#endif /* __SPRITE_PACKER_HELPER_H__ */