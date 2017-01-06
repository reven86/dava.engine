#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FilePath.h"

#include "CommandLineTool.h"

class ArchiveUnpackTool : public CommandLineTool
{
public:
    ArchiveUnpackTool();

private:
    bool ConvertOptionsToParamsInternal() override;
    int ProcessInternal() override;

    int UnpackFile(const DAVA::ResourceArchive& ra, const DAVA::ResourceArchive::FileInfo& fileInfo);

    DAVA::FilePath dstDir;
    DAVA::FilePath packFilename;
};
