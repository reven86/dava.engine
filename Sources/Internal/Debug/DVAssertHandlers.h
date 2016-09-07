#pragma once

#include "DVAssert.h"

namespace DAVA
{
namespace Assert
{
/// Part of the message each predefined handler generates
/// Can be used to distinguish assert messages when processing log for some reason
const String AssertMessageTag = "Assert failed";

FailBehaviour LoggerHandler(const AssertInfo& assertInfo);

FailBehaviour DialogBoxHandler(const AssertInfo& assertInfo);
}
}