#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include <fmod_event.hpp>
#include <fmod_errors.h>

#include "Logger/Logger.h"

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
