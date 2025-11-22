#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <limits>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 800;
const TGAColor WHITE = TGAColor(255, 255, 255, 255);

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = 255.f / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = 255.f / 2.f;
    return m;
}

Matrix projection(float coeff = 0.f) {
    Matrix m = Matrix::identity(4);
    m[3][2] = coeff;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up.cross(z)).normalize();
    Vec3f y = (z.cross(x)).normalize();
    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

void drawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, float z0, float z1, float z2,
    float* zbuffer, TGAImage& image, TGAColor color) {
    if (t0.y == t1.y && t0.y == t2.y) return;

    // Сортируем вершины по Y
    if (t0.y > t1.y) { std::swap(t0, t1); std::swap(z0, z1); }
    if (t0.y > t2.y) { std::swap(t0, t2); std::swap(z0, z2); }
    if (t1.y > t2.y) { std::swap(t1, t2); std::swap(z1, z2); }

    int total_height = t2.y - t0.y;

    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

        // Вычисляем координаты A и B
        Vec2i A = Vec2i(t0.x + (t2.x - t0.x) * alpha, t0.y + i);
        Vec2i B = second_half ?
            Vec2i(t1.x + (t2.x - t1.x) * beta, t1.y + (i - (t1.y - t0.y))) :
            Vec2i(t0.x + (t1.x - t0.x) * beta, t0.y + i);

        // Вычисляем z для A и B
        float zA = z0 + (z2 - z0) * alpha;
        float zB = second_half ? z1 + (z2 - z1) * beta : z0 + (z1 - z0) * beta;

        if (A.x > B.x) {
            std::swap(A, B);
            std::swap(zA, zB);
        }

        // Рисуем горизонтальную линию
        for (int j = A.x; j <= B.x; j++) {
            float t = (B.x == A.x) ? 0.0f : (float)(j - A.x) / (float)(B.x - A.x);
            float z = zA + (zB - zA) * t;

            int idx = j + A.y * WIDTH;
            if (idx >= 0 && idx < WIDTH * HEIGHT && zbuffer[idx] < z) {
                zbuffer[idx] = z;
                image.set(j, A.y, color);
            }
        }
    }
}

int main(int argc, char** argv) {
    const char* modelFile = (argc == 2) ? argv[1] : "obj/new_peter_griffin.obj";
    Model model(modelFile);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    // Инициализация z-буфера
    float* zbuffer = new float[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    // Настройка камеры
    Vec3f eye(1, 1, 3);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);

    // Матрицы преобразований
    Matrix ModelView = lookat(eye, center, up);
    Matrix Viewport = viewport(WIDTH / 8, HEIGHT / 8, WIDTH * 3 / 4, HEIGHT * 3 / 4);
    Matrix Projection = projection(-1.f / (eye - center).length());

    Vec3f light_dir(0, 0, -1);

    for (int i = 0; i < model.getFaceCount(); i++) {
        std::vector<int> face = model.getFace(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        float z_values[3];

        for (int j = 0; j < 3; j++) {
            Vec3f v = model.getVertex(face[j]);

            // Преобразование координат через матрицы
            Matrix m = Viewport * Projection * ModelView * v2m(v);
            Vec3f transformed = m2v(m);

            screen_coords[j] = Vec2i(transformed.x, transformed.y);
            world_coords[j] = v;
            z_values[j] = transformed.z;
        }

        Vec3f n = (world_coords[2] - world_coords[0]).cross(world_coords[1] - world_coords[0]);
        n.normalize();

        float intensity = n.dot(light_dir);

        if (intensity > 0) {
            TGAColor color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
            drawTriangle(screen_coords[0], screen_coords[1], screen_coords[2],
                z_values[0], z_values[1], z_values[2],
                zbuffer, image, color);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zbuffer;
    return 0;
}