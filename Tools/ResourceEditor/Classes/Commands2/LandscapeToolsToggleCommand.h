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


#ifndef __LANDSCAPETOOLSTOGGLECOMMAND_H__
#define __LANDSCAPETOOLSTOGGLECOMMAND_H__

#include "Functional/Function.h"
#include "Commands2/Command2.h"
#include "Qt/Scene/System/LandscapeEditorDrawSystem.h"

class SceneEditor2;

class LandscapeToolsToggleCommand : public Command2
{
public:
    LandscapeToolsToggleCommand(DAVA::int32 identifier, SceneEditor2* sceneEditor,
                                DAVA::uint32 allowedTools, DAVA::String disablingError);
    DAVA::Entity* GetEntity() const override;

    void Redo() override;
    void Undo() override;

    void SaveEnabledToolsState();
    void ApplySavedState();

    using IsEnabledFunction = DAVA::Function<bool()>;
    using EnableFunction = DAVA::Function<LandscapeEditorDrawSystem::eErrorType()>;
    using DisableFunction = DAVA::Function<bool()>;

protected:
    virtual void OnEnabled();
    virtual void OnDisabled();

protected:
    SceneEditor2* sceneEditor = nullptr;
    DAVA::String disablingError;
    DAVA::uint32 allowedTools = 0;
    DAVA::int32 enabledTools = 0;
    IsEnabledFunction isEnabledFunction;
    EnableFunction enableFunction;
    DisableFunction disableFunction;
};

template <typename ForwardCommand>
class LandscapeToolsReverseCommand : public ForwardCommand
{
public:
    template <typename... Args>
    LandscapeToolsReverseCommand(SceneEditor2* sceneEditor, Args... a)
        : ForwardCommand(sceneEditor, a...)
    {
    }

    inline void Redo() override
    {
        ForwardCommand::Undo();
    }

    inline void Undo() override
    {
        ForwardCommand::Redo();
    }
};

/*
 * Concerete commands
 */
class EnableHeightmapEditorCommand : public LandscapeToolsToggleCommand
{
public:
    EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor);

private:
    void OnDisabled() override;
};
using DisableHeightmapEditorCommand = LandscapeToolsReverseCommand<EnableHeightmapEditorCommand>;

class EnableNotPassableCommand : public LandscapeToolsToggleCommand
{
public:
    EnableNotPassableCommand(SceneEditor2* forSceneEditor);
};
using DisableNotPassableCommand = LandscapeToolsReverseCommand<EnableNotPassableCommand>;

class EnableRulerToolCommand : public LandscapeToolsToggleCommand
{
public:
    EnableRulerToolCommand(SceneEditor2* forSceneEditor);
};
using DisableRulerToolCommand = LandscapeToolsReverseCommand<EnableRulerToolCommand>;

class EnableTilemaskEditorCommand : public LandscapeToolsToggleCommand
{
public:
    EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor);
};
using DisableTilemaskEditorCommand = LandscapeToolsReverseCommand<EnableTilemaskEditorCommand>;

class EnableCustomColorsCommand : public LandscapeToolsToggleCommand
{
public:
    EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool saveChanges);

private:
    void OnEnabled() override;

private:
    bool saveChanges = false;
};
using DisableCustomColorsCommand = LandscapeToolsReverseCommand<EnableCustomColorsCommand>;

#endif // __LANDSCAPETOOLSTOGGLECOMMAND_H__
