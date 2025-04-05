#pragma once

#include "LMath.h"
#include "LVectors.h"

namespace Louron 
{
    namespace Vectors
    {
        struct Vector2;
        struct Vector3;
        struct Vector4;
    }

	namespace Matrices
	{
		struct Mat3;
		struct Mat4;

    #pragma region Mat3

        struct Mat3
        {
            union {
                float data[3 * 3];
                Vectors::Vector3 columns[3]; // Column-major layout
            };

            // Constructors
            Mat3();
            Mat3(float diagonal);
            Mat3(const Vectors::Vector3& col0, const Vectors::Vector3& col1, const Vectors::Vector3& col2);

            // Factory
            static Mat3 Identity();

            // Operators
            Vectors::Vector3& operator[](int index);
            const Vectors::Vector3& operator[](int index) const;

            Mat3 operator*(const Mat3& other) const;
            Vectors::Vector3 operator*(const Vectors::Vector3& v) const;

            bool operator==(const Mat3& other) const;
            bool operator!=(const Mat3& other) const;

            // Core operations
            Mat3 Transposed() const;
            float Determinant() const;
            Mat3 Inversed() const;
            Mat3 TransposeInverse() const;

            // Transform matrices
            static Mat3 Rotation(float angleRadians);              // 2D rotation
            static Mat3 Scale(const Vectors::Vector2& scale);               // 2D scale
            static Mat3 Translation(const Vectors::Vector2& translation);   // 2D translation

            // Extracted from Mat4
            static Mat3 FromMat4(const Mat4& mat4);

            // Row access
            Vectors::Vector3 GetRow(int index) const;
            void SetRow(int index, const Vectors::Vector3& row);

            const float* Data() const { return data; }
            float* Data() { return data; }

        };

    #pragma endregion

    #pragma region Mat4

	    struct Mat4
	    {
            union {
                float data[4 * 4];     // Column-major: data[column * 4 + row]
                Vectors::Vector4 columns[4];    // columns[0] = first column
            };

            // Constructors
            Mat4();
            Mat4(float diagonal);
            Mat4(const Vectors::Vector4& col0, const Vectors::Vector4& col1, const Vectors::Vector4& col2, const Vectors::Vector4& col3);

            // Factory
            static Mat4 Identity();

            // Operators
            Vectors::Vector4& operator[](int index);
            const Vectors::Vector4& operator[](int index) const;

            Mat4 operator*(const Mat4& other) const;
            Vectors::Vector4 operator*(const Vectors::Vector4& v) const;

            bool operator==(const Mat4& other) const;
            bool operator!=(const Mat4& other) const;

            // Core operations
            Mat4 Transposed() const;
            Mat4 Inversed() const;
            float Determinant() const;
            Mat4 TransposeInverse() const;

            // Transform matrix helpers
            static Mat4 Translation(const Vectors::Vector3& translation);
            static Mat4 Scale(const Vectors::Vector3& scale);
            static Mat4 Rotation(float angleRadians, const Vectors::Vector3& axis);

            // Projection matrices
            static Mat4 Perspective(float fovRadians, float aspect, float near, float far);
            static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);

            // Camera transform
            static Mat4 LookAt(const Vectors::Vector3& eye, const Vectors::Vector3& center, const Vectors::Vector3& up);

            // Normal matrix (upper-left 3x3 of transpose-inverse)
            Mat3 NormalMatrix() const;

            // Raw pointer access
            const float* Data() const { return data; }
            float* Data() { return data; }

            // Row access
            Vectors::Vector4 GetRow(int index) const;
            void SetRow(int index, const Vectors::Vector4& row);

	    };

    #pragma endregion

    }
}