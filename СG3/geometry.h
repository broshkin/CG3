#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

template <class T>
struct Vec2 {
    T x, y;
    Vec2() : x(0), y(0) {}
    Vec2(T _x, T _y) : x(_x), y(_y) {}

    Vec2<T> operator +(const Vec2<T>& v) const {
        return Vec2<T>(x + v.x, y + v.y);
    }

    Vec2<T> operator -(const Vec2<T>& v) const {
        return Vec2<T>(x - v.x, y - v.y);
    }

    Vec2<T> operator *(float f) const {
        return Vec2<T>(x * f, y * f);
    }
};

template <class T>
struct Vec3 {
    T x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    Vec3<T> operator +(const Vec3<T>& v) const {
        return Vec3<T>(x + v.x, y + v.y, z + v.z);
    }

    Vec3<T> operator -(const Vec3<T>& v) const {
        return Vec3<T>(x - v.x, y - v.y, z - v.z);
    }

    Vec3<T> operator *(float f) const {
        return Vec3<T>(x * f, y * f, z * f);
    }

    Vec3<T> cross(const Vec3<T>& v) const {
        return Vec3<T>(y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
    }

    float dot(const Vec3<T>& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    void normalize() {
        float len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

#endif