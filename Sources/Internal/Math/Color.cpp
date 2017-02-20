#include "Color.h"

namespace DAVA
{
const Color Color::White(1.f, 1.f, 1.f, 1.f);
const Color Color::Transparent(1.f, 1.f, 1.f, 0.f);
const Color Color::Clear(0.f, 0.f, 0.f, 0.f);
const Color Color::Black(0.f, 0.f, 0.f, 1.f);

Color ClampToUnityRange(Color color)
{
    color.r = DAVA::Clamp(0.f, 1.f, color.r);
    color.g = DAVA::Clamp(0.f, 1.f, color.g);
    color.b = DAVA::Clamp(0.f, 1.f, color.b);
    color.a = DAVA::Clamp(0.f, 1.f, color.a);

    return color;
}

Color MakeGrayScale(const Color& rgb)
{
    // http://www.tannerhelland.com/3643/grayscale-image-algorithm-vb6/

    float32 channel = rgb.r * 0.3f + rgb.g * 0.59f + rgb.b * 0.11f;
    return Color(channel, channel, channel, rgb.a);
}

template <>
bool AnyCompare<Color>::IsEqual(const Any& v1, const Any& v2)
{
    const Color& c1 = v1.Get<Color>();
    const Color& c2 = v2.Get<Color>();
    return c1 == c2;
}
}