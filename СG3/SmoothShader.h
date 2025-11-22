#ifndef SMOOTH_SHADER_H
#define SMOOTH_SHADER_H

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
#include "ishader.h"

struct SmoothShader : public IShader {
    Model* model;
    Matrix ModelView;
    Matrix Projection;
    Matrix Viewport;
    Vec3f light_dir;

    // Material properties
    float ambient_k;
    float diffuse_k;
    float specular_k;
    float shiny_k;

    // Varying variables для интерполяции
    mat<3, 3, float> varying_nrm;  // Нормали вершин
    mat<3, 3, float> world_coords; // Мировые координаты вершин

    SmoothShader()
        : ambient_k(0.2f), diffuse_k(0.6f), specular_k(0.2f), shiny_k(50.0f) {
    }

    virtual Vec4f vertex(int iface, int nthvert) {
        // Получаем вершину
        Vec3f vertex = model->vert(iface, nthvert);
        world_coords.set_col(nthvert, vertex);

        // Получаем нормаль вершины из модели
        Vec3f normal;
        try {
            normal = model->normal(iface, nthvert);
        }
        catch (...) {
            // Если нормалей нет, вычисляем по грани
            Vec3f v0 = model->vert(iface, 0);
            Vec3f v1 = model->vert(iface, 1);
            Vec3f v2 = model->vert(iface, 2);
            Vec3f edge1 = v1 - v0;
            Vec3f edge2 = v2 - v0;
            normal = cross(edge1, edge2).normalize();
        }

        // Преобразуем нормаль в пространство камеры
        Vec4f normal_camera = ModelView * embed<4>(normal, 0.0f);
        Vec3f n_cam = Vec3f(normal_camera[0], normal_camera[1], normal_camera[2]).normalize();
        varying_nrm.set_col(nthvert, n_cam);

        // Преобразуем вершину
        Vec4f gl_Vertex = Projection * ModelView * embed<4>(vertex, 1.0f);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        // ИНТЕРПОЛИРУЕМ НОРМАЛЬ МЕЖДУ ВЕРШИНАМИ
        Vec3f n;
        for (int i = 0; i < 3; i++) {
            n[i] = varying_nrm[i][0] * bar.x + varying_nrm[i][1] * bar.y + varying_nrm[i][2] * bar.z;
        }
        n = n.normalize();

        // ИНТЕРПОЛИРУЕМ МИРОВЫЕ КООРДИНАТЫ ДЛЯ ПРАВИЛЬНОГО ОСВЕЩЕНИЯ
        Vec3f world_pos;
        for (int i = 0; i < 3; i++) {
            world_pos[i] = world_coords[i][0] * bar.x + world_coords[i][1] * bar.y + world_coords[i][2] * bar.z;
        }

        // Преобразуем направление света
        Vec4f light_camera = ModelView * embed<4>(light_dir, 0.0f);
        Vec3f l = Vec3f(light_camera[0], light_camera[1], light_camera[2]).normalize();

        // 1. AMBIENT компонент
        float ambient = ambient_k;

        // 2. DIFFUSE компонент (корректный расчет)
        float diffuse = diffuse_k * std::max(0.0f, n * l);

        // 3. SPECULAR компонент (исправленный расчет)
        Vec3f view_dir = Vec3f(0, 0, 1).normalize(); // Направление взгляда в пространстве камеры
        Vec3f reflect_dir = (n * (n * l * 2.0f) - l).normalize();
        float specular = specular_k * pow(std::max(0.0f, reflect_dir * view_dir), shiny_k);

        // ОГРАНИЧИВАЕМ КОМПОНЕНТЫ ДЛЯ ИЗБЕЖАНИЯ ПЕРЕСВЕТА
        float intensity = ambient + diffuse + specular;
        intensity = std::min(1.0f, std::max(0.0f, intensity));

        // Применяем освещение
        int col = static_cast<int>(255 * intensity);
        color = TGAColor(col, col, col, 255);

        return false;
    }

    void set_material(float amb, float diff, float spec, float shine) {
        ambient_k = amb;
        diffuse_k = diff;
        specular_k = spec;
        shiny_k = shine;
    }
};

#endif