#include "DAEConverter.h"

#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"

#include "Collada/ColladaConvert.h"

namespace DAEConverter
{
bool Convert(const DAVA::FilePath& daePath)
{
    if (DAVA::FileSystem::Instance()->Exists(daePath) && daePath.IsEqualToExtension(".dae"))
    {
        eColladaErrorCodes code = ConvertDaeToSc2(daePath);
        if (code == COLLADA_OK)
        {
            return true;
        }
        else if (code == COLLADA_ERROR_OF_ROOT_NODE)
        {
            DAVA::Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
        }
        else
        {
            DAVA::Logger::Error("[DAE to SC2] Can't convert from DAE.");
        }
    }
    else
    {
        DAVA::Logger::Error("[DAE to SC2] Wrong pathname: %s.", daePath.GetStringValue().c_str());
    }

    return false;
}
}
