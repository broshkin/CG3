//#include <fstream>
//#include <sstream>
//#include <vector>
//#include <string>
//
//struct Vertex {
//    float x, y, z;
//};
//
//int main() {
//    // Открываем файлы
//    std::ifstream input("obj/123.obj");
//    std::ofstream output("obj/1234.obj");
//
//    if (!input.is_open() || !output.is_open()) {
//        return -1; // Ошибка открытия файлов
//    }
//
//    std::vector<Vertex> vertices;
//    std::string line;
//
//    // Чтение вершин и вычисление среднего
//    Vertex center{ 0, 0, 0 };
//    int vertexCount = 0;
//
//    while (std::getline(input, line)) {
//        if (line.substr(0, 2) == "v ") {
//            std::istringstream ss(line.substr(2));
//            Vertex v;
//            ss >> v.x >> v.y >> v.z;
//            vertices.push_back(v);
//
//            // Суммируем для вычисления среднего
//            center.x += v.x;
//            center.y += v.y;
//            center.z += v.z;
//            vertexCount++;
//        }
//    }
//
//    // Вычисляем центр
//    center.x /= vertexCount;
//    center.y /= vertexCount;
//    center.z /= vertexCount;
//
//    // Сброс указателя чтения файла
//    input.clear();
//    input.seekg(0);
//
//    // Обработка и запись
//    while (std::getline(input, line)) {
//        if (line.substr(0, 2) == "v ") {
//            std::istringstream ss(line.substr(2));
//            Vertex v;
//            ss >> v.x >> v.y >> v.z;
//
//            // Центрируем вершину
//            v.x -= center.x;
//            v.y -= center.y;
//            v.z -= center.z;
//
//            output << "v " << v.x << " " << v.y << " " << v.z << "\n";
//        }
//        else {
//            // Копируем остальные строки без изменений
//            output << line << "\n";
//        }
//    }
//
//    input.close();
//    output.close();
//    return 0;
//}