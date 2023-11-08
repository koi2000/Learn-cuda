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

// 检测射线与边的交点
bool isIntersect(const Point& p1, const Point& p2, const Point& testPoint) {
    if ((p1.y > testPoint.y) != (p2.y > testPoint.y) &&
        (testPoint.x < (p2.x - p1.x) * (testPoint.y - p1.y) / (p2.y - p1.y) + p1.x)) {
        return true;
    }
    return false;
}

// 判断点是否在多边形内部
bool isPointInPolygon(const Point& testPoint, const std::vector<Point>& polygon) {
    int numIntersections = 0;
    size_t n = polygon.size();

    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = polygon[i];
        const Point& p2 = polygon[(i + 1) % n];  // 下一个顶点

        if (isIntersect(p1, p2, testPoint)) {
            numIntersections++;
        }
    }

    return (numIntersections % 2 == 1);  // 奇数交点表示点在多边形内部
}

// 函数用于将字符串中的坐标提取到数组中
std::vector<std::vector<double>> extractCoordinates(const std::string& wkt) {
    std::vector<std::vector<double>> polygonCoordinates;
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
                polygonCoordinates.push_back(coordinates);
                coordinates.clear();
            }
        }

        // 最后一个点坐标
        if (!coordinates.empty()) {
            polygonCoordinates.push_back(coordinates);
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

    std::vector<std::vector<double>> polygonCoordinates;

    // 逐行读取文件内容
    std::string line;
    while (std::getline(inputFile, line)) {
        // 提取坐标并存储到数组中
        std::vector<std::vector<double>> coordinates = extractCoordinates(line);
        polygonCoordinates.insert(polygonCoordinates.end(), coordinates.begin(), coordinates.end());
    }

    // 关闭文件
    inputFile.close();

    // 输出提取的坐标
    for (const auto& polygon : polygonCoordinates) {
        for (double coordinate : polygon) {
            std::cout << coordinate << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
