#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <iostream>

// Шаблонный класс для векторов произвольной размерности
template <size_t DIM, typename T>
struct vec {
    std::vector<T> data;

    vec() : data(DIM) {}
    vec(T value) : data(DIM, value) {}

    T& operator[](const size_t i) {
        if (i < DIM) return data[i];
        throw std::out_of_range("Index out of range");
    }

    const T& operator[](const size_t i) const {
        if (i < DIM) return data[i];
        throw std::out_of_range("Index out of range");
    }

    vec<DIM, T>& operator=(const vec<DIM, T>& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }

    size_t size() const { return DIM; }
};

// Специализации для 2D, 3D, 4D векторов
template <typename T>
struct vec<2, T> {
    T x, y;

    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    vec(T value) : x(value), y(value) {}

    T& operator[](const size_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        throw std::out_of_range("Index out of range");
    }

    const T& operator[](const size_t i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        throw std::out_of_range("Index out of range");
    }

    vec<2, T>& operator=(const vec<2, T>& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    size_t size() const { return 2; }
};

template <typename T>
struct vec<3, T> {
    T x, y, z;

    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    vec(T value) : x(value), y(value), z(value) {}

    T& operator[](const size_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        throw std::out_of_range("Index out of range");
    }

    const T& operator[](const size_t i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        throw std::out_of_range("Index out of range");
    }

    vec<3, T>& operator=(const vec<3, T>& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }

    float norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    vec<3, T>& normalize() {
        float n = norm();
        if (n > 0) {
            x /= n;
            y /= n;
            z /= n;
        }
        return *this;
    }

    size_t size() const { return 3; }
};

template <typename T>
struct vec<4, T> {
    T x, y, z, w;

    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    vec(T value) : x(value), y(value), z(value), w(value) {}

    T& operator[](const size_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        if (i == 3) return w;
        throw std::out_of_range("Index out of range");
    }

    const T& operator[](const size_t i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        if (i == 3) return w;
        throw std::out_of_range("Index out of range");
    }

    vec<4, T>& operator=(const vec<4, T>& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
        }
        return *this;
    }

    size_t size() const { return 4; }
};

// Псевдонимы для удобства
typedef vec<2, float> Vec2f;
typedef vec<2, int>   Vec2i;
typedef vec<3, float> Vec3f;
typedef vec<3, int>   Vec3i;
typedef vec<4, float> Vec4f;

// Операции с векторами
template<size_t DIM, typename T>
T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = 0; i < DIM; i++)
        ret += lhs[i] * rhs[i];
    return ret;
}

template<size_t DIM, typename T>
vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; i++)
        lhs[i] += rhs[i];
    return lhs;
}

template<size_t DIM, typename T>
vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; i++)
        lhs[i] -= rhs[i];
    return lhs;
}

template<size_t DIM, typename T, typename U>
vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = 0; i < DIM; i++)
        lhs[i] *= rhs;
    return lhs;
}

template<size_t DIM, typename T, typename U>
vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = 0; i < DIM; i++)
        lhs[i] /= rhs;
    return lhs;
}

// Векторное произведение для 3D
template<typename T>
vec<3, T> cross(const vec<3, T>& v1, const vec<3, T>& v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
}

// Преобразования размерности
template <size_t LEN, size_t DIM, typename T>
vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1) {
    vec<LEN, T> ret;
    for (size_t i = 0; i < LEN; i++)
        ret[i] = (i < DIM ? v[i] : fill);
    return ret;
}

template <size_t LEN, size_t DIM, typename T>
vec<LEN, T> proj(const vec<DIM, T>& v) {
    vec<LEN, T> ret;
    for (size_t i = 0; i < LEN; i++)
        ret[i] = v[i];
    return ret;
}

// Матричный класс
template<size_t ROWS, size_t COLS, typename T>
struct mat {
    vec<ROWS, vec<COLS, T>> rows;

    mat() {
        for (size_t i = 0; i < ROWS; i++)
            rows[i] = vec<COLS, T>(T());
    }

    vec<COLS, T>& operator[](const size_t idx) {
        return rows[idx];
    }

    const vec<COLS, T>& operator[](const size_t idx) const {
        return rows[idx];
    }

    vec<ROWS, T> col(const size_t idx) const {
        vec<ROWS, T> ret;
        for (size_t i = 0; i < ROWS; i++)
            ret[i] = rows[i][idx];
        return ret;
    }

    void set_col(size_t idx, const vec<ROWS, T>& v) {
        for (size_t i = 0; i < ROWS; i++)
            rows[i][idx] = v[i];
    }

    static mat<ROWS, COLS, T> identity() {
        mat<ROWS, COLS, T> ret;
        for (size_t i = 0; i < ROWS && i < COLS; i++)
            ret[i][i] = 1;
        return ret;
    }

    mat<ROWS, COLS, T>& operator=(const mat<ROWS, COLS, T>& other) {
        if (this != &other) {
            for (size_t i = 0; i < ROWS; i++)
                rows[i] = other.rows[i];
        }
        return *this;
    }
};

// Умножение матрицы на вектор
template<size_t ROWS, size_t COLS, typename T>
vec<ROWS, T> operator*(const mat<ROWS, COLS, T>& lhs, const vec<COLS, T>& rhs) {
    vec<ROWS, T> ret;
    for (size_t i = 0; i < ROWS; i++)
        ret[i] = lhs[i] * rhs;
    return ret;
}

// Умножение матрицы на матрицу
template<size_t ROWS, size_t COLS, size_t COLS2, typename T>
mat<ROWS, COLS2, T> operator*(const mat<ROWS, COLS, T>& lhs, const mat<COLS, COLS2, T>& rhs) {
    mat<ROWS, COLS2, T> result;
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS2; j++) {
            result[i][j] = T();
            for (size_t k = 0; k < COLS; k++) {
                result[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }
    return result;
}

// Транспонирование матрицы
template<size_t ROWS, size_t COLS, typename T>
mat<COLS, ROWS, T> transpose(const mat<ROWS, COLS, T>& m) {
    mat<COLS, ROWS, T> ret;
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            ret[j][i] = m[i][j];
        }
    }
    return ret;
}

// Специализация для 4x4 матриц (для удобства)
typedef mat<4, 4, float> Matrix;

// Функции для работы с матрицами
inline Matrix v2m(const Vec3f& v) {
    Matrix m = Matrix::identity();
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.0f;
    return m;
}

inline Vec3f m2v(const Matrix& m) {
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

inline Vec4f m2v4(const Matrix& m) {
    return Vec4f(m[0][0], m[1][0], m[2][0], m[3][0]);
}

inline Matrix v42m(const Vec4f& v) {
    Matrix m = Matrix::identity();
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = v.w;
    return m;
}

#endif