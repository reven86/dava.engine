#include "ColladaConvert.h"
#include "ColladaDocument.h"
#include "Collada/ColladaToSc2Importer/ColladaToSc2Importer.h"
#include "Classes/Collada/ImportParams.h"

eColladaErrorCodes ConvertDaeToSc2(const DAVA::FilePath& pathToFile, std::unique_ptr<DAEConverter::ImportParams>&& importParams)
{
    FCollada::Initialize();

    DAVA::ColladaDocument colladaDocument;

    eColladaErrorCodes code = colladaDocument.Open(pathToFile.GetAbsolutePathname().c_str());
    if (code != COLLADA_OK)
    {
        DAVA::Logger::Error("[ConvertDaeToSc2] Failed to read %s with error %d", pathToFile.GetAbsolutePathname().c_str(), (int32)code);
        return code;
    }

    DAVA::FilePath pathSc2 = DAVA::FilePath::CreateWithNewExtension(pathToFile, ".sc2");

    eColladaErrorCodes ret = colladaDocument.SaveSC2(pathSc2, std::move(importParams));
    colladaDocument.Close();

    FCollada::Release();

    return ret;
}
