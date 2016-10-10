#pragma once

#include "DVAssert.h"

namespace DAVA
{
namespace Assert
{
/**
Part of a message each predefined handler generates
Can be used to distinguish assert messages when processing log for some reason
*/
extern const String AssertMessageTag;

/** Log assert info as an error and return FailBehaviour::Default */
FailBehaviour DefaultLoggerHandler(const AssertInfo& assertInfo);

/** Show message box with assert info asking user if program should be halted. Return FailBehaviour::Halt if it should */
FailBehaviour DefaultDialogBoxHandler(const AssertInfo& assertInfo);
}
}