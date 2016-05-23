#ifndef __RULERSETTINGS__H__
#define __RULERSETTINGS__H__

// Settings for the ruler widget.
struct RulerSettings
{
    int startPos = 0; // Start ruler position.
    int smallTicksDelta = 1; // Distance between "small" ticks.
    int bigTicksDelta = 10; // Distance between "big" ticks.

    float zoomLevel = 1.0f; // Zoom level for the current control.
};

#endif /* defined(__RULERSETTINGS__H__) */
