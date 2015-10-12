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

#include "Library.h"

#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include "core_qt_common/helpers/qt_helpers.hpp"

#include "FileSystemModel.h"

#include <QFileDialog>
#include <QModelIndex>

namespace
{
std::string SCENE_EXTENSION = ".sc2";
char const * SCENE_FILE_FILTER = "Scene (*.sc2)";
#ifdef _WIN32
char const* FILE_SYSTEM_ROOT = "d:\\";
#elif __APPLE__
char const* FILE_SYSTEM_ROOT = "/";
#endif

bool IsSceneFile(std::string const & filePath)
{
    return std::equal(SCENE_EXTENSION.rbegin(), SCENE_EXTENSION.rend(), filePath.rbegin());
}

} // namespace

Library::Library()
    : selectedScene("")
{
}

void Library::Initialize(IUIFramework & uiFramework, IUIApplication & uiApplication)
{
    model = new FileSystemModel(FILE_SYSTEM_ROOT, std::bind(&IsSceneFile, std::placeholders::_1));
    openSceneAction = uiFramework.createAction("OpenScene", std::bind(&Library::OnOpenSceneMenu, this));
    uiApplication.addAction(*openSceneAction);

    libraryView = uiFramework.createView("qrc:/default/Library.qml", IUIFramework::ResourceType::Url, ObjectHandle(this));
}

void Library::Finilize()
{
    delete model;
    openSceneAction.reset();
}

IView & Library::GetView()
{
    return *libraryView;
}

void Library::OnOpenSceneMenu()
{
    QString scenePath = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Choose your destiny"),
                                                     QString(FILE_SYSTEM_ROOT), QString(SCENE_FILE_FILTER));
    if (scenePath.isEmpty())
        return;
        
    emit OpenScene(scenePath.toStdString());
}

bool Library::CanBeLoaded() const
{
    return !selectedScene.empty();
}

void Library::OnOpenSceneButton()
{
    DVASSERT(CanBeLoaded());

    emit OpenScene(selectedScene);
}

void Library::OnSelectionChanged(const QList<QVariant> & selections)
{
    foreach(QVariant s, selections)
    {
        if (!s.canConvert<QModelIndex>())
            continue;

        QModelIndex index = s.toModelIndex();
        if (!index.isValid())
            continue;

        FileSystemModel::Item * item = reinterpret_cast<FileSystemModel::Item *>(index.internalPointer());
        std::string const & selectedFile = item->getFilePath();
        if (IsSceneFile(selectedFile))
            selectedScene = selectedFile;
        else
            selectedScene.clear();
        emit CanBeLoadedChanged();
    }
}

QVariant Library::GetFileSystemModel()
{
    DVASSERT(model != nullptr);
    return QtHelpers::toQVariant(ObjectHandle(model));
}
