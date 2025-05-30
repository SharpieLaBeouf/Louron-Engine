#include "LQuaternion.h"

#include "LVectors.h"

namespace Louron
{
    Quaternion Quaternion::Inverse() const
    {
        float len_sq = x * x + y * y + z * z + w * w;
        if (len_sq == 0.0f) return Identity();
        return Conjugate() / len_sq;
    }

    Quaternion Quaternion::Euler(const Vectors::Vector3 &euler_degrees)
    {
        float yaw = LMath::Radians(euler_degrees.y);
        float pitch = LMath::Radians(euler_degrees.x);
        float roll = LMath::Radians(euler_degrees.z);

        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);

        return Quaternion(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }

    Vectors::Vector3 Louron::Quaternion::ToEuler() const
    {
        Vectors::Vector3 euler;

        // Pitch (x-axis rotation)
        float sinp = 2.0f * (w * x + y * z);
        float cosp = 1.0f - 2.0f * (x * x + y * y);
        euler.x = LMath::Degrees(std::atan2(sinp, cosp));

        // Yaw (y-axis rotation)
        float siny = 2.0f * (w * y - z * x);
        siny = LMath::Clamp(siny, -1.0f, 1.0f);
        euler.y = LMath::Degrees(std::asin(siny));

        // Roll (z-axis rotation)
        float sinr = 2.0f * (w * z + x * y);
        float cosr = 1.0f - 2.0f * (y * y + z * z);
        euler.z = LMath::Degrees(std::atan2(sinr, cosr));

        return euler;
    }

    Quaternion Quaternion::AxisAngle(const Vectors::Vector3 &axis, float angle_degrees)
    {
        float angle_radians = LMath::Radians(angle_degrees);
        float half_angle = angle_radians * 0.5f;
        float s = std::sin(half_angle);
        Vectors::Vector3 norm_axis = Vectors::Vector3::Normalise(axis);
        return Quaternion(norm_axis.x * s, norm_axis.y * s, norm_axis.z * s, std::cos(half_angle));
    }

    Quaternion Quaternion::Slerp(const Quaternion &a, const Quaternion &b, float t)
    {
        Quaternion qa = a.Normalise();
        Quaternion qb = b.Normalise();

        float dot = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;

        if (dot < 0.0f) {
            qb = -qb;
            dot = -dot;
        }

        if (dot > 0.9995f) {
            // Fallback to linear interpolation
            return Normalise(Quaternion(
                LMath::Lerp(t, qa.x, qb.x),
                LMath::Lerp(t, qa.y, qb.y),
                LMath::Lerp(t, qa.z, qb.z),
                LMath::Lerp(t, qa.w, qb.w)
            ));
        }

        float theta_0 = std::acos(dot);
        float theta = theta_0 * t;

        Quaternion rel = (qb - qa * dot).Normalise();
        return qa * std::cos(theta) + rel * std::sin(theta);
    }

    Vectors::Vector3 Quaternion::Rotate(const Vectors::Vector3 &v) const
    {
        Quaternion qv(v.x, v.y, v.z, 0.0f);
        Quaternion result = (*this) * qv * Inverse();
        return Vectors::Vector3(result.x, result.y, result.z);
    }

    Quaternion Quaternion::operator*(const Quaternion &q) const
    {
        return Quaternion(
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        );
    }
}
