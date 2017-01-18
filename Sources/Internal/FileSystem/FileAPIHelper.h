#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace FileAPI
{
FILE* OpenFile(const char* fileName, const char* mode);
int32 RemoveFile(const char* fileName);
int32 RenameFile(const char* oldFileName, const char* newFileName);

bool IsRegularFile(const char* fileName);
}
}
