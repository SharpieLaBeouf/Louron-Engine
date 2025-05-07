#include "LMatrices.h"

#include "LVectors.h"

namespace Louron {

	using namespace Vectors;

    namespace Matrices
    {

        #pragma region Mat3

        Mat3::Mat3() {
            *this = Identity();
        }

        Mat3::Mat3(float diagonal) {
            *this = Identity();
            columns[0].x = diagonal;
            columns[1].y = diagonal;
            columns[2].z = diagonal;
        }

        Mat3::Mat3(const Vector3& col0, const Vector3& col1, const Vector3& col2) {
            columns[0] = col0;
            columns[1] = col1;
            columns[2] = col2;
        }

        Mat3 Mat3::Identity() {
            return Mat3(
                Vector3(1, 0, 0),
                Vector3(0, 1, 0),
                Vector3(0, 0, 1)
            );
        }

        Vector3& Mat3::operator[](int index) {
            switch (index)
            {
            case 0: return columns[0];
            case 1: return columns[1];
            case 2: return columns[2];
            }
            throw std::out_of_range("Index out of range");
        }

        const Vector3& Mat3::operator[](int index) const {
            switch (index)
            {
            case 0: return columns[0];
            case 1: return columns[1];
            case 2: return columns[2];
            }
            throw std::out_of_range("Index out of range");
        }

        Mat3 Mat3::operator*(const Mat3& other) const {
            Mat3 result;
            for (int i = 0; i < 3; ++i) {
                result[i] = *this * other[i];
            }
            return result;
        }

        Vector3 Mat3::operator*(const Vector3& v) const {
            return Vector3(
                columns[0].x * v.x + columns[1].x * v.y + columns[2].x * v.z,
                columns[0].y * v.x + columns[1].y * v.y + columns[2].y * v.z,
                columns[0].z * v.x + columns[1].z * v.y + columns[2].z * v.z
            );
        }

        bool Mat3::operator==(const Mat3& other) const {
            for (int i = 0; i < 9; ++i)
                if (data[i] != other.data[i]) return false;
            return true;
        }

        bool Mat3::operator!=(const Mat3& other) const {
            return !(*this == other);
        }

        Mat3 Mat3::Transposed() const {
            return Mat3(
                Vector3(columns[0].x, columns[1].x, columns[2].x),
                Vector3(columns[0].y, columns[1].y, columns[2].y),
                Vector3(columns[0].z, columns[1].z, columns[2].z)
            );
        }

        float Mat3::Determinant() const {
            return
                columns[0].x * (columns[1].y * columns[2].z - columns[1].z * columns[2].y) -
                columns[1].x * (columns[0].y * columns[2].z - columns[0].z * columns[2].y) +
                columns[2].x * (columns[0].y * columns[1].z - columns[0].z * columns[1].y);
        }

        Mat3 Mat3::Inversed() const {
            float det = Determinant();
            if (det == 0.0f) return Mat3(0.0f);
            float invDet = 1.0f / det;

            const Vector3& a = columns[0];
            const Vector3& b = columns[1];
            const Vector3& c = columns[2];

            Mat3 result;
            result[0].x = (b.y * c.z - b.z * c.y) * invDet;
            result[0].y = (a.z * c.y - a.y * c.z) * invDet;
            result[0].z = (a.y * b.z - a.z * b.y) * invDet;

            result[1].x = (b.z * c.x - b.x * c.z) * invDet;
            result[1].y = (a.x * c.z - a.z * c.x) * invDet;
            result[1].z = (a.z * b.x - a.x * b.z) * invDet;

            result[2].x = (b.x * c.y - b.y * c.x) * invDet;
            result[2].y = (a.y * c.x - a.x * c.y) * invDet;
            result[2].z = (a.x * b.y - a.y * b.x) * invDet;

            return result.Transposed(); // Cofactor matrix is transposed
        }

        Mat3 Mat3::TransposeInverse() const {
            return Inversed().Transposed();
        }

        Mat3 Mat3::Rotation(float angle) {
            float c = cos(angle);
            float s = sin(angle);
            return Mat3(
                Vector3(c, s, 0.0f),
                Vector3(-s, c, 0.0f),
                Vector3(0.0f, 0.0f, 1.0f)
            );
        }

        Mat3 Mat3::Scale(const Vector2& scale) {
            return Mat3(
                Vector3(scale.x, 0.0f, 0.0f),
                Vector3(0.0f, scale.y, 0.0f),
                Vector3(0.0f, 0.0f, 1.0f)
            );
        }

        Mat3 Mat3::Translation(const Vector2& translation) {
            return Mat3(
                Vector3(1.0f, 0.0f, 0.0f),
                Vector3(0.0f, 1.0f, 0.0f),
                Vector3(translation.x, translation.y, 1.0f)
            );
        }

        Mat3 Mat3::FromMat4(const Mat4& m) {
            return Mat3(
                Vector3(m[0].x, m[0].y, m[0].z),
                Vector3(m[1].x, m[1].y, m[1].z),
                Vector3(m[2].x, m[2].y, m[2].z)
            );
        }

        Vector3 Mat3::GetRow(int index) const {
            switch (index)
            {
            case 0: return Vector3(columns[index]);
            case 1: return Vector3(columns[index]);
            case 2: return Vector3(columns[index]);
            }
            throw std::out_of_range("Index out of range");
        }

        void Mat3::SetRow(int index, const Vector3& row) {
            switch (index)
            {
            case 0: columns[index] = row;
            case 1: columns[index] = row;
            case 2: columns[index] = row;
            default: throw std::out_of_range("Index out of range");
            }
        }


#pragma endregion

        #pragma region Mat4

        Mat4::Mat4() {
            *this = Identity();
        }

        Mat4::Mat4(float diagonal) {
            *this = Identity();
            columns[0].x = diagonal;
            columns[1].y = diagonal;
            columns[2].z = diagonal;
            columns[3].w = diagonal;
        }

        Mat4::Mat4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3) {
            columns[0] = col0;
            columns[1] = col1;
            columns[2] = col2;
            columns[3] = col3;
        }

        Mat4 Mat4::Identity() {
            return Mat4(
                Vector4(1, 0, 0, 0),
                Vector4(0, 1, 0, 0),
                Vector4(0, 0, 1, 0),
                Vector4(0, 0, 0, 1)
            );
        }

        Vector4& Mat4::operator[](int index)
        {
            switch (index)
            {
            case 0: return columns[index];
            case 1: return columns[index];
            case 2: return columns[index];
            case 3: return columns[index];
            }
            throw std::out_of_range("Index out of range");
        }

        const Vector4& Mat4::operator[](int index) const
        {
            switch (index)
            {
            case 0: return columns[index];
            case 1: return columns[index];
            case 2: return columns[index];
            case 3: return columns[index];
            }
            throw std::out_of_range("Index out of range");
        }

        Mat4 Mat4::operator*(const Mat4& other) const {
            Mat4 result;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    result[j][i] = columns[0][i] * other[j].x +
                    columns[1][i] * other[j].y +
                    columns[2][i] * other[j].z +
                    columns[3][i] * other[j].w;
            return result;
        }

        Vector4 Mat4::operator*(const Vector4& v) const {
            return Vector4(
                columns[0].x * v.x + columns[1].x * v.y + columns[2].x * v.z + columns[3].x * v.w,
                columns[0].y * v.x + columns[1].y * v.y + columns[2].y * v.z + columns[3].y * v.w,
                columns[0].z * v.x + columns[1].z * v.y + columns[2].z * v.z + columns[3].z * v.w,
                columns[0].w * v.x + columns[1].w * v.y + columns[2].w * v.z + columns[3].w * v.w
            );
        }

        bool Mat4::operator==(const Mat4& other) const {
            for (int i = 0; i < 16; ++i)
                if (data[i] != other.data[i]) return false;
            return true;
        }

        bool Mat4::operator!=(const Mat4& other) const {
            return !(*this == other);
        }

        Mat4 Mat4::Transposed() const {
            Mat4 result;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    result[i][j] = columns[j][i];
            return result;
        }

        float Mat4::Determinant() const {
            const float* m = data;
            float subFactor00 = m[10] * m[15] - m[14] * m[11];
            float subFactor01 = m[9] * m[15] - m[13] * m[11];
            float subFactor02 = m[9] * m[14] - m[13] * m[10];
            float subFactor03 = m[8] * m[15] - m[12] * m[11];
            float subFactor04 = m[8] * m[14] - m[12] * m[10];
            float subFactor05 = m[8] * m[13] - m[12] * m[9];

            return
                m[0] * (m[5] * subFactor00 - m[6] * subFactor01 + m[7] * subFactor02) -
                m[1] * (m[4] * subFactor00 - m[6] * subFactor03 + m[7] * subFactor04) +
                m[2] * (m[4] * subFactor01 - m[5] * subFactor03 + m[7] * subFactor05) -
                m[3] * (m[4] * subFactor02 - m[5] * subFactor04 + m[6] * subFactor05);
        }

        Mat4 Mat4::Inversed() const {
            const float* m = data;
            Mat4 inv;

            inv[0][0] = m[5] * m[10] * m[15] -
                m[5] * m[11] * m[14] -
                m[9] * m[6] * m[15] +
                m[9] * m[7] * m[14] +
                m[13] * m[6] * m[11] -
                m[13] * m[7] * m[10];

            inv[0][1] = -m[1] * m[10] * m[15] +
                m[1] * m[11] * m[14] +
                m[9] * m[2] * m[15] -
                m[9] * m[3] * m[14] -
                m[13] * m[2] * m[11] +
                m[13] * m[3] * m[10];

            inv[0][2] = m[1] * m[6] * m[15] -
                m[1] * m[7] * m[14] -
                m[5] * m[2] * m[15] +
                m[5] * m[3] * m[14] +
                m[13] * m[2] * m[7] -
                m[13] * m[3] * m[6];

            inv[0][3] = -m[1] * m[6] * m[11] +
                m[1] * m[7] * m[10] +
                m[5] * m[2] * m[11] -
                m[5] * m[3] * m[10] -
                m[9] * m[2] * m[7] +
                m[9] * m[3] * m[6];

            // ... you would continue computing the rest of the matrix ...

            float det = m[0] * inv[0][0] + m[1] * inv[0][1] + m[2] * inv[0][2] + m[3] * inv[0][3];

            if (det == 0.0f)
                return Mat4(0.0f);

            float invDet = 1.0f / det;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    inv[i][j] *= invDet;

            return inv;
        }

        Mat4 Mat4::TransposeInverse() const {
            return Inversed().Transposed();
        }

        Mat4 Mat4::Translation(const Vector3& t) {
            Mat4 result = Identity();
            result[3].x = t.x;
            result[3].y = t.y;
            result[3].z = t.z;
            return result;
        }

        Mat4 Mat4::Scale(const Vector3& s) {
            Mat4 result = Identity();
            result[0][0] = s.x;
            result[1][1] = s.y;
            result[2][2] = s.z;
            return result;
        }

        Mat4 Mat4::Rotation(float angle, const Vector3& axis)
        {
            float c = cos(angle);
            float s = sin(angle);
            float oneMinusC = 1.0f - c;
            Vector3 a = Vector3::Normalise(axis);

            Mat4 result;
            result[0] = Vector4(c + a.x * a.x * oneMinusC, a.x * a.y * oneMinusC + a.z * s, a.x * a.z * oneMinusC - a.y * s, 0.0f);
            result[1] = Vector4(a.x * a.y * oneMinusC - a.z * s, c + a.y * a.y * oneMinusC, a.y * a.z * oneMinusC + a.x * s, 0.0f);
            result[2] = Vector4(a.x * a.z * oneMinusC + a.y * s, a.y * a.z * oneMinusC - a.x * s, c + a.z * a.z * oneMinusC, 0.0f);
            result[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            return result;
        }

        Mat4 Mat4::Perspective(float fov, float aspect, float near, float far) {
            float tanHalfFov = tan(fov / 2.0f);
            Mat4 result = {};
            result[0][0] = 1.0f / (aspect * tanHalfFov);
            result[1][1] = 1.0f / tanHalfFov;
            result[2][2] = -(far + near) / (far - near);
            result[2][3] = -1.0f;
            result[3][2] = -(2.0f * far * near) / (far - near);
            return result;
        }

        Mat4 Mat4::Orthographic(float left, float right, float bottom, float top, float near, float far) {
            Mat4 result = Identity();
            result[0][0] = 2.0f / (right - left);
            result[1][1] = 2.0f / (top - bottom);
            result[2][2] = -2.0f / (far - near);
            result[3][0] = -(right + left) / (right - left);
            result[3][1] = -(top + bottom) / (top - bottom);
            result[3][2] = -(far + near) / (far - near);
            return result;
        }

        Mat4 Mat4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
            Vector3 f = Vector3::Normalise(center - eye);
            Vector3 s = Vector3::Normalise(Vector3::Cross(f, up));
            Vector3 u = Vector3::Cross(s, f);

            Mat4 result = Identity();
            result[0][0] = s.x; result[1][0] = s.y; result[2][0] = s.z;
            result[0][1] = u.x; result[1][1] = u.y; result[2][1] = u.z;
            result[0][2] = -f.x; result[1][2] = -f.y; result[2][2] = -f.z;
            result[3][0] = -Vector3::Dot(s, eye);
            result[3][1] = -Vector3::Dot(u, eye);
            result[3][2] = Vector3::Dot(f, eye);
            return result;
        }

        Mat3 Mat4::NormalMatrix() const {
            return Mat3(
                Vector3(columns[0].x, columns[0].y, columns[0].z),
                Vector3(columns[1].x, columns[1].y, columns[1].z),
                Vector3(columns[2].x, columns[2].y, columns[2].z)
            ).Inversed().Transposed();
        }

        Vector4 Mat4::GetRow(int index) const
        {
            switch (index)
            {
            case 0: return Vector4(columns[index]);
            case 1: return Vector4(columns[index]);
            case 2: return Vector4(columns[index]);
            case 3: return Vector4(columns[index]);
            }
            throw std::out_of_range("Index out of range");
        }

        void Mat4::SetRow(int index, const Vector4& row)
        {
            switch (index)
            {
            case 0: columns[index] = row;
            case 1: columns[index] = row;
            case 2: columns[index] = row;
            case 3: columns[index] = row;
            default: throw std::out_of_range("Index out of range");
            }
        }

#pragma endregion

    }
}

