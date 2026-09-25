#pragma once

#include <cmath>
#include <algorithm>

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3()
        : x(0.0f), y(0.0f), z(0.0f)
    {
    }

    Vec3(float X, float Y, float Z)
        : x(X), y(Y), z(Z)
    {
    }

    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float value) const
    {
        return Vec3(x * value, y * value, z * value);
    }

    Vec3& operator+=(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    float Length() const
    {
        return std::sqrt(
            x * x +
            y * y +
            z * z
        );
    }

    Vec3 Normalized() const
    {
        float length = Length();

        if (length <= 0.0001f)
            return Vec3();

        return (*this) * (1.0f / length);
    }
};

namespace Physics
{
    const float EarthGravity = 9.80665f;

    inline Vec3 Gravity(float mass, float gravity)
    {
        return Vec3(
            0.0f,
            0.0f,
            -mass * gravity
        );
    }

    inline Vec3 Drag(
        const Vec3& velocity,
        float airDensity,
        float dragCoefficient,
        float area)
    {
        float speed = velocity.Length();

        if (speed <= 0.001f)
            return Vec3();

        float magnitude =
            0.5f *
            airDensity *
            dragCoefficient *
            area *
            speed *
            speed;

        return velocity.Normalized() * (-magnitude);
    }

    inline float KineticEnergy(
        float mass,
        float speed)
    {
        if (mass < 0.0f)
            mass = 0.0f;

        if (speed < 0.0f)
            speed = -speed;

        return 0.5f * mass * speed * speed;
    }

    inline float Momentum(
        float mass,
        float speed)
    {
        if (mass < 0.0f)
            mass = 0.0f;

        return mass * speed;
    }

    inline float ImpactForce(
        float deltaMomentum,
        float collisionTime)
    {
        if (collisionTime <= 0.0001f)
            collisionTime = 0.0001f;

        return std::fabs(deltaMomentum) / collisionTime;
    }

    inline float GroundImpactEnergy(
        float mass,
        float verticalSpeed)
    {
        return KineticEnergy(
            mass,
            std::fabs(verticalSpeed)
        );
    }

    inline float DamageFromEnergy(
        float energy,
        float scale)
    {
        if (energy < 0.0f)
            energy = 0.0f;

        if (scale < 0.0f)
            scale = 0.0f;

        return energy * scale;
    }

    inline Vec3 ImpulseFromEnergy(
        const Vec3& direction,
        float energy,
        float mass)
    {
        if (mass <= 0.0f)
            return Vec3();

        if (energy < 0.0f)
            energy = 0.0f;

        float velocity =
            std::sqrt(
                2.0f *
                energy /
                mass
            );

        return direction.Normalized() *
               (mass * velocity);
    }
}
