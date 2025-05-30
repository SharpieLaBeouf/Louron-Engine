#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace Louron {

    namespace LMath 
    {
        constexpr float PI = 3.141592653589793f;

        float Radians(float degrees);
        float Degrees(float radians);

        float Max(float a, float b);

        float Lerp(float t, float a, float b);

        float SCurve(float t);

        float PingPong(float value, float max);

        float DegreesToRadians(float degrees);

        template <typename T>
        T Clamp(const T& value, const T& min, const T& max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        float SimpleNoise2D(int x, int y);

        float PerlinNoise(float x, float y);
    }

}
