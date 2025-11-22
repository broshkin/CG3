#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
#include "ishader.h"

struct SimpleShader : public IShader {
    Model* model;
    Matrix ModelView;
    Matrix Projection;
    Vec3f light_dir;
    Vec3f camera_pos; 

    float ambient_k;
    float diffuse_k;
    float specular_k; 
    float shininess; 

    // Для интерполяции нормалей и позиций
    mat<3, 3, float> varying_nrm;
    mat<3, 3, float> varying_pos; // Добавил для хранения позиций в пространстве камеры

    SimpleShader() : ambient_k(0.2f), diffuse_k(0.6f), specular_k(0.4f), shininess(32.0f) {}

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec3f vertex = model->vert(iface, nthvert);


        if (nthvert == 0) {
            Vec3f v0 = model->vert(iface, 0);
            Vec3f v1 = model->vert(iface, 1);
            Vec3f v2 = model->vert(iface, 2);

            Vec3f edge1 = v1 - v0;
            Vec3f edge2 = v2 - v0;
            Vec3f normal = cross(edge1, edge2).normalize();


            Vec4f normal_camera = ModelView * embed<4>(normal, 0.0f);
            Vec3f n_cam = Vec3f(normal_camera[0], normal_camera[1], normal_camera[2]).normalize();

            for (int i = 0; i < 3; i++) {
                varying_nrm.set_col(i, n_cam);
            }
        }


        Vec4f vertex_camera = ModelView * embed<4>(vertex, 1.0f);
        varying_pos.set_col(nthvert, Vec3f(vertex_camera[0], vertex_camera[1], vertex_camera[2]));

        Vec4f gl_Vertex = Projection * vertex_camera;
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec3f n = varying_nrm.col(0).normalize();
        Vec4f light_camera = ModelView * embed<4>(light_dir, 0.0f);
        Vec3f l = Vec3f(light_camera[0], light_camera[1], light_camera[2]).normalize();


        Vec3f pos_camera = varying_pos * bar;


        float ambient = ambient_k;


        float diffuse = diffuse_k * std::max(0.0f, n * l);


        Vec3f view_dir = (camera_pos - pos_camera).normalize();
        Vec3f reflect_dir = (n * (2.0f * (n * l)) - l).normalize();
        float specular = specular_k * pow(std::max(0.0f, view_dir * reflect_dir), shininess);

        // Итоговая интенсивность
        float intensity = ambient + diffuse + specular;
        intensity = std::min(1.0f, std::max(0.0f, intensity));

        int col = static_cast<int>(255 * intensity);
        color = TGAColor(col, col, col, 255);

        return false;
    }
};

#endif