#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) {
    std::ifstream file;
    file.open(filename);

    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream s(line.substr(2));
            Vec3f v;
            s >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (line.substr(0, 2) == "f ") {
            std::istringstream s(line.substr(2));
            std::vector<int> face;
            std::string vertex;
            while (s >> vertex) {
                // OBJ format: "f v1/vt1/vn1 v2/vt2/vn2 ..."
                // We only need the vertex index (first number)
                std::stringstream vstream(vertex);
                std::string index;
                std::getline(vstream, index, '/'); // get only vertex index
                face.push_back(std::stoi(index) - 1);
            }
            faces.push_back(face);
        }
    }

    file.close();
    std::cout << "Loaded " << vertices.size() << " vertices, " << faces.size() << " faces" << std::endl;
}

Model::~Model() {
}

int Model::getVertexCount() {
    return vertices.size();
}

int Model::getFaceCount() {
    return faces.size();
}

Vec3f Model::getVertex(int i) {
    return vertices[i];
}

std::vector<int> Model::getFace(int idx) {
    return faces[idx];
}