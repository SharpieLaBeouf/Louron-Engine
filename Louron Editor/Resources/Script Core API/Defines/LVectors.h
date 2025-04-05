#pragma once

#include "LMath.h"

namespace Louron {

    namespace Vectors {

    #pragma region Vector 2

        struct Vector2
        {
            float x, y;
            static Vector2 Zero() { return Vector2(0.0f); }

            Vector2() = default;
            Vector2(float scalar) : x(scalar), y(scalar) {}
            Vector2(float x, float y) : x(x), y(y) {}

            float Length() const { return std::sqrt(x * x + y * y); }
            float LengthSquared() const { return x * x + y * y; }

            Vector2 Normalise() const {
                float len = Length();
                return len > 0 ? (*this) * (1.0f / len) : Zero();
            }

            static Vector2 Normalise(const Vector2& v) { return v.Normalise(); }

            static float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }

            static float Cross(const Vector2& a, const Vector2& b)
            {
                return a.x * b.y - a.y * b.x;
            }

            static float Distance(const Vector2& a, const Vector2& b) {
                float dx = a.x - b.x, dy = a.y - b.y;
                return std::sqrt(dx * dx + dy * dy);
            }

            static Vector2 Lerp(float t, const Vector2& a, const Vector2& b) {
                return Vector2(LMath::Lerp(t, a.x, b.x), LMath::Lerp(t, a.y, b.y));
            }

            static Vector2 Slerp(float t, const Vector2& a, const Vector2& b) {
                Vector2 na = Normalise(a), nb = Normalise(b);
                float dot = LMath::Clamp(Vector2::Dot(na, nb), -1.0f, 1.0f);
                float theta = std::acos(dot) * t;
                Vector2 relVec = Normalise(nb - na * dot);
                return na * std::cos(theta) + relVec * std::sin(theta);
            }

            static Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max)
            {
                return Vector2(
                    std::max(min.x, std::min(value.x, max.x)),
                    std::max(min.y, std::min(value.y, max.y))
                );
            }

            float& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            const float& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
            Vector2 operator+(float s) const { return Vector2(x + s, y + s); }
            Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
            Vector2 operator-(float s) const { return Vector2(x - s, y - s); }
            Vector2 operator*(const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
            Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
            Vector2 operator/(const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
            Vector2 operator/(float s) const { return Vector2(x / s, y / s); }

            bool operator==(const Vector2& v) const { return x == v.x && y == v.y; }
            bool operator!=(const Vector2& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "Vector2(" << std::to_string(x) << ", " << std::to_string(y) << ")";
                return oss.str();
            }
        };

        struct IVector2
        {
            int32_t x, y;
            static IVector2 Zero() { return IVector2(0); }

            IVector2() = default;
            IVector2(int scalar) : x(scalar), y(scalar) {}
            IVector2(int x, int y) : x(x), y(y) {}

            int32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            const int32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            IVector2 operator+(const IVector2& v) const { return IVector2(x + v.x, y + v.y); }
            IVector2 operator-(const IVector2& v) const { return IVector2(x - v.x, y - v.y); }
            IVector2 operator*(const IVector2& v) const { return IVector2(x * v.x, y * v.y); }
            IVector2 operator/(const IVector2& v) const { return IVector2(x / v.x, y / v.y); }
            bool operator==(const IVector2& v) const { return x == v.x && y == v.y; }
            bool operator!=(const IVector2& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "IVector2(" << std::to_string(x) << ", " << std::to_string(y) << ")";
                return oss.str();
            }
        };

        struct UVector2
        {
            uint32_t x, y;
            static UVector2 Zero() { return UVector2(0); }

            UVector2() = default;
            UVector2(uint32_t scalar) : x(scalar), y(scalar) {}
            UVector2(uint32_t x, uint32_t y) : x(x), y(y) {}

            uint32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            const uint32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            UVector2 operator+(const UVector2& v) const { return UVector2(x + v.x, y + v.y); }
            UVector2 operator-(const UVector2& v) const { return UVector2(x - v.x, y - v.y); }
            UVector2 operator*(const UVector2& v) const { return UVector2(x * v.x, y * v.y); }
            UVector2 operator/(const UVector2& v) const { return UVector2(x / v.x, y / v.y); }
            bool operator==(const UVector2& v) const { return x == v.x && y == v.y; }
            bool operator!=(const UVector2& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "UVector2(" << std::to_string(x) << ", " << std::to_string(y) << ")";
                return oss.str();
            }
        };

        struct DVector2
        {
            double x, y;
            static DVector2 Zero() { return DVector2(0.0); }

            DVector2() = default;
            DVector2(double scalar) : x(scalar), y(scalar) {}
            DVector2(double x, double y) : x(x), y(y) {}

            double& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            const double& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            DVector2 operator+(const DVector2& v) const { return DVector2(x + v.x, y + v.y); }
            DVector2 operator-(const DVector2& v) const { return DVector2(x - v.x, y - v.y); }
            DVector2 operator*(const DVector2& v) const { return DVector2(x * v.x, y * v.y); }
            DVector2 operator/(const DVector2& v) const { return DVector2(x / v.x, y / v.y); }
            bool operator==(const DVector2& v) const { return x == v.x && y == v.y; }
            bool operator!=(const DVector2& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "DVector2(" << std::to_string(x) << ", " << std::to_string(y) << ")";
                return oss.str();
            }
        };

        struct BVector2
        {
            bool x, y;
            static BVector2 Zero() { return BVector2(false); }

            BVector2() = default;
            BVector2(bool scalar) : x(scalar), y(scalar) {}
            BVector2(bool x, bool y) : x(x), y(y) {}

            bool& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            const bool& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                }
                throw std::out_of_range("Index out of range");
            }

            bool operator==(const BVector2& v) const { return x == v.x && y == v.y; }
            bool operator!=(const BVector2& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "BVector2(" << (x ? "True" : "False") << ", " << (y ? "True" : "False") << ")";
                return oss.str();
            }
        };

    #pragma endregion

    #pragma region Vector3

        struct Vector3
        {
            float x, y, z;
            static Vector3 Zero() { return Vector3(0.0f); }

            Vector3() = default;
            Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {}
            Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

            float Length() const { return std::sqrt(x * x + y * y + z * z); }
            float LengthSquared() const { return x * x + y * y + z * z; }

            Vector3 Normalise() const {
                float len = Length();
                return len > 0 ? (*this) * (1.0f / len) : Zero();
            }

            static Vector3 Normalise(const Vector3& v) { return v.Normalise(); }

            static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

            static Vector3 Cross(const Vector3& a, const Vector3& b) {
                return Vector3(
                    a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x
                );
            }
            static float Distance(const Vector3& a, const Vector3& b) {
                float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
                return std::sqrt(dx * dx + dy * dy + dz * dz);
            }

            static Vector3 Lerp(float t, const Vector3& a, const Vector3& b) {
                return Vector3(LMath::Lerp(t, a.x, b.x), LMath::Lerp(t, a.y, b.y), LMath::Lerp(t, a.z, b.z));
            }

            static Vector3 Slerp(float t, const Vector3& a, const Vector3& b)
            {
                Vector3 na = Normalise(a), nb = Normalise(b);
                float dot = LMath::Clamp(Vector3::Dot(na, nb), -1.0f, 1.0f);
                float theta = std::acos(dot) * t;
                Vector3 relVec = Normalise(nb - na * dot);
                return na * std::cos(theta) + relVec * std::sin(theta);
            }

            static Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max)
            {
                return Vector3(
                    std::max(min.x, std::min(value.x, max.x)),
                    std::max(min.y, std::min(value.y, max.y)),
                    std::max(min.z, std::min(value.z, max.z))
                );
            }

            float& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            const float& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
            Vector3 operator+(float s) const { return Vector3(x + s, y + s, z + s); }
            Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
            Vector3 operator-(float s) const { return Vector3(x - s, y - s, z - s); }
            Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
            Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
            Vector3 operator/(const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
            Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

            bool operator==(const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const Vector3& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "Vector3(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ")";
                return oss.str();
            }
        };

        struct IVector3
        {
            int32_t x, y, z;
            static IVector3 Zero() { return IVector3(0); }

            IVector3() = default;
            IVector3(int scalar) : x(scalar), y(scalar), z(scalar) {}
            IVector3(int x, int y, int z) : x(x), y(y), z(z) {}

            int32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            const int32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            IVector3 operator+(const IVector3& v) const { return IVector3(x + v.x, y + v.y, z + v.z); }
            IVector3 operator-(const IVector3& v) const { return IVector3(x - v.x, y - v.y, z - v.z); }
            IVector3 operator*(const IVector3& v) const { return IVector3(x * v.x, y * v.y, z * v.z); }
            IVector3 operator/(const IVector3& v) const { return IVector3(x / v.x, y / v.y, z / v.z); }
            bool operator==(const IVector3& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const IVector3& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "IVector3(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ")";
                return oss.str();
            }
        };

        struct UVector3
        {
            uint32_t x, y, z;
            static UVector3 Zero() { return UVector3(0); }

            UVector3() = default;
            UVector3(uint32_t scalar) : x(scalar), y(scalar), z(scalar) {}
            UVector3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}

            uint32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            const uint32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            UVector3 operator+(const UVector3& v) const { return UVector3(x + v.x, y + v.y, z + v.z); }
            UVector3 operator-(const UVector3& v) const { return UVector3(x - v.x, y - v.y, z - v.z); }
            UVector3 operator*(const UVector3& v) const { return UVector3(x * v.x, y * v.y, z * v.z); }
            UVector3 operator/(const UVector3& v) const { return UVector3(x / v.x, y / v.y, z / v.z); }
            bool operator==(const UVector3& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const UVector3& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "UVector3(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ")";
                return oss.str();
            }
        };

        struct DVector3
        {
            double x, y, z;
            static DVector3 Zero() { return DVector3(0.0); }

            DVector3() = default;
            DVector3(double scalar) : x(scalar), y(scalar), z(scalar) {}
            DVector3(double x, double y, double z) : x(x), y(y), z(z) {}

            double& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            const double& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            DVector3 operator+(const DVector3& v) const { return DVector3(x + v.x, y + v.y, z + v.z); }
            DVector3 operator-(const DVector3& v) const { return DVector3(x - v.x, y - v.y, z - v.z); }
            DVector3 operator*(const DVector3& v) const { return DVector3(x * v.x, y * v.y, z * v.z); }
            DVector3 operator/(const DVector3& v) const { return DVector3(x / v.x, y / v.y, z / v.z); }
            bool operator==(const DVector3& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const DVector3& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "DVector3(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ")";
                return oss.str();
            }
        };

        struct BVector3
        {
            bool x, y, z;
            static BVector3 Zero() { return BVector3(false); }

            BVector3() = default;
            BVector3(bool scalar) : x(scalar), y(scalar), z(scalar) {}
            BVector3(bool x, bool y, bool z) : x(x), y(y), z(z) {}

            bool& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            const bool& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                }
                throw std::out_of_range("Index out of range");
            }

            bool operator==(const BVector3& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const BVector3& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "BVector3(" << (x ? "True" : "False") << ", " << (y ? "True" : "False") << ", " << (z ? "True" : "False") << ")";
                return oss.str();
            }
        };


    #pragma endregion

    #pragma region Vector4

        struct Vector4
        {
            float x, y, z, w;
            static Vector4 Zero() { return Vector4(0.0f); }

            Vector4() = default;
            Vector4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

            float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
            float LengthSquared() const { return x * x + y * y + z * z + w * w; }

            Vector4 Normalise() const {
                float len = Length();
                return len > 0 ? (*this) * (1.0f / len) : Zero();
            }

            static Vector4 Normalise(const Vector4& v) { return v.Normalise(); }

            static float Dot(const Vector4& a, const Vector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

            static Vector4 Cross(const Vector4& a, const Vector4& b) {
                return Vector4(
                    a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x,
                    0.0f
                );
            }

            static float Distance(const Vector4& a, const Vector4& b) {
                float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z, dw = a.w - b.w;
                return std::sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
            }

            static Vector4 Lerp(float t, const Vector4& a, const Vector4& b) {
                return Vector4(LMath::Lerp(t, a.x, b.x), LMath::Lerp(t, a.y, b.y), LMath::Lerp(t, a.z, b.z), LMath::Lerp(t, a.w, b.w));
            }

            static Vector4 Slerp(float t, const Vector4& a, const Vector4& b) {
                Vector4 na = Normalise(a), nb = Normalise(b);
                float dot = LMath::Clamp(Vector4::Dot(na, nb), -1.0f, 1.0f);
                float theta = std::acos(dot) * t;
                Vector4 relVec = Normalise(nb - na * dot);
                return na * std::cos(theta) + relVec * std::sin(theta);
            }

            static Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max)
            {
                return Vector4(
                    std::max(min.x, std::min(value.x, max.x)),
                    std::max(min.y, std::min(value.y, max.y)),
                    std::max(min.z, std::min(value.z, max.z)),
                    std::max(min.w, std::min(value.w, max.w))
                );
            }

            float& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            const float& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            Vector4 operator+(const Vector4& v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
            Vector4 operator+(float s) const { return Vector4(x + s, y + s, z + s, w + s); }
            Vector4 operator-(const Vector4& v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
            Vector4 operator-(float s) const { return Vector4(x - s, y - s, z - s, w - s); }
            Vector4 operator*(const Vector4& v) const { return Vector4(x * v.x, y * v.y, z * v.z, w * v.w); }
            Vector4 operator*(float s) const { return Vector4(x * s, y * s, z * s, w * s); }
            Vector4 operator/(const Vector4& v) const { return Vector4(x / v.x, y / v.y, z / v.z, w / v.w); }
            Vector4 operator/(float s) const { return Vector4(x / s, y / s, z / s, w / s); }

            bool operator==(const Vector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const Vector4& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "Vector4(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ", " << std::to_string(w) << ")";
                return oss.str();
            }
        };

        struct IVector4
        {
            int32_t x, y, z, w;
            static IVector4 Zero() { return IVector4(0); }

            IVector4() = default;
            IVector4(int scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            IVector4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}

            int32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            const int32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            IVector4 operator+(const IVector4& v) const { return IVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
            IVector4 operator-(const IVector4& v) const { return IVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
            IVector4 operator*(const IVector4& v) const { return IVector4(x * v.x, y * v.y, z * v.z, w * v.w); }
            IVector4 operator/(const IVector4& v) const { return IVector4(x / v.x, y / v.y, z / v.z, w / v.w); }
            bool operator==(const IVector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const IVector4& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "IVector4(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ", " << std::to_string(w) << ")";
                return oss.str();
            }
        };

        struct UVector4
        {
            uint32_t x, y, z, w;
            static UVector4 Zero() { return UVector4(0); }

            UVector4() = default;
            UVector4(uint32_t scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            UVector4(uint32_t x, uint32_t y, uint32_t z, uint32_t w) : x(x), y(y), z(z), w(w) {}

            uint32_t& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            const uint32_t& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            UVector4 operator+(const UVector4& v) const { return UVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
            UVector4 operator-(const UVector4& v) const { return UVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
            UVector4 operator*(const UVector4& v) const { return UVector4(x * v.x, y * v.y, z * v.z, w * v.w); }
            UVector4 operator/(const UVector4& v) const { return UVector4(x / v.x, y / v.y, z / v.z, w / v.w); }
            bool operator==(const UVector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const UVector4& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "UVector4(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ", " << std::to_string(w) << ")";
                return oss.str();
            }
        };

        struct DVector4
        {
            double x, y, z, w;
            static DVector4 Zero() { return DVector4(0.0); }

            DVector4() = default;
            DVector4(double scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            DVector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

            double& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            const double& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            DVector4 operator+(const DVector4& v) const { return DVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
            DVector4 operator-(const DVector4& v) const { return DVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
            DVector4 operator*(const DVector4& v) const { return DVector4(x * v.x, y * v.y, z * v.z, w * v.w); }
            DVector4 operator/(const DVector4& v) const { return DVector4(x / v.x, y / v.y, z / v.z, w / v.w); }
            bool operator==(const DVector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const DVector4& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "DVector4(" << std::to_string(x) << ", " << std::to_string(y) << ", " << std::to_string(z) << ", " << std::to_string(w) << ")";
                return oss.str();
            }
        };

        struct BVector4
        {
            bool x, y, z, w;
            static BVector4 Zero() { return BVector4(false); }

            BVector4() = default;
            BVector4(bool scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            BVector4(bool x, bool y, bool z, bool w) : x(x), y(y), z(z), w(w) {}

            bool& operator[](int index)
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            const bool& operator[](int index) const
            {
                switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                }
                throw std::out_of_range("Index out of range");
            }

            bool operator==(const BVector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const BVector4& v) const { return !(*this == v); }

            std::string ToString() const
            {
                std::ostringstream oss;
                oss << "BVector4(" << (x ? "True" : "False") << ", " << (y ? "True" : "False") << ", " << (z ? "True" : "False") << ", " << (w ? "True" : "False") << ")";
                return oss.str();
            }
        };

    #pragma endregion

    }

}