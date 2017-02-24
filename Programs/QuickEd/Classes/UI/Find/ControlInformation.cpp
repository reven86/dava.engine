#include "ControlInformation.h"

using namespace DAVA;

namespace ControlInformationHelpers
{
String GetPathToControl(const ControlInformation* provider)
{
    String pathToParent;
    provider->VisitParent(
    [&pathToParent](const ControlInformation* parent)
    {
        if (parent)
        {
            pathToParent = ControlInformationHelpers::GetPathToControl(parent);
        }
    });

    if (pathToParent.empty())
    {
        return DAVA::String(provider->GetName().c_str());
    }
    else
    {
        return pathToParent + '/' + DAVA::String(provider->GetName().c_str());
    }
}
}
