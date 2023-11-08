#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

struct Point {
    double x;
    double y;
    Point(){}
    Point(double x_, double y_) : x(x_), y(y_) {}
};

struct BoundingBox {
    Point min;  // 左下角
    Point max;  // 右上角
    BoundingBox(){}
    BoundingBox(Point min_, Point max_) : max(max_), min(min_) {}
};

BoundingBox calculateBoundingBox(const std::vector<Point>& polygon) {
    BoundingBox bbox;
    bbox.min.x = std::numeric_limits<double>::max();
    bbox.min.y = std::numeric_limits<double>::max();
    bbox.max.x = std::numeric_limits<double>::min();
    bbox.max.y = std::numeric_limits<double>::min();

    for (const Point& vertex : polygon) {
        if (vertex.x < bbox.min.x) bbox.min.x = vertex.x;
        if (vertex.y < bbox.min.y) bbox.min.y = vertex.y;
        if (vertex.x > bbox.max.x) bbox.max.x = vertex.x;
        if (vertex.y > bbox.max.y) bbox.max.y = vertex.y;
    }

    return bbox;
}

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

// 将最小包围盒像素化
void pixelizeBoundingBox(const BoundingBox& bbox, int numRows, int numCols) {
    double pixelWidth = (bbox.max.x - bbox.min.x) / numCols;
    double pixelHeight = (bbox.max.y - bbox.min.y) / numRows;

    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            double x = bbox.min.x + (j + 0.5) * pixelWidth;
            double y = bbox.min.y + (i + 0.5) * pixelHeight;

            std::cout << "Pixel Center: (" << x << ", " << y << ")" << std::endl;
        }
    }
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

    // 计算多边形的最小包围盒
    BoundingBox bbox = calculateBoundingBox(polygonCoordinates);

    // 输出最小包围盒的坐标
    std::cout << "Minimum Bounding Box:" << std::endl;
    std::cout << "Min: (" << bbox.min.x << ", " << bbox.min.y << ")" << std::endl;
    std::cout << "Max: (" << bbox.max.x << ", " << bbox.max.y << ")" << std::endl;

    pixelizeBoundingBox(bbox, 20, 30);
    return 0;
}
