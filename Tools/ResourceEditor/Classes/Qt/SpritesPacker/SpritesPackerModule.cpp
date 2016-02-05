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


#include "Render/RenderBase.h"

#include "SpritesPacker/SpritesPackerModule.h"

#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"

#include "Main/mainwindow.h"

#include <QAction>

SpritesPackerModule::SpritesPackerModule()
    : QObject(nullptr)
    , spritesPacker(new SpritesPacker())
{
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");
}

SpritesPackerModule::~SpritesPackerModule() = default;

QAction* SpritesPackerModule::GetReloadAction() const
{
    return reloadSpritesAction;
}

void SpritesPackerModule::SetAction(QAction* reloadSpritesAction_)
{
    if (reloadSpritesAction != nullptr)
    {
        disconnect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }

    reloadSpritesAction = reloadSpritesAction_;

    if (reloadSpritesAction != nullptr)
    {
        connect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }
}

void SpritesPackerModule::RepackWithDialog()
{
    SetupSpritesPacker(ProjectManager::Instance()->GetProjectPath());
    ShowPackerDialog();
    ReloadObjects();
}

void SpritesPackerModule::SetupSpritesPacker(const DAVA::FilePath& projectPath)
{
    FilePath inputDir = projectPath + "/DataSource/Gfx/Particles";
    FilePath outputDir = projectPath + "/Data/Gfx/Particles";

    spritesPacker->ClearTasks();
    spritesPacker->AddTask(QString::fromStdString(inputDir.GetAbsolutePathname()), QString::fromStdString(outputDir.GetAbsolutePathname()));
}

void SpritesPackerModule::ShowPackerDialog()
{
    DAVA::uint32 lastFlags = acceptableLoggerFlags;
    acceptableLoggerFlags = (1 << DAVA::Logger::LEVEL_ERROR) | (1 << DAVA::Logger::LEVEL_WARNING);

    DialogReloadSprites dialogReloadSprites(spritesPacker.get(), QtMainWindow::Instance());
    dialogReloadSprites.exec();
    acceptableLoggerFlags = lastFlags;
}

void SpritesPackerModule::RepackSilently(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu)
{
    SetupSpritesPacker(ProjectManager::Instance()->GetProjectPath());

    QtMainWindow::Instance()->WaitStart("Reload Particles particles for project", "Reload Particles for " + QString::fromStdString(projectPath.GetAbsolutePathname()));

    spritesPacker->ReloadSprites(false, false, gpu, DAVA::TextureConverter::ECQ_DEFAULT);

    QtMainWindow::Instance()->WaitStop();

    ReloadObjects();
}

void SpritesPackerModule::ReloadObjects()
{
    Sprite::ReloadSprites();

    emit SpritesReloaded();
}
