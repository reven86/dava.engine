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


#include "EditorSettings.h"
#include "AssetCache/AssetCacheConstants.h"
#include <QString>
#include <QDir>

using namespace DAVA;

static const Color DEFAULT_BACKGROUND_FRAME_COLOR(0.2f, 0.2f, 0.2f, 1.0f);
static const Color DEFAULT_GRID_COLOR(0.5f, 0.5f, 0.5f, 1.0f);
static const String EDITOR_SETTINGS_FILE("~doc:/QuickEdSettings.archive");

EditorSettings::EditorSettings()
{
    settings = new KeyedArchive();
    settings->Load(EDITOR_SETTINGS_FILE);
}

EditorSettings::~EditorSettings()
{
    Save();
    SafeRelease(settings);
}

void EditorSettings::Save()
{
    settings->Save(EDITOR_SETTINGS_FILE);
}

void EditorSettings::SetPixelized(bool value)
{
    settings->SetBool("editor.pixelized", value);
    Save();
}

bool EditorSettings::IsPixelized() const
{
    return settings->GetBool("editor.pixelized", true);
}

Color EditorSettings::GetGrigColor() const
{
    return GetColor("editor.gridColor", DEFAULT_BACKGROUND_FRAME_COLOR);
}

void EditorSettings::SetGrigColor(const Color& color)
{
    SetColor("editor.gridColor", color);
    GridColorChanged.Emit(color);
}

eBackgroundType EditorSettings::GetGridType() const
{
    int64 value = settings->GetInt64("editor.gridType");
    return static_cast<eBackgroundType>(value);
}

void EditorSettings::SetGridType(eBackgroundType type)
{
    settings->SetInt64("editor.gridType", type);
    GridTypeChanged.Emit(type);
}

Color EditorSettings::GetColor(const String& colorName, const Color& defaultColor) const
{
    Vector4 defaultValue(defaultColor.r, defaultColor.g, defaultColor.b, defaultColor.a);
    Vector4 colorValues = settings->GetVector4(colorName, defaultValue);
    return Color(colorValues);
}

void EditorSettings::SetColor(const String& colorName, const Color& color)
{
    Vector4 colorValues(color.r, color.g, color.b, color.a);
    settings->SetVector4(colorName, colorValues);
    Save();
}

bool EditorSettings::IsUsingAssetCache() const
{
    return settings->GetBool("editor.usingAssetCache", false);
}

String EditorSettings::GetAssetCacheIp() const
{
    return settings->GetString("editor.assetCacheIp", DAVA::AssetCache::LOCALHOST);
}

uint32 EditorSettings::GetAssetCachePort() const
{
    return settings->GetUInt32("editor.assetCachePort", DAVA::AssetCache::ASSET_SERVER_PORT);
}

uint64 EditorSettings::GetAssetCacheTimeoutSec() const
{
    return settings->GetUInt64("editor.assetCacheTimeoutSec", 1);
}
