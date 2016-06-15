#ifdef DAVA_FMOD

#ifndef __DAVAENGINE_FMODUTILS_H__
#define __DAVAENGINE_FMODUTILS_H__

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include "fmod_event.hpp"
#include "fmod_errors.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace DAVA
{

#define FMOD_VERIFY(command) \
	{ \
	FMOD_RESULT execResult = command; \
	if (execResult != FMOD_OK && execResult != FMOD_ERR_EVENT_FAILED) \
	{ \
		Logger::Error("FMOD: %s file:%s line:%d failed with error: %s", #command, __FILE__, __LINE__, FMOD_ErrorString(execResult)); \
	} \
}
};

#endif //__DAVAENGINE_FMODUTILS_H__

#endif //DAVA_FMOD