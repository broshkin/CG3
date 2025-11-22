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
    m[0][0] = w / 2.0f;
    m[1][1] = h / 2.0f;
    m[2][2] = 1.0f;
    m[0][3] = x + w / 2.0f;
    m[1][3] = y + h / 2.0f;
    return m;
}

Matrix projection(float coeff = 1.0f) {
    Matrix m = Matrix::identity(4);
    m[3][2] = coeff;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = up.cross(z).normalize();
    Vec3f y = z.cross(x).normalize();

    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

// Улучшенная функция барицентрических координат с проверкой точности
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f v0 = B - A, v1 = C - A, v2 = P - A;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;

    // Защита от деления на ноль
    if (std::abs(denom) < 1e-10f) {
        return Vec3f(-1, -1, -1);
    }

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return Vec3f(u, v, w);
}

// Улучшенный растеризатор с перспективной коррекцией
void drawTriangle(Vec4f* pts, float* zbuffer, TGAImage& image, TGAColor color) {
    // Находим bounding box с небольшим запасом
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for (int i = 0; i < 3; i++) {
        // Делим на w для получения экранных координат
        Vec3f screen = Vec3f(pts[i].x / pts[i].w, pts[i].y / pts[i].w, pts[i].z / pts[i].w);
        bboxmin.x = std::max(0.0f, std::min(bboxmin.x, screen.x));
        bboxmin.y = std::max(0.0f, std::min(bboxmin.y, screen.y));
        bboxmax.x = std::min((float)image.get_width() - 1, std::max(bboxmax.x, screen.x));
        bboxmax.y = std::min((float)image.get_height() - 1, std::max(bboxmax.y, screen.y));
    }

    // Добавляем небольшой запас для избежания артефактов на границах
    bboxmin.x = std::floor(bboxmin.x);
    bboxmin.y = std::floor(bboxmin.y);
    bboxmax.x = std::ceil(bboxmax.x);
    bboxmax.y = std::ceil(bboxmax.y);

    // Проходим по всем пикселям в bounding box
    for (int x = (int)bboxmin.x; x <= (int)bboxmax.x; x++) {
        for (int y = (int)bboxmin.y; y <= (int)bboxmax.y; y++) {
            // Используем центр пикселя для более точного определения
            Vec3f P(x + 0.5f, y + 0.5f, 0);

            // Преобразуем вершины в экранные координаты для барицентрического расчета
            Vec3f A = Vec3f(pts[0].x / pts[0].w, pts[0].y / pts[0].w, pts[0].z / pts[0].w);
            Vec3f B = Vec3f(pts[1].x / pts[1].w, pts[1].y / pts[1].w, pts[1].z / pts[1].w);
            Vec3f C = Vec3f(pts[2].x / pts[2].w, pts[2].y / pts[2].w, pts[2].z / pts[2].w);

            Vec3f bc = barycentric(A, B, C, P);

            // Используем небольшой эпсилон для учета ошибок округления
            if (bc.x < -0.001f || bc.y < -0.001f || bc.z < -0.001f) continue;

            // Перспективно-корректная интерполяция Z
            float z = 0;
            float w = 0;

            // Интерполируем 1/w для перспективной коррекции
            float interpolated_w = 1.0f / (bc.x / pts[0].w + bc.y / pts[1].w + bc.z / pts[2].w);

            // Интерполируем Z с перспективной коррекцией
            z = (bc.x * pts[0].z / pts[0].w + bc.y * pts[1].z / pts[1].w + bc.z * pts[2].z / pts[2].w) * interpolated_w;

            int idx = x + y * WIDTH;
            if (idx < 0 || idx >= WIDTH * HEIGHT) continue;

            // Z-буферизация
            if (zbuffer[idx] < z) {
                zbuffer[idx] = z;
                image.set(x, y, color);
            }
        }
    }
}

int main(int argc, char** argv) {
    const char* modelFile = (argc == 2) ? argv[1] : "obj/african_head.obj";
    Model model(modelFile);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    // Инициализация z-буфера
    float* zbuffer = new float[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    // Настройка камеры
    Vec3f eye(0, 0, 3);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);

    // Матрицы преобразований
    Matrix ModelView = lookat(eye, center, up);
    Matrix Projection = projection(-1.0f / (eye - center).length());
    Matrix Viewport = viewport(0, 0, WIDTH, HEIGHT);

    // Общая матрица преобразований
    Matrix MVP = Viewport * Projection * ModelView;

    Vec3f light_dir(0, 0, -1);

    for (int i = 0; i < model.getFaceCount(); i++) {
        std::vector<int> face = model.getFace(i);
        Vec4f clip_coords[3];
        Vec3f world_coords[3];

        for (int j = 0; j < 3; j++) {
            Vec3f v = model.getVertex(face[j]);
            world_coords[j] = v;

            // Преобразуем вершину через все матрицы
            Matrix m = MVP * v2m(v);
            clip_coords[j] = m2v4(m);
        }

        // Вычисляем нормаль для затенения
        Vec3f n = (world_coords[2] - world_coords[0]).cross(world_coords[1] - world_coords[0]);
        n.normalize();

        float intensity = n.dot(light_dir);

        if (intensity > 0) {
            TGAColor color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
            drawTriangle(clip_coords, zbuffer, image, color);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zbuffer;
    return 0;
}