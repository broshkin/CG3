#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "ishader.h"
#include "Camera.h"
#include "ImprovedShader.h"
#include "SimpleShader.h"
#include "SmoothShader.h"
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

void triangle(mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer, float clip_plane = 0.0f) {
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


            if (frag_depth > clip_plane) { 
                continue; 
            }

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

int main(int argc, char** argv) {
    Model* model = new Model("obj/123456.obj");
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
    Vec3f eye(1, 0, 1);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);
    Camera camera(eye, center, up);
    
    // Направление света
    Vec3f light_dir = (Vec3f(1, 1, 1)).normalize();

    SimpleShader shader;
    shader.model = model;
    shader.ModelView = camera.get_view_matrix();
    shader.Projection = camera.get_projection_matrix();
    shader.light_dir = light_dir;

    for (int i = 0; i < model->nfaces(); i++) {
        if (i % 500 == 0) std::cout << "Rendering face " << i << std::endl;
        mat<4, 3, float> screen_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords.set_col(j, shader.vertex(i, j));
        }
        float clip_plane = 0.15f;
        triangle(screen_coords, shader, image, zbuffer, clip_plane);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    
    std::cout << "Rendering completed!" << std::endl;
    
    delete model;
    delete[] zbuffer;
    return 0;
}