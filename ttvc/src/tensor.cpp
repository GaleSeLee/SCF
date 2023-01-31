#include <cstring>
#include <cmath>
#include <algorithm>

#include "tensor.h"

Tensor::Tensor() {
    data = nullptr;
}

Tensor::Tensor(vint shape_in) {
    shape = shape_in;
    size = 1;
    ndim = shape.size();
    for (int ii = 0; ii < ndim; ii++) 
        size *= shape[ii];
    data = reinterpret_cast<double *>(malloc(sizeof(double) * size));
    std::memset(data, 0, sizeof(double) * size);
}

Tensor::~Tensor() {
    if (data != nullptr) {
        free(data);
    }
}

void Tensor::constructor(vint shape_in) {
    shape = shape_in;
    size = 1;
    ndim = shape.size();
    for (int ii = 0; ii < ndim; ii++) 
        size *= shape[ii];
    data = reinterpret_cast<double *>(malloc(sizeof(double) * size));
    std::memset(data, 0, sizeof(double) * size);
}

void Tensor::norm() {
    double sum = 0;
    for (int ii = 0; ii < size; ii++) {
        sum += data[ii] * data[ii];
    }
    sum = std::sqrt(sum);
    for (int ii = 0; ii < size; ii++) {
        data[ii] /= sum;
    }
}