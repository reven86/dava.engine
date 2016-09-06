#pragma once

#include "DVAssert.h"

namespace DAVA
{
namespace Assert
{

FailBehaviour LoggerHandler(const AssertInfo& assertInfo);

FailBehaviour DialogBoxHandler(const AssertInfo& assertInfo);

}
}