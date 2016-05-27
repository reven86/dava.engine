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

KeyedArchive* EditorSettings::GetSettings()
{
    return settings;
}

void EditorSettings::Save()
{
    settings->Save(EDITOR_SETTINGS_FILE);
}

void EditorSettings::SetProjectPath(const String& projectPath)
{
    settings->SetString(String("ProjectPath"), projectPath);
    Save();
}

String EditorSettings::GetProjectPath()
{
    return settings->GetString(String("ProjectPath"), String(""));
}

int32 EditorSettings::GetLastOpenedCount()
{
    return settings->GetInt32("LastOpenedFilesCount", 0);
}

String EditorSettings::GetLastOpenedFile(int32 index)
{
    int32 count = GetLastOpenedCount();
    DVASSERT((0 <= index) && (index < count));

    return settings->GetString(Format("LastOpenedFile_%d", index), "");
}

void EditorSettings::AddLastOpenedFile(const String& pathToFile)
{
    Vector<String> filesList;

    // Put all slash symbols to Unix style
    QString normalizedPath = QDir::toNativeSeparators(QString::fromStdString(pathToFile));
    String _pathToFile = normalizedPath.toStdString();

    const int32 count = GetLastOpenedCount();

    for (int32 i = 0; i < count; ++i)
    {
        String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
        if (path == _pathToFile)
        {
            continue;
        }

        filesList.push_back(path);
    }

    filesList.insert(filesList.begin(), _pathToFile);

    if (filesList.size() > RECENT_FILES_COUNT)
    {
        filesList.erase(filesList.begin() + RECENT_FILES_COUNT, filesList.end());
    }

    for (int32 i = 0; i < static_cast<int32>(filesList.size()); ++i)
    {
        settings->SetString(Format("LastOpenedFile_%d", i), filesList[i]);
    }

    settings->SetInt32("LastOpenedFilesCount", static_cast<int32>(filesList.size()));

    Save();
}

void EditorSettings::SetUIEditorVersion(const String& editorVersion)
{
    settings->SetString("editor.version", editorVersion);
    Save();
}

String EditorSettings::GetUIEditorVersion()
{
    return settings->GetString("editor.version");
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
