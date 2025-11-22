#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;

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
            std::vector<int> fn;
            std::vector<int> fuv;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // wavefront obj starts from 1
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }

    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
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
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert]];
}

Vec3f Model::normal(int iface, int nthvert) {
    if (norms_.empty()) {
        // Compute flat normal if no normals in file
        Vec3f v0 = vert(iface, 0);
        Vec3f v1 = vert(iface, 1);
        Vec3f v2 = vert(iface, 2);
        Vec3f n = cross((v2 - v0), (v1 - v0));
        n.normalize();
        return n;
    }
    return norms_[faces_norms_[iface][nthvert]];
}

Vec2f Model::uv(int iface, int nthvert) {
    if (uv_.empty()) return Vec2f(0, 0);
    return uv_[faces_uv_[iface][nthvert]];
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}