#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

struct Point {
    double x;
    double y;
    Point() {}
    Point(double x_, double y_) : x(x_), y(y_) {}
};

struct BoundingBox {
    Point min;
    Point max;
    BoundingBox() {}
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

bool isIntersect(const Point& p1, const Point& p2, const Point& testPoint) {
    if ((p1.y > testPoint.y) != (p2.y > testPoint.y) &&
        (testPoint.x < (p2.x - p1.x) * (testPoint.y - p1.y) / (p2.y - p1.y) + p1.x)) {
        return true;
    }
    return false;
}

bool isPointInPolygon(const Point& testPoint, const std::vector<Point>& polygon) {
    int numIntersections = 0;
    size_t n = polygon.size();

    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = polygon[i];
        const Point& p2 = polygon[(i + 1) % n];

        if (isIntersect(p1, p2, testPoint)) {
            numIntersections++;
        }
    }

    return (numIntersections % 2 == 1);
}

std::vector<Point> extractCoordinates(const std::string& wkt) {
    std::vector<Point> polygonCoordinates;
    size_t start = wkt.find("((") + 2;
    size_t end = wkt.find("))");

    if (start != std::string::npos && end != std::string::npos) {
        std::string coordinatesStr = wkt.substr(start, end - start);  // 提取坐标部分
        std::istringstream iss(coordinatesStr);
        std::vector<double> coordinates;
        double coordinate;
        while (iss >> coordinate) {
            coordinates.push_back(coordinate);

            if (iss.peek() == ',') {
                iss.ignore();
                polygonCoordinates.push_back(Point(coordinates[0], coordinates[1]));
                coordinates.clear();
            }
        }
        if (!coordinates.empty()) {
            polygonCoordinates.push_back(Point(coordinates[0], coordinates[1]));
        }
    }

    return polygonCoordinates;
}

std::vector<Point> pixelizeBoundingBox(const BoundingBox& bbox, int numRows, int numCols) {
    double pixelWidth = (bbox.max.x - bbox.min.x) / numCols;
    double pixelHeight = (bbox.max.y - bbox.min.y) / numRows;
    std::vector<Point> pixels;
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            double x = bbox.min.x + (j + 0.5) * pixelWidth;
            double y = bbox.min.y + (i + 0.5) * pixelHeight;
            pixels.emplace_back(x, y);
        }
    }
    return pixels;
}

int main() {
    std::string filename = "../bigpolygon.wkt";

    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }
    std::vector<Point> polygonCoordinates;

    std::string line;
    while (std::getline(inputFile, line)) {
        std::vector<Point> coordinates = extractCoordinates(line);
        polygonCoordinates.insert(polygonCoordinates.end(), coordinates.begin(), coordinates.end());
    }

    inputFile.close();

    BoundingBox bbox = calculateBoundingBox(polygonCoordinates);
    auto begin = std::chrono::high_resolution_clock::now();
    std::vector<Point> pixels = pixelizeBoundingBox(bbox, 20, 30);
    for (Point& point : pixels) {
        if (isPointInPolygon(point, polygonCoordinates)) {
            // std::cout << "Pixel Center: (" << point.x << ", " << point.y << ")" << std::endl;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);
    return 0;
}
