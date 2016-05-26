#ifndef __DAVAENGINE_CRC_ADDITION_INTERFACE_H__
#define __DAVAENGINE_CRC_ADDITION_INTERFACE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class CRCAdditionInterface
{
public:
    virtual bool AddCRCIntoMetaData(const FilePath& filePathname) const = 0;

    virtual uint32 GetCRCFromFile(const FilePath& filePathname) const = 0;
};
};

#endif // __DAVAENGINE_CRC_ADDITION_INTERFACE_H__