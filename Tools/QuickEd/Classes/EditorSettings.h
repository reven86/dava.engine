#ifndef UIEditor_EditorSettings_h
#define UIEditor_EditorSettings_h

#include "DAVAEngine.h"

enum eBackgroundType : DAVA::int64
{
    BackgroundTexture,
    BackgroundColor
};

class EditorSettings : public DAVA::Singleton<EditorSettings>
{
public:
    enum eDefaultSettings
    {
        RECENT_FILES_COUNT = 5,
    };

public:
    EditorSettings();
    virtual ~EditorSettings();

    DAVA::KeyedArchive* GetSettings();
    void Save();

    void SetProjectPath(const DAVA::String& projectPath);
    DAVA::String GetProjectPath();

    DAVA::int32 GetLastOpenedCount();
    DAVA::String GetLastOpenedFile(DAVA::int32 index);
    void AddLastOpenedFile(const DAVA::String& pathToFile);

    void SetUIEditorVersion(const DAVA::String& editorVersion);
    DAVA::String GetUIEditorVersion();

    // Whether "Pixelized" or "Smooth" images representation is used.
    void SetPixelized(bool value);
    bool IsPixelized() const;

    DAVA::Color GetGrigColor() const;
    void SetGrigColor(const DAVA::Color& color);
    DAVA::Signal<const DAVA::Color&> GridColorChanged;

    eBackgroundType GetGridType() const;
    void SetGridType(eBackgroundType type);
    DAVA::Signal<eBackgroundType> GridTypeChanged;

    bool IsUsingAssetCache() const;
    DAVA::String GetAssetCacheIp() const;
    DAVA::uint32 GetAssetCachePort() const;
    DAVA::uint64 GetAssetCacheTimeoutSec() const;

protected:
    DAVA::Color GetColor(const DAVA::String& colorName, const DAVA::Color& defaultColor) const;
    void SetColor(const DAVA::String& colorName, const DAVA::Color& color);

private:
    using HashType = size_t;

    DAVA::KeyedArchive* settings;
};

#endif //UIEditor_EditorSettings_h
