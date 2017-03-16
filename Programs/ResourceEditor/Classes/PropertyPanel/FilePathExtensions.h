#pragma once

#include <TArc/Core/ContextAccessor.h>
#include <Reflection/ReflectedMeta.h>

DAVA::M::Validator CreateHeightMapValidator(DAVA::TArc::ContextAccessor* accessor);
DAVA::M::Validator CreateTextureValidator(DAVA::TArc::ContextAccessor* accessor);
DAVA::M::Validator CreateImageValidator(DAVA::TArc::ContextAccessor* accessor);
DAVA::M::Validator CreateSceneValidator(DAVA::TArc::ContextAccessor* accessor);

class REFileMeta : public DAVA::Metas::File
{
public:
    REFileMeta(const DAVA::String& filters, const DAVA::String& dlgTitle);

    DAVA::String GetDefaultPath() const override;
    DAVA::String GetRootDirectory() const override;
};

class HeightMapFileMeta : public REFileMeta
{
public:
    HeightMapFileMeta(const DAVA::String& filters);
};

class TextureFileMeta : public REFileMeta
{
public:
    TextureFileMeta(const DAVA::String& filters);
};

class ImageFileMeta : public REFileMeta
{
public:
    ImageFileMeta(const DAVA::String& filters);
};

class SceneFileMeta : public REFileMeta
{
public:
    SceneFileMeta(const DAVA::String& filters);
};

DAVA::Meta<HeightMapFileMeta, DAVA::Metas::File> CreateHeightMapFileMeta(DAVA::TArc::ContextAccessor* accessor);
DAVA::Meta<TextureFileMeta, DAVA::Metas::File> CreateTextureFileMeta(DAVA::TArc::ContextAccessor* accessor);
DAVA::Meta<ImageFileMeta, DAVA::Metas::File> CreateImageFileMeta(DAVA::TArc::ContextAccessor* accessor);
DAVA::Meta<SceneFileMeta, DAVA::Metas::File> CreateSceneFileMeta(DAVA::TArc::ContextAccessor* accessor);
