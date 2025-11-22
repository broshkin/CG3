#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "ishader.h"
#include <limits>
#include <algorithm>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 800;

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity();
    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = 1.f;
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    return m;
}

Matrix projection(float coeff = 0.f) {
    Matrix m = Matrix::identity();
    m[3][2] = coeff;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();
    Matrix res = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

void triangle(mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer) {
    Matrix viewport_mat = viewport(0, 0, image.get_width(), image.get_height());
    mat<3, 4, float> pts;

    for (int i = 0; i < 3; i++) {
        Vec4f vertex;
        for (int j = 0; j < 4; j++) {
            vertex[j] = clipc[j][i];
        }
        Vec4f transformed = viewport_mat * vertex;
        pts[i] = transformed;
    }

    mat<3, 2, float> pts2;
    for (int i = 0; i < 3; i++) {
        Vec4f v = pts[i];
        pts2[i] = Vec2f(v[0] / v[3], v[1] / v[3]);
    }

    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
        }
    }

    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], Vec2f(P.x, P.y));
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            float frag_depth = 0;
            for (int k = 0; k < 3; k++) {
                frag_depth += bc_screen[k] * clipc[2][k];
            }

            int idx = P.x + P.y * image.get_width();
            if (idx < 0 || idx >= WIDTH * HEIGHT) continue;

            if (zbuffer[idx] < frag_depth) {
                zbuffer[idx] = frag_depth;
                bool discard = shader.fragment(bc_screen, color);
                if (!discard) {
                    image.set(P.x, P.y, color);
                }
            }
        }
    }
}

// Шейдер с вычисляемыми нормалями (плоское освещение)
struct PhongShader : public IShader {
    Model* model;
    Matrix ModelView;
    Matrix Projection;
    Vec3f light_dir;

    // Для хранения нормалей вершин (одинаковая для всех вершин грани)
    Vec3f face_normal;

    virtual Vec4f vertex(int iface, int nthvert) {
        // Проверяем границы
        if (iface < 0 || iface >= model->nfaces()) {
            return Vec4f(0, 0, 0, 1);
        }
        if (nthvert < 0 || nthvert >= 3) {
            return Vec4f(0, 0, 0, 1);
        }

        // Вычисляем нормаль грани по трем вершинам
        if (nthvert == 0) {
            Vec3f v0 = model->vert(iface, 0);
            Vec3f v1 = model->vert(iface, 1);
            Vec3f v2 = model->vert(iface, 2);

            Vec3f edge1 = v1 - v0;
            Vec3f edge2 = v2 - v0;
            face_normal = cross(edge1, edge2).normalize();
        }

        // Получаем вершину
        Vec3f vertex = model->vert(iface, nthvert);

        // Применяем матричные преобразования
        Vec4f gl_Vertex = Projection * ModelView * embed<4>(vertex, 1.0f);

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        // Используем нормаль грани (плоское освещение)
        Vec3f n = face_normal;

        // Преобразуем нормаль в пространство камеры
        Vec4f normal_camera = ModelView * embed<4>(n, 0.0f);
        Vec3f n_cam = Vec3f(normal_camera[0], normal_camera[1], normal_camera[2]).normalize();

        // Преобразуем направление света в пространство камеры
        Vec4f light_camera = ModelView * embed<4>(light_dir, 0.0f);
        Vec3f l_cam = Vec3f(light_camera[0], light_camera[1], light_camera[2]).normalize();

        // Освещение по Ламберту
        float intensity = std::max(0.0f, n_cam * l_cam);

        // Добавляем немного ambient освещения
        intensity = 0.2f + 0.8f * intensity;

        int col = static_cast<int>(255 * intensity);

        // Ограничиваем диапазон цвета
        col = std::max(0, std::min(255, col));
        color = TGAColor(col, col, col, 255);

        return false;
    }
};

int main(int argc, char** argv) {
    Model* model = new Model("obj/new_peter_griffin.obj");
    if (model->nfaces() == 0) {
        std::cerr << "ERROR: Model not loaded!" << std::endl;
        return -1;
    }
    std::cout << "Model loaded: " << model->nfaces() << " faces" << std::endl;

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    float* zbuffer = new float[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    // Настройка камеры
    Vec3f eye(1, 1, 3);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);

    // Матрицы
    Matrix ModelView = lookat(eye, center, up);
    Matrix Projection = projection(-1.f / (eye - center).norm());

    // Направление света
    Vec3f light_dir = (Vec3f(1, 1, 1)).normalize();

    // Шейдер
    PhongShader shader;
    shader.model = model;
    shader.ModelView = ModelView;
    shader.Projection = Projection;
    shader.light_dir = light_dir;

    // РЕНДЕРИМ ВСЕ ГРАНИ
    std::cout << "Rendering all faces with computed normals..." << std::endl;
    for (int i = 0; i < model->nfaces(); i++) {
        if (i % 500 == 0) std::cout << "Rendering face " << i << std::endl;
        mat<4, 3, float> screen_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords.set_col(j, shader.vertex(i, j));
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    std::cout << "Rendering completed!" << std::endl;

    delete model;
    delete[] zbuffer;
    return 0;
}