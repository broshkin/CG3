#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
    std::vector<Vec3f> vertices;
    std::vector<std::vector<int>> faces;

public:
    Model(const char* filename);
    ~Model();

    int getVertexCount();
    int getFaceCount();
    Vec3f getVertex(int i);
    std::vector<int> getFace(int idx);
};

#endif