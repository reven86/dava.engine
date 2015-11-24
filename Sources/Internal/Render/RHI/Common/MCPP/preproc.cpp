/* preproc.c:   to "pre-preprocess" header files.   */

#include "_mcpp.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

#pragma MCPP preprocess

#include "system.h"
#include "internal.h"

#pragma MCPP put_defines

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
#pragma warning(pop)
#endif
