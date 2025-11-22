#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "model.h"

Model::Model(const char* filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; i++) iss >> uv[i];
            uv_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> f_uv;
            std::vector<int> f_norm;
            
            iss >> trash; // читаем 'f'
            std::string face_token;
            
            while (iss >> face_token) {
                std::istringstream token_stream(face_token);
                std::string token;
                std::vector<std::string> indices;
                
                // –азбиваем по '/'
                while (std::getline(token_stream, token, '/')) {
                    indices.push_back(token);
                }
                
                // Vertex index (об€зательный)
                if (!indices[0].empty()) {
                    int v_idx = std::stoi(indices[0]) - 1;
                    f.push_back(v_idx);
                }
                
                // Texture coordinate index (опциональный)
                if (indices.size() > 1 && !indices[1].empty()) {
                    int uv_idx = std::stoi(indices[1]) - 1;
                    f_uv.push_back(uv_idx);
                } else {
                    f_uv.push_back(-1); // маркер отсутстви€ UV
                }
                
                // Normal index (опциональный)
                if (indices.size() > 2 && !indices[2].empty()) {
                    int n_idx = std::stoi(indices[2]) - 1;
                    f_norm.push_back(n_idx);
                } else {
                    f_norm.push_back(-1); // маркер отсутстви€ нормали
                }
            }
            
            faces_.push_back(f);
            faces_uv_.push_back(f_uv);
            faces_norms_.push_back(f_norm);
        }
    }

    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() 
              << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

Vec3f Model::vert(int i) {
    if (i < 0 || i >= verts_.size()) {
        std::cerr << "vert index out of range: " << i << std::endl;
        return Vec3f(0, 0, 0);
    }
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    if (iface < 0 || iface >= faces_.size()) {
        std::cerr << "face index out of range: " << iface << std::endl;
        return Vec3f(0, 0, 0);
    }
    if (nthvert < 0 || nthvert >= faces_[iface].size()) {
        std::cerr << "vertex index out of range in face: " << iface << ", " << nthvert << std::endl;
        return Vec3f(0, 0, 0);
    }
    return verts_[faces_[iface][nthvert]];
}

Vec3f Model::normal(int iface, int nthvert) {
    // ≈сли нормалей нет в файле или дл€ этой вершины нет нормали
    if (norms_.empty() || faces_norms_.empty() || 
        iface < 0 || iface >= faces_norms_.size() ||
        nthvert < 0 || nthvert >= faces_norms_[iface].size() ||
        faces_norms_[iface][nthvert] == -1) {
        
        // ¬ычисл€ем нормаль по грани
        Vec3f v0 = vert(iface, 0);
        Vec3f v1 = vert(iface, 1);
        Vec3f v2 = vert(iface, 2);
        Vec3f n = cross((v2 - v0), (v1 - v0));
        n.normalize();
        return n;
    }

    int normal_index = faces_norms_[iface][nthvert];
    if (normal_index < 0 || normal_index >= norms_.size()) {
        std::cerr << "Normal index out of range: " << normal_index << std::endl;
        return Vec3f(0, 1, 0); // нормаль по умолчанию
    }

    return norms_[normal_index];
}

Vec2f Model::uv(int iface, int nthvert) {
    if (uv_.empty() || faces_uv_.empty() || 
        iface < 0 || iface >= faces_uv_.size() ||
        nthvert < 0 || nthvert >= faces_uv_[iface].size() ||
        faces_uv_[iface][nthvert] == -1) {
        return Vec2f(0, 0);
    }

    int uv_index = faces_uv_[iface][nthvert];
    if (uv_index < 0 || uv_index >= uv_.size()) {
        std::cerr << "UV index out of range: " << uv_index << std::endl;
        return Vec2f(0, 0);
    }

    return uv_[uv_index];
}

std::vector<int> Model::face(int idx) {
    if (idx < 0 || idx >= faces_.size()) {
        std::cerr << "Face index out of range: " << idx << std::endl;
        return std::vector<int>();
    }
    return faces_[idx];
}