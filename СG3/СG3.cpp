#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <limits>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 800;
const TGAColor WHITE = TGAColor(255, 255, 255, 255);

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
    const char* modelFile = (argc == 2) ? argv[1] : "obj/african_head.obj";
    Model model(modelFile);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    // Инициализация z-буфера
    float* zbuffer = new float[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    Vec3f lightDir(0, 0, -1);

    for (int i = 0; i < model.getFaceCount(); i++) {
        std::vector<int> face = model.getFace(i);
        Vec2i screenCoords[3];
        Vec3f worldCoords[3];
        float zValues[3];

        for (int j = 0; j < 3; j++) {
            Vec3f vertex = model.getVertex(face[j]);
            screenCoords[j] = Vec2i((vertex.x + 1.0f) * WIDTH / 2.0f,
                (vertex.y + 1.0f) * HEIGHT / 2.0f);
            worldCoords[j] = vertex;
            zValues[j] = vertex.z;
        }

        Vec3f normal = (worldCoords[2] - worldCoords[0]).cross(worldCoords[1] - worldCoords[0]);
        normal.normalize();

        float intensity = normal.dot(lightDir);

        if (intensity > 0) {
            TGAColor color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
            drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2],
                zValues[0], zValues[1], zValues[2],
                zbuffer, image, color);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zbuffer;
    return 0;
}