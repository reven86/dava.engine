#ifndef __COLLADA_CONVERT_H__
#define __COLLADA_CONVERT_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "ColladaErrorCodes.h"
#include "ImportParams.h"

eColladaErrorCodes ConvertDaeToSc2(const DAVA::FilePath& pathToFile, std::unique_ptr<DAEConverter::ImportParams>&& importParams);

#endif // __COLLADA_CONVERT_H__