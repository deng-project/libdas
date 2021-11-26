#include <cstdint>
#include <utility>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <cfloat>
using size_t = std::size_t;

#include <Vector.h>
#include <Matrix.h>
#include <DasStructures.h>

template <typename Matrix>
void OutputMatrix(const Matrix &_mat, int n) {
    int i = 0;
    std::cout << "row major order:" << std::endl;
    for(auto it = _mat.BeginRowMajor(); it != _mat.EndRowMajor(); it++) {
        std::cout << *it << " ";
        i++;
        if(i >= n && i % n == 0) std::cout << std::endl;
    }

    std::cout << std::endl;

    i = 0;
    std::cout << std::endl << "column major order:" << std::endl;
    for(auto it = _mat.BeginColumnMajor(); it != _mat.EndColumnMajor(); it++) {
        std::cout << *it << " ";
        i++;
        if(i >= n && i % n == 0) std::cout << std::endl;
    }

    std::cout << std::endl;
}


int main() {
    Libdas::Matrix4<float> mat4;
    mat4.row1.fourth = FLT_MAX;
    mat4.row4.first = FLT_MIN;

    Libdas::Matrix3<float> mat3;
    mat3.row1.third = FLT_MAX;
    mat3.row3.first = FLT_MIN;

    Libdas::Matrix2<float> mat2;
    mat2.row1.second = FLT_MAX;
    mat2.row2.first = FLT_MIN;

    OutputMatrix(mat2, 2);
    OutputMatrix(mat3, 3);
    OutputMatrix(mat4, 4);

    return 0;
}
