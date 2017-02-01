#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace FileAPI
{
/**
	fileName - utf8 string
*/
FILE* OpenFile(const String& fileName, const String& mode);
/**
	fileName - utf8 string
*/
int32 RemoveFile(const String& fileName);
/**
	oldfileName - utf8 string
	newFileName - utf8 string
*/
int32 RenameFile(const String& oldFileName, const String& newFileName);

/**
	fileName - utf8 string
*/
bool IsRegularFile(const String& fileName);

/**
	dirName - utf8 string
*/
bool IsDirectory(const String& dirName);
}
}
