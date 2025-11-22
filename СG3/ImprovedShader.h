#ifndef IMPROVED_SHADER_H
#define IMPROVED_SHADER_H

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
#include "ishader.h"

struct ImprovedShader : public IShader {
    Model* model;
    Matrix ModelView;
    Matrix Projection;
    Vec3f light_dir;

    // Material properties
    float ambient_k;
    float diffuse_k;
    float specular_k;
    float shiny_k;

    // Для хранения данных вершин
    Vec3f face_normal;
    Vec3f world_coords[3];

    ImprovedShader()
        : ambient_k(0.2f), diffuse_k(0.6f), specular_k(0.2f), shiny_k(50.0f) {
    }

    virtual Vec4f vertex(int iface, int nthvert) {
        // Проверяем границы
        if (iface < 0 || iface >= model->nfaces()) {
            return Vec4f(0, 0, 0, 1);
        }
        if (nthvert < 0 || nthvert >= 3) {
            return Vec4f(0, 0, 0, 1);
        }

        // Сохраняем мировые координаты для вычислений освещения
        world_coords[nthvert] = model->vert(iface, nthvert);

        // Вычисляем нормаль грани только для первой вершины
        if (nthvert == 0) {
            Vec3f v0 = world_coords[0];
            Vec3f v1 = world_coords[1];
            Vec3f v2 = world_coords[2];

            Vec3f edge1 = v1 - v0;
            Vec3f edge2 = v2 - v0;
            face_normal = cross(edge1, edge2).normalize();
        }

        // Применяем матричные преобразования
        Vec4f gl_Vertex = Projection * ModelView * embed<4>(world_coords[nthvert], 1.0f);

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        // Используем нормаль грани
        Vec3f n = face_normal;

        // Преобразуем нормаль в пространство камеры
        Vec4f normal_camera = ModelView * embed<4>(n, 0.0f);
        Vec3f n_cam = Vec3f(normal_camera[0], normal_camera[1], normal_camera[2]).normalize();

        // Преобразуем направление света в пространство камеры
        Vec4f light_camera = ModelView * embed<4>(light_dir, 0.0f);
        Vec3f l_cam = Vec3f(light_camera[0], light_camera[1], light_camera[2]).normalize();

        // 1. AMBIENT компонент (постоянное освещение)
        float ambient = ambient_k;

        // 2. DIFFUSE компонент (рассеянное освещение по Ламберту)
        float diffuse = diffuse_k * std::max(0.0f, n_cam * l_cam);

        // 3. SPECULAR компонент (блики)
        Vec3f reflect_dir = (n_cam * (n_cam * l_cam * 2.0f) - l_cam).normalize();
        Vec3f view_dir = Vec3f(0, 0, 1); // Камера смотрит по -Z в пространстве камеры
        float specular = specular_k * pow(std::max(0.0f, reflect_dir * view_dir), shiny_k);

        // Комбинируем все компоненты
        float intensity = ambient + diffuse + specular;
        intensity = std::min(1.0f, intensity); // Ограничиваем максимальную яркость

        // Применяем освещение к цвету
        int col = static_cast<int>(255 * intensity);
        col = std::max(0, std::min(255, col));
        color = TGAColor(col, col, col, 255);

        return false;
    }

    // Методы для настройки материала
    void set_material(float amb, float diff, float spec, float shine) {
        ambient_k = amb;
        diffuse_k = diff;
        specular_k = spec;
        shiny_k = shine;
    }
};

#endif