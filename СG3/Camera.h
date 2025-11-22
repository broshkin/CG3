#ifndef CAMERA_H
#define CAMERA_H

#include "geometry.h"

class Camera {
private:
    Vec3f eye;
    Vec3f center;
    Vec3f up;
    Matrix view_matrix;
    Matrix projection_matrix;

public:
    Camera(Vec3f eye_pos, Vec3f look_at, Vec3f up_dir)
        : eye(eye_pos), center(look_at), up(up_dir) {
        update_view_matrix();
        update_projection_matrix();
    }

    void update_view_matrix() {
        Vec3f z = (eye - center).normalize();
        Vec3f x = cross(up, z).normalize();
        Vec3f y = cross(z, x).normalize();

        view_matrix = Matrix::identity();
        for (int i = 0; i < 3; i++) {
            view_matrix[0][i] = x[i];
            view_matrix[1][i] = y[i];
            view_matrix[2][i] = z[i];
            view_matrix[i][3] = -center[i];
        }
    }

    void update_projection_matrix(float coeff = 0.f) {
        projection_matrix = Matrix::identity();
        projection_matrix[3][2] = coeff;
    }

    Matrix get_view_matrix() const { return view_matrix; }
    Matrix get_projection_matrix() const { return projection_matrix; }

    void set_eye(const Vec3f& new_eye) {
        eye = new_eye;
        update_view_matrix();
    }

    void set_center(const Vec3f& new_center) {
        center = new_center;
        update_view_matrix();
    }
};

#endif