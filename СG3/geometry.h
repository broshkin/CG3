#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <iostream>

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

    Vec3<T>& normalize() {
        float len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
        return *this;
    }

    T& operator[](int i) {
        return i == 0 ? x : (i == 1 ? y : z);
    }
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

class Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;

public:
    Matrix(int r = 4, int c = 4) : rows(r), cols(c) {
        m.resize(rows);
        for (int i = 0; i < rows; i++) {
            m[i].resize(cols, 0);
        }
    }

    int nrows() const { return rows; }
    int ncols() const { return cols; }

    static Matrix identity(int dimensions) {
        Matrix E(dimensions, dimensions);
        for (int i = 0; i < dimensions; i++) {
            E[i][i] = 1;
        }
        return E;
    }

    std::vector<float>& operator[](const int i) {
        return m[i];
    }

    const std::vector<float>& operator[](const int i) const {
        return m[i];
    }

    Matrix operator*(const Matrix& a) const {
        Matrix result(rows, a.cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < a.cols; j++) {
                result[i][j] = 0;
                for (int k = 0; k < cols; k++) {
                    result[i][j] += m[i][k] * a[k][j];
                }
            }
        }
        return result;
    }

    Matrix transpose() const {
        Matrix result(cols, rows);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result[j][i] = m[i][j];
            }
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& s, Matrix& m) {
        for (int i = 0; i < m.nrows(); i++) {
            for (int j = 0; j < m.ncols(); j++) {
                s << m[i][j] << " ";
            }
            s << "\n";
        }
        return s;
    }
};

inline Matrix v2m(const Vec3f& v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

inline Vec3f m2v(const Matrix& m) {
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

#endif