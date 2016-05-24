#include "SpritePackerHelper.h"
#include "SpriteResourcesPacker.h"
#include "Qt/Settings/SettingsManager.h"
#include "Project/ProjectManager.h"
#include <QtConcurrentRun>

#include "TexturePacker/ResourcePacker2D.h"
#include "QtLayer.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Scene/SceneTabWidget.h"

using namespace DAVA;

SpritePackerHelper::SpritePackerHelper()
{
    QObject::connect(&watcher, SIGNAL(finished()), this, SLOT(threadRepackAllFinished()), Qt::QueuedConnection);
}

void SpritePackerHelper::UpdateParticleSprites(DAVA::eGPUFamily gpu)
{
    FilePath projectPath = ProjectManager::Instance()->GetProjectPath();
    if (projectPath.IsEmpty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

    Pack(gpu);

    Reload();
}

void SpritePackerHelper::Pack(DAVA::eGPUFamily gpu)
{
    void* pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    FilePath projectPath = ProjectManager::Instance()->GetProjectPath();
    FilePath inputDir = projectPath + "DataSource/Gfx/Particles/";
    FilePath outputDir = projectPath + "Data/Gfx/Particles/";

    if (!FileSystem::Instance()->IsDirectory(inputDir))
    {
        Logger::Error("[SpritePackerHelper::Pack] inputDir is not directory (%s)", inputDir.GetAbsolutePathname().c_str());
        DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
        return;
    }

    ResourcePacker2D resourcePacker;

    bool isSrcChanged = resourcePacker.RecalculateDirMD5(inputDir, projectPath + "DataSource/Gfx/particles.md5", true);
    if (isSrcChanged)
    {
        SpriteResourcesPacker packer;
        packer.SetInputDir(inputDir);
        packer.SetOutputDir(outputDir);
        packer.PackTextures(gpu);
    }

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
}

void SpritePackerHelper::Reload()
{
    Sprite::ReloadSprites();
    QtMainWindow::Instance()->RestartParticleEffects();
}

void SpritePackerHelper::EnumerateSpritesForParticleEmitter(ParticleEmitter* emitter, Map<String, Sprite*>& sprites)
{
    if (!emitter)
    {
        return;
    }

    size_type layersCount = emitter->layers.size();
    for (size_type il = 0; il < layersCount; ++il)
    {
        ParticleLayer* curLayer = emitter->layers[il];
        Sprite* sprite = curLayer->sprite;
        if (sprite)
        {
            sprites[sprite->GetRelativePathname().GetAbsolutePathname()] = sprite;
        }

        // Superemitter layers might have inner emitter with its own sprites.
        if (curLayer->innerEmitter)
        {
            EnumerateSpritesForParticleEmitter(curLayer->innerEmitter, sprites);
        }
    }
}

void SpritePackerHelper::threadRepackAllFinished()
{
    future = NULL;

    Reload();

    emit readyAll();
}
