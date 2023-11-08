#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <stdio.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#define CHECK(call)                                                         \
    do {                                                                    \
        const cudaError_t error_code = call;                                \
        if (error_code != cudaSuccess) {                                    \
            printf("CUDA ERROR: \n");                                       \
            printf("    FILE: %s\n", __FILE__);                             \
            printf("    LINE: %d\n", __LINE__);                             \
            printf("    ERROR CODE: %d\n", error_code);                     \
            printf("    ERROR TEXT: %s\n", cudaGetErrorString(error_code)); \
            exit(1);                                                        \
        }                                                                   \
    } while (0);

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

__global__ void pixelizeBoundingBoxKernel(const BoundingBox* bbox, int rows, int cols, Point* pixels) {
    int j = threadIdx.x + blockIdx.x * blockDim.x;
    int i = threadIdx.y + blockIdx.y * blockDim.y;
    if (i < rows && j < cols) {
        double pixelWidth = (bbox->max.x - bbox->min.x) / cols;
        double pixelHeight = (bbox->max.y - bbox->min.y) / rows;
        // printf("pixelHeight %.3f %.3f ", bbox->max.x, bbox->min.x);
        pixels[i * cols + j].x = bbox->min.x + (j + 0.5) * pixelWidth;
        pixels[i * cols + j].y = bbox->min.y + (i + 0.5) * pixelHeight;
    }
}

__global__ void pointInPolygonKernel(const Point* pixels, const Point* vertices, int pointsNum, int rows, int cols, int* results) {
    int i = threadIdx.y + blockIdx.y * blockDim.y;
    int j = threadIdx.x + blockIdx.x * blockDim.x;
    // printf("i=%d ", i);
    // printf("j=%d ", j);
    if (i < rows && j < cols) {
        double x = pixels[i * cols + j].x;
        double y = pixels[i * cols + j].y;

        int crossings = 0;
        // printf("enter");
        for (int k = 0; k < pointsNum; k++) {
            double x1 = vertices[k].x;
            double y1 = vertices[k].y;
            double x2 = vertices[(k + 1) % pointsNum].x;
            double y2 = vertices[(k + 1) % pointsNum].y;

            if (((y1 > y) != (y2 > y)) && (x < (x2 - x1) * (y - y1) / (y2 - y1) + x1)) {
                crossings++;
            }
        }

        if (crossings % 2 == 1) {
            results[i * cols + j] = 1;
        } else {
            results[i * cols + j] = 0;
            // printf("%d", results[i * cols + j]);
        }
    }
}

__global__ void output(int rows, int cols, int* results) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d", results[i * cols + j]);
        }
    }
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
    // 从host拷贝所有点数据到device
    Point* devicePolygon;
    int pointNum = polygonCoordinates.size();
    cudaMalloc((void**)&devicePolygon, sizeof(Point) * pointNum);
    cudaMemcpy(devicePolygon, polygonCoordinates.data(), sizeof(Point) * pointNum, cudaMemcpyHostToDevice);
    // 从host拷贝bbox到device
    BoundingBox* deviceBbox;
    cudaMalloc((void**)&deviceBbox, sizeof(BoundingBox));
    cudaMemcpy(deviceBbox, &bbox, sizeof(BoundingBox), cudaMemcpyHostToDevice);

    // 在gpu上执行pixelize
    Point* devicePixels;
    int rows = 20;
    int cols = 30;
    cudaMalloc((void**)&devicePixels, rows * cols * sizeof(Point));
    // 在gpu上执行像素化
    dim3 pixelBlockSize(16, 16);
    dim3 pixelGridSize((cols + pixelBlockSize.x - 1) / pixelBlockSize.x, (rows + pixelBlockSize.y - 1) / pixelBlockSize.y);
    pixelizeBoundingBoxKernel<<<pixelGridSize, pixelBlockSize>>>(deviceBbox, rows, cols, devicePixels);

    Point* hostPixels = new Point[rows * cols];
    cudaMemcpy(hostPixels, devicePixels, rows * cols * sizeof(Point), cudaMemcpyDeviceToHost);
    CHECK(cudaGetLastError());
    CHECK(cudaDeviceSynchronize());
    int* hostResults = new int[rows * cols];
    // 在gpu上判断是否在多边形内
    int* deviceResults;
    cudaMalloc((void**)&deviceResults, rows * cols * sizeof(int));
    pointInPolygonKernel<<<pixelGridSize, pixelBlockSize>>>(devicePixels, devicePolygon, pointNum, rows, cols, deviceResults);
    CHECK(cudaGetLastError());
    CHECK(cudaDeviceSynchronize());
    cudaMemcpy(hostResults, deviceResults, rows * cols * sizeof(int), cudaMemcpyDeviceToHost);
    // output<<<1, 1>>>(rows, cols, deviceResults);
    CHECK(cudaGetLastError());
    CHECK(cudaDeviceSynchronize());

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);

    // 输出像素点的坐标
    // for (int i = 0; i < rows; ++i) {
    //     for (int j = 0; j < cols; ++j) {
    //         if (hostResults[i * cols + j]) {
    //             Point pixel = hostPixels[i * cols + j];
    //             std::cout << "Pixel Center: (" << pixel.x << ", " << pixel.y << ")" << std::endl;
    //         }
    //     }
    // }

    cudaDeviceSynchronize();
    // 释放设备内存
    cudaFree(devicePolygon);
    cudaFree(deviceBbox);
    cudaFree(devicePixels);
    cudaFree(deviceResults);
    free(hostPixels);
    free(hostResults);
    return 0;
}
