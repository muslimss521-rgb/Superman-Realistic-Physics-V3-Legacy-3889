#pragma once
#include <cmath>
#include <algorithm>

struct Vec3 {
    float x{}, y{}, z{};
    Vec3 operator+(const Vec3& o) const { return {x+o.x,y+o.y,z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x,y-o.y,z-o.z}; }
    Vec3 operator*(float s) const { return {x*s,y*s,z*s}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    float Length() const { return std::sqrt(x*x+y*y+z*z); }
    Vec3 Normalized() const { float l=Length(); return l>0.0001f ? *this*(1.0f/l) : Vec3{}; }
};

namespace Physics {
    constexpr float EarthGravity = 9.80665f;

    inline Vec3 Gravity(float mass, float g = EarthGravity) {
        return {0.0f, 0.0f, -mass*g};
    }

    inline Vec3 Drag(const Vec3& velocity, float airDensity, float cd, float area) {
        float speed = velocity.Length();
        if (speed < 0.001f) return {};
        float mag = 0.5f * airDensity * cd * area * speed * speed;
        return velocity.Normalized() * -mag;
    }

    inline float KineticEnergy(float mass, float speed) {
        return 0.5f * mass * speed * speed;
    }

    inline float Momentum(float mass, float speed) {
        return mass * speed;
    }

    inline float ImpactForce(float deltaMomentum, float collisionTime) {
        return collisionTime > 0.0001f ? deltaMomentum / collisionTime : deltaMomentum / 0.0001f;
    }

    inline float GroundImpactEnergy(float mass, float verticalSpeed) {
        return KineticEnergy(mass, std::abs(verticalSpeed));
    }

    inline float DamageFromEnergy(float energy, float scale) {
        return std::max(0.0f, energy * scale);
    }

    inline Vec3 ImpulseFromEnergy(const Vec3& direction, float energy, float mass) {
        if (mass <= 0.0f) return {};
        float v = std::sqrt(std::max(0.0f, 2.0f * energy / mass));
        return direction.Normalized() * (mass * v);
    }
}
