#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace Louron {

    namespace LMath 
    {
        constexpr float PI = 3.141592653589793f;

        static float Lerp(float t, float a, float b);

        static float SCurve(float t);

        static float PingPong(float value, float max);

        static float DegreesToRadians(float degrees);

        template <typename T>
        static T Clamp(const T& value, const T& min, const T& max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        static float SimpleNoise2D(int x, int y);

        static float PerlinNoise(float x, float y);
    }

}
