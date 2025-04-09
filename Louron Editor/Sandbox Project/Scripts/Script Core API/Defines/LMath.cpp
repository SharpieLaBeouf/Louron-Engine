#include "LMath.h"

namespace Louron
{
    namespace LMath
    {
        float Max(float a, float b) { return (a > b) ? a : b; }

        float Lerp(float t, float a, float b) { return a + t * (b - a); }

        float SCurve(float t)
        {
            return t * t * (3.0f - 2.0f * t);  // S-curve function (smoothstep)
        }

        float PingPong(float value, float max)
        {
            float mod_value = fmod(value, max * 2.0f);
            return mod_value < max ? mod_value : (max * 2.0f) - mod_value;
        }

        float DegreesToRadians(float degrees) { return degrees * (PI / 180.0f); }

        float SimpleNoise2D(int x, int y)
        {
            int n = x + y * 257;
            n = (n << 13) ^ n;
            return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        }

        float PerlinNoise(float x, float y)
        {
            int xi, yi;                // integer component
            float xf, yf;              // floating point component
            float xs, ys;              // s-curve function results
            float l00, l01, l10, l11;  // uninterpolated values
            float l0, l1;              // partially interpolated values

            xi = (int)x;
            xf = x - xi;
            xs = SCurve(xf);
            yi = (int)y;
            yf = y - yi;
            ys = SCurve(yf);

            l00 = SimpleNoise2D(xi, yi);
            l01 = SimpleNoise2D(xi + 1, yi);
            l10 = SimpleNoise2D(xi, yi + 1);
            l11 = SimpleNoise2D(xi + 1, yi + 1);

            l0 = Lerp(xs, l00, l01);  // interpolate across the x axis using the s curve
            l1 = Lerp(xs, l10, l11);

            return Lerp(ys, l0, l1);  // interpolate across the y axis using the s curve
        }

    }
}