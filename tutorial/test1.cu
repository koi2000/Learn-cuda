#include <cstdio>
using namespace std;

__global__ void hello_from_gpu() {
    const int bx = blockIdx.x;
    const int by = blockIdx.y;
    const int bz = blockIdx.z;
}

int main() {
    const dim3 block_size(2, 4);
    // grid的维度，说明了block的数量
    // block的维度，说明了thread的数量
    hello_from_gpu<<<2, block_size>>>();
    cudaDeviceSynchronize();
}