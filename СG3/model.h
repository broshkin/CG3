#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    std::vector<std::vector<int> > faces_;
    std::vector<std::vector<int> > faces_norms_;
    std::vector<std::vector<int> > faces_uv_;

public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec3f normal(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    std::vector<int> face(int idx);
};

#endif