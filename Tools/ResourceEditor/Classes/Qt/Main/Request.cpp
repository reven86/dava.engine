#include "Request.h"

Request::Request()
    : isAccepted(true)
{
}

void Request::Accept()
{
    isAccepted = true;
}

void Request::Cancel()
{
    isAccepted = false;
}

bool Request::IsAccepted() const
{
    return isAccepted;
}