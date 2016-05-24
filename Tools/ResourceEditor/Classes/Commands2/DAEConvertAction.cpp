#include "DAEConvertAction.h"
#include "Collada/ColladaConvert.h"

#include "Deprecated/SceneValidator.h"

#include "Scene/SceneHelper.h"
#include "Commands2/ConvertToShadowCommand.h"

using namespace DAVA;

DAEConvertAction::DAEConvertAction(const DAVA::FilePath& path)
    : CommandAction(CMDID_DAE_CONVERT, "DAE to SC2 Convert")
    , daePath(path)
{
}

void DAEConvertAction::Redo()
{
    if (FileSystem::Instance()->Exists(daePath) && daePath.IsEqualToExtension(".dae"))
    {
        eColladaErrorCodes code = ConvertDaeToSc2(daePath);
        if (code == COLLADA_OK)
        {
            return;
        }
        else if (code == COLLADA_ERROR_OF_ROOT_NODE)
        {
            Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
        }
        else
        {
            Logger::Error("[DAE to SC2] Can't convert from DAE.");
        }
    }
}
