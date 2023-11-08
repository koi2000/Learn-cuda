#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// 函数用于将字符串中的坐标提取到数组中
std::vector<double> extractCoordinates(const std::string& wkt) {
    std::vector<double> coordinates;
    size_t start = wkt.find("(") + 1;  // 寻找坐标开始的位置
    size_t end = wkt.find(")");        // 寻找坐标结束的位置

    if (start != std::string::npos && end != std::string::npos) {
        std::string coordinatesStr = wkt.substr(start, end - start);  // 提取坐标部分
        std::istringstream iss(coordinatesStr);
        double coordinate;
        while (iss >> coordinate) {
            coordinates.push_back(coordinate);
        }
    }

    return coordinates;
}

int main() {
    // 文件路径
    std::string filename = "bigpolygon.wkt";

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
        std::vector<double> coordinates = extractCoordinates(line);
        if (!coordinates.empty()) {
            polygonCoordinates.push_back(coordinates);
        }
    }

    // 关闭文件
    inputFile.close();

    // 输出提取的坐标
    for (const auto& coordinates : polygonCoordinates) {
        for (double coordinate : coordinates) {
            std::cout << coordinate << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
