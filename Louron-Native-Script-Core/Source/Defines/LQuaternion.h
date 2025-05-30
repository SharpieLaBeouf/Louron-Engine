#pragma once

#ifdef __linux__
#include <math.h>
#endif

#include "LMath.h"

namespace Louron 
{

    namespace Vectors
    {
        struct Vector3;
    }
    
    struct Quaternion
    {
        float x, y, z, w;

        Quaternion() : x(0), y(0), z(0), w(1) {}
        Quaternion(float scalar) : x(0), y(0), z(0), w(scalar) {}
        Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
        Quaternion Normalise() const { float len = Length(); return len > 0.0f ? (*this) / len : Identity(); }
                
        Quaternion Inverse() const;
        Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }
        
        Vectors::Vector3 ToEuler() const;
        
        static Quaternion Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }
        static Quaternion Normalise(const Quaternion& q) { return q.Normalise(); }
        static Quaternion Euler(const Vectors::Vector3& euler_degrees);
        static Quaternion AxisAngle(const Vectors::Vector3& axis, float angle_degrees);
        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);

        Vectors::Vector3 Rotate(const Vectors::Vector3& v) const;

        Quaternion operator-() const { return Quaternion(-x, -y, -z, -w); }

        Quaternion operator+(const Quaternion& q) const { return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w); }
        Quaternion operator-(const Quaternion& q) const { return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w); }
        Quaternion operator*(float s) const { return Quaternion(x * s, y * s, z * s, w * s); }
        Quaternion operator/(float s) const { return Quaternion(x / s, y / s, z / s, w / s); }

        Quaternion operator*(const Quaternion& q) const;

        bool operator==(const Quaternion& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
        bool operator!=(const Quaternion& q) const { return !(*this == q); }

        std::string ToString() const
        {
            std::ostringstream oss;
            oss << "Quaternion(" << std::to_string(x) << ", " << std::to_string(y)
                << ", " << std::to_string(z) << ", " << std::to_string(w) << ")";
            return oss.str();
        }
    };


}