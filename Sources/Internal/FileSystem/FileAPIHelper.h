#ifndef __DAVAENGINE_FILE_API_HELPER_H__
#define __DAVAENGINE_FILE_API_HELPER_H__

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include "Base/BaseTypes.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{
namespace FileAPI
{

#if defined(__DAVAENGINE_WINDOWS__)

#define NativeStringLiteral(x) L##x

const auto OpenFile = ::_wfopen;
const auto RemoveFile = ::_wremove;
const auto RenameFile = ::_wrename;
struct Stat : public _stat
{
};
const auto FileStat = _wstat;

#else

#define NativeStringLiteral(x) x

const auto OpenFile = ::fopen;
const auto RemoveFile = ::remove;
const auto RenameFile = ::rename;
struct Stat : public stat
{
};
const auto FileStat = stat;

#endif
}
}

#endif //  __DAVAENGINE_FILE_API_HELPER_H__