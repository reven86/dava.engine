#ifndef __DAVAENGINE_DVASSERT_MESSAGE_H__
#define __DAVAENGINE_DVASSERT_MESSAGE_H__

#include "DAVAConfig.h"
#include "Base/BaseTypes.h"
#include "Functional/Function.h"

namespace DAVA
{
namespace DVAssertMessage
{
// Modality type
enum eModalType
{
    // Try to show non-modal assert message on the platforms where it is applicable.
    TRY_NONMODAL,

    // Always show the assertion message as modal.
    ALWAYS_MODAL
};

#if defined(__DAVAENGINE_COREV2__)
DAVA_DEPRECATED(bool ShowMessage(eModalType modalType, const char8* text, ...));
#else

void SetShowInnerOverride(const Function<bool(eModalType, const char8*)>& fn);
bool ShowMessage(eModalType modalType, const char8* text, ...);
bool IsHidden();

// return true if user click Break
bool InnerShow(eModalType modalType, const char* content);
#endif
};
};

#endif // __DAVAENGINE_DVASSERT_MESSAGE_H__
