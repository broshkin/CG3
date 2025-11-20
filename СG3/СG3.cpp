#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int WIDTH = 800;
const int HEIGHT = 800;
const TGAColor WHITE = TGAColor(255, 255, 255, 255);

void drawTriangle(Vec2i p0, Vec2i p1, Vec2i p2, TGAImage& image, TGAColor color) {
    if (p0.y > p1.y) std::swap(p0, p1);
    if (p0.y > p2.y) std::swap(p0, p2);
    if (p1.y > p2.y) std::swap(p1, p2);

    int totalHeight = p2.y - p0.y;

    for (int y = p0.y; y <= p2.y; y++) {
        bool isSecondPart = y > p1.y || p1.y == p0.y;
        int segmentHeight = isSecondPart ? p2.y - p1.y : p1.y - p0.y;

        float alpha = (float)(y - p0.y) / totalHeight;
        float beta = (float)(y - (isSecondPart ? p1.y : p0.y)) / segmentHeight;

        Vec2i A = p0 + (p2 - p0) * alpha;
        Vec2i B = isSecondPart ? p1 + (p2 - p1) * beta : p0 + (p1 - p0) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int x = A.x; x <= B.x; x++) {
            image.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    const char* modelFile = (argc == 2) ? argv[1] : "obj/new_peter_griffin.obj";
    Model model(modelFile);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    Vec3f lightDirection(0, 0, -1);

    for (int i = 0; i < model.getFaceCount(); i++) {
        std::vector<int> face = model.getFace(i);
        Vec2i screenCoords[3];
        Vec3f worldCoords[3];

        for (int j = 0; j < 3; j++) {
            Vec3f vertex = model.getVertex(face[j]);
            screenCoords[j] = Vec2i((vertex.x + 1.0f) * WIDTH / 2.0f, (vertex.y + 1.0f) * HEIGHT / 2.0f);
            worldCoords[j] = vertex;
        }

        Vec3f normal = (worldCoords[2] - worldCoords[0]).cross(worldCoords[1] - worldCoords[0]);
        normal.normalize();

        float intensity = normal.dot(lightDirection);

        if (intensity > 0) {
            TGAColor color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
            drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2], image, color);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}