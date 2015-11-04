/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "WaypointsInterpolator.h"

float32 WaypointsInterpolator::SPLINE_DELTA_TIME = 0.00001f;

WaypointsInterpolator::WaypointsInterpolator(const Vector<PathComponent::Waypoint*>& _waypoints, float32 _time)
    :   waypoints(_waypoints)
    ,   segment(0)
    ,   segmentTime(0.0f)
    ,   targetSegmentTime(0.0f)
    ,   splineTime(_time)
    ,   splineLength(0.0f)
{
    Init();
}

void WaypointsInterpolator::Init()
{
    DVASSERT(waypoints.size() > 3);

    spline = std::unique_ptr<BasicSpline3>(new BasicSpline3());
    Polygon3 poly;

    for (auto *point : waypoints)
    {
        poly.AddPoint(point->position);
    }

    spline->Construct(poly);

    for (uint32 i = 0; i < spline->pointCount - 1; i++)
    {
        float32 t = 0.0f;
        float32 length = 0.0f;

        Vector3 prev = spline->Evaluate(i, t);

        while (t <= 1.0f)
        {
            const Vector3& current = spline->Evaluate(i, t);
            length += (current - prev).Length();

            prev = current;
            t += SPLINE_DELTA_TIME;
        }

        splineLength += length;
        segmentsLength.push_back(length);
    }

    for (uint32 i = 0; i < segmentsLength.size(); i++)
    {
        segmentsLength[i] = segmentsLength[i] / splineLength * splineTime;
    }

    currentPosition = waypoints[0]->position;
    targetPosition = waypoints[1]->position;

    targetSegmentTime = segmentsLength[0];
}

void WaypointsInterpolator::NextPosition(Vector3& position, Vector3& target, float32 timeElapsed)
{
    position = currentPosition;
    target = targetPosition;
  
    segmentTime += timeElapsed;

    if (segmentTime >= targetSegmentTime)
    {
        segment = (segment == spline->pointCount - 2) ? 0 : segment + 1;
        segmentTime -= targetSegmentTime;
        targetSegmentTime = segmentsLength[segment];
    }

    Vector3 tangent = spline->EvaluateDerivative(segment, segmentTime / targetSegmentTime);
    tangent.Normalize();

    currentPosition = spline->Evaluate(segment, segmentTime / targetSegmentTime);
    targetPosition = currentPosition + tangent;
}