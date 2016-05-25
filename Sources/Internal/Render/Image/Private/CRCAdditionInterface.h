#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class CRCAdditionInterface
{
public:
    virtual ~CRCAdditionInterface() = default;

    virtual bool AddCRCIntoMetaData(const FilePath& filePathname) const = 0;
    virtual uint32 GetCRCFromMetaData(const FilePath& filePathname) const = 0;
};
};
