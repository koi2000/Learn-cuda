#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
struct Point {
    double x;
    double y;
    Point(double x_, double y_) : x(x_), y(y_) {}
};
// 函数用于将字符串中的坐标提取到数组中
std::vector<Point> extractCoordinates(const std::string& wkt) {
    std::vector<Point> polygonCoordinates;
    size_t start = wkt.find("((") + 2;  // 寻找坐标开始的位置
    size_t end = wkt.find("))");        // 寻找坐标结束的位置

    if (start != std::string::npos && end != std::string::npos) {
        std::string coordinatesStr = wkt.substr(start, end - start);  // 提取坐标部分
        std::istringstream iss(coordinatesStr);
        std::vector<double> coordinates;
        double coordinate;

        while (iss >> coordinate) {
            coordinates.push_back(coordinate);

            // 当遇到逗号时，将当前点坐标存入数组，然后清空 coordinates
            if (iss.peek() == ',') {
                iss.ignore();
                polygonCoordinates.push_back(Point(coordinates[0], coordinates[1]));
                coordinates.clear();
            }
        }

        // 最后一个点坐标
        if (!coordinates.empty()) {
            polygonCoordinates.push_back(Point(coordinates[0], coordinates[1]));
        }
    }

    return polygonCoordinates;
}

int main() {
    // 文件路径
    std::string filename = "../bigpolygon.wkt";

    // 创建文件输入流对象
    std::ifstream inputFile(filename);

    // 检查文件是否成功打开
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }

    std::vector<Point> polygonCoordinates;

    // 逐行读取文件内容
    std::string line;
    while (std::getline(inputFile, line)) {
        // 提取坐标并存储到数组中
        std::vector<Point> coordinates = extractCoordinates(line);
        polygonCoordinates.insert(polygonCoordinates.end(), coordinates.begin(), coordinates.end());
    }

    // 关闭文件
    inputFile.close();

    // 输出提取的坐标
    for (const auto& polygon : polygonCoordinates) {
        std::cout << polygon.x << " " << polygon.y << std::endl;
    }

    return 0;
}
