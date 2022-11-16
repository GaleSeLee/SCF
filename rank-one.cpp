
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <chrono>
#include <tuple>
#include <cstring>
#include <string>
#include <cmath>
#include <map>


/*MKL*/
#include "mkl_cblas.h"
#include "mkl_lapacke.h"

typedef std::vector<int> vint;

std::map<double *, int> ref_count;
/* struct */
struct Tensor {
    vint shape;
    int size;
    int ndim;
    double *data = nullptr;
    void copy(Tensor &b) {
        if(ref_count[this->data] == 0 && this->data != nullptr) {
            std::free(this->data);
        }
        shape = {};
        for(int ii = 0; ii < b.shape.size(); ii++) {
            int t = b.shape[ii];
            this->shape.push_back(t);
        }
        this->size = b.size;
        this->ndim = b.ndim;
        this->data = (double *)std::malloc(sizeof(double) * b.size);
        ref_count[this->data] = 1;
        std::memcpy(this->data, b.data, sizeof(double) * this->size);
    }
    Tensor& operator=(const Tensor& b)
    {
        this->size = b.size;
        this->shape = {};
        for(int ii = 0; ii < b.shape.size(); ii++) {
            int t = b.shape[ii];
            this->shape.push_back(t);
        }
        this->ndim = b.ndim;
        if(this->data!=nullptr) ref_count[this->data]--;
        if (ref_count[this->data] == 0 && this->data != nullptr) { 
            ref_count.erase(this->data);
        }
        this->data = b.data;
        if (this->data != nullptr) { ref_count[this->data]++; }
        return *this;
    }
    Tensor(const Tensor& b)//复制构造函数
    {
        *this = b;
    }
    Tensor() {
        this->data = nullptr;
    }
    ~Tensor() {
        if(this->data!=nullptr){
            ref_count[this->data]--;
            if (ref_count[this->data] == 0)  {
                std::free(this->data);
                ref_count.erase(this->data);
            }
        }
    }
};

/* time */
std::chrono::system_clock::time_point tnow() {
    return std::chrono::system_clock::now();
}

void pti(std::chrono::system_clock::time_point time) {
    auto now_ = tnow();
    auto time_span = std::chrono::duration_cast<std::chrono::milliseconds >(now_ - time);
    auto pt = time_span.count();
    std::cout<<"time/ms = "<<std::to_string(pt)<<std::endl;
} 


double randn() {
    double u = ((double)rand() / (RAND_MAX)) * 2 - 1;
    double v = ((double)rand() / (RAND_MAX)) * 2 - 1;
    double r = u * u + v * v;
    if (r == 0 || r > 1)
        return randn();
    double c = std::sqrt(-2 * std::log(r) / r);
    return u * c;
}

double fnorm(Tensor &A) {
    int n = A.size;
    double norm = 0;
    for (size_t i = 0; i < n; i++) {
        norm += A.data[i] * A.data[i];
    }
    return std::sqrt(norm);
}

void Nmul(Tensor &A, double num) {
    for(int ii = 0; ii < A.size; ii++) {
        A.data[ii] *= num;
    }
}

double cal_lambda(Tensor &A, std::vector<Tensor> &U) {
    int n = A.ndim;
    vint shape = A.shape;
    int lambda = 0;
    int scan[6];
    scan[5] = 1;
    for (int ii = 4; ii >= 0; ii++) {
        scan[ii] = scan[ii+1] * shape[ii+1];
    }

    for (int ii = 0; ii < shape[0]; ii++) {
        int idx_ii = ii * scan[0];
        for (int jj = 0; jj < shape[1]; jj++) {
            int idx_jj = jj * scan[1] + idx_ii;
            for (int kk = 0; kk < shape[2]; kk++) {
                int idx_kk = kk * scan[2] + idx_jj;
                for (int ll = 0; ll < shape[3]; ll++) {
                    int idx_ll = ll * scan[3] + idx_kk;
                    for (int uu = 0; uu < shape[4]; uu++) {
                        int idx_uu = uu * scan[4] + idx_ll;
                        for (int tt = 0; tt < shape[5]; tt++) {
                            lambda += A[idx_uu + tt] * U[0][ii] * U[1][jj]
                                      * U[2][kk] * U[3][ll] * U[4][uu] * U[5][tt];
                        }
                    }
                }
            }
        }
    }
    return lambda;
}

Tensor& ttvc_except_dim(Tensor &A, std::vector<Tensor> &U, int dim0, int dim1) {
    Tensor block_J;
    block_J.size = A.shape[dim0] * A.shape[dim1];
    block_J.ndim = 2;
    block_J.shape = {A.shape[dim0] * A.shape[dim1]};
    block_J.data = 
    int dim[4];
    int cnt = 0;
    for (int ii = 0; ii < n; ii++) {
        if (ii == dim0 || ii == dim1) continue;
        dim[cnt++] = ii;
    }
    int scan[6];
    scan[5] = 1;
    for (int ii = 4; ii >= 0; ii++) {
        scan[ii] = scan[ii+1] * shape[ii+1];
    }

    for (int ii = 0; ii < shape[dim0]; ii++) {
        int idx_ii = ii * scan[dim0];
        int block_idx_ii = ii * shape[dim1];
        for (int jj = 0; jj < shape[dim1]; jj++) {
            int idx_jj = jj * scan[dim1] + idx_ii;
            int block_idx = block_idx_ii + jj;
            for (int kk = 0; kk < shape[dim[0]]; kk++) {
                int idx_kk = kk * scan[dim[0] + idx_jj;
                for (int ll = 0; ll < shape[dim[1]]; ll++) {
                    int idx_ll = ll * scan[dim[1]] + idx_kk;
                    for (int uu = 0; uu < shape[dim[2]]; uu++) {
                        int idx_uu = uu * scan[dim[2]] + idx_ll;
                        for (int tt = 0; tt < shape[dim[3]]; tt++) {
                            block_J.data[block_idx] += 
                                A[idx_uu + tt]* U[dim[0]][kk] * U[dim[1]][ll]
                                * U[dim[2]][uu] * U[dim[3]][tt];
                        }
                    }
                }
            }
        }
    }

    return block_J;
}

std::tuple<Tensor&, double> svd_solve(Tensor &J) { // evg
// dsyev
    

}

double cal_res(Tensor& J, Tensor &X) {

}

double scf(Tensor &A, std::vector<Tensor> &U, double tol, int max_iter) {
    int n = A.ndim;
    vint shape = A.shape;
    double AF = fnorm(A);
    int iter = 0;
    int n_j = 0;
    int scan_nj[7];
    scan_nj[0] = 0;
    for (int ii = 0; ii < n; ii++) {
        n_j += shape[ii];
        scan_nj[ii+1] = n_j;
    }

    Tensor J;
    Tensor X;
    J.size = n_j * n_j;
    J.ndim = 2;
    J.shape={n_j, n_j};
    J.data = (double *)std::malloc(sizeof(double) * J.size);
    std::memset(J.data, 0, sizeof(double) * J.size);
    ref_count[J.data] ++;
    X.size = n_j;
    X.ndim = 1;
    X.shape = {X.size};
    X.data = (double *)std::malloc(sizeof(double) * X.size);
    for (int ii = 0; ii < n; ii++) {
        std::memcpy(X.data + sizeof(double) * scan_nj[ii];
                    U[ii].data, shape[ii] * sizeof(double));
    }
    ref_count[X.data] ++;
    auto lambda = cal_lambda(J, X);

    while (iter < max_iter) {
        // omp
        for (int ii = 1; ii < n - 1; ii++) {
            for (int jj = ii + 1; jj < n; jj++) {
                auto block_J = ttvc_except_dim(A, U, ii, jj);
                Nmul(block_J, n - 1);
                // TODO
                // push block_J / (N-1) to A
            }
        }
        Nmul(X, fnorm(X));

        // cal residual        
        auto res = cal_res(J, X);
        if (res < tol) {
            break;
        }
        
        iter++;

        // update U
        // lambda X 赋值问题
        auto [eigvec, vec] = svd_solve(J);

    }
    // assign U
}

bool test_svd() {

}

bool test_ttv_except_dim(Tensor &A, std::vector<Tensor> &U) {

}

bool test_ttv_cal_lambda(Tensor &A, std::vector<Tensor> &U) {

}

bool test_cal_res() {

}

int main() {
    vint shapeA = {100,10,10,10,10,10};
    int ndim = shapeA.size();
    Tensor A;
    for (int ii = 0; ii < ndim; ii++) {
        A.shape.push_back(shapeA[ii]);
    }
    A.ndim = A.shape.size();
    A.size = 1;
    for(int ii = 0; ii < ndim; ii++)
        A.size *= A.shape[ii];
    A.data = (double *)std::malloc(sizeof(double) * A.size);
    ref_count[A.data] = 1;

    for(int ii = 0; ii < A.shape[0]; ii++) {
        for(int jj = 0; jj < A.shape[1]; jj++)
            for(int kk = 0; kk < A.shape[2]; kk++)
                for(int ll = 0; ll < A.shape[3];ll++)
                A.data[ii*A.shape[1]*A.shape[2]*A.shape[3]+jj*A.shape[2]*A.shape[3]+kk*A.shape[3]+ll] 
                    = std::sin(ii+jj+kk+ll);
    }

    std::vector<Tensor> U;
    for(int ii = 0; ii < ndim; ii++) {
        Tensor u;
        u.ndim = 1;
        u.size = shapeA[ii];
        u.shape.push_back(shapeA[ii]);
        u.data = (double*)std::malloc(sizeof(double) * u.size);
        ref_count[u.data] ++;
        for(int jj = 0; jj < u.size; jj++)
            u.data[jj] = randn();
        double a = fnorm(u);
        for(int jj = 0; jj < u.size; jj++)
            u.data[jj] /= a;
        U.push_back(u);
    }
    if (test_ttv_except_dim(A, U)) {
        std::cout << "Error ttv_except_dim" << std::endl;
        exit(1);
    }
    if (test_ttv_cal_lambda(A, U)) {
        std::cout << "Error ttv_cal_lambda" << std::endl;
        exit(1);
    }
    if (test_svd()) {
        std::cout << "Error svd" << std::endl;
        exit(1);
    }
    if (test_cal_res()) {
        std::cout << "Error cal_res" << std::endl;
        exit(1);
    }

}

// int main(int argc, char **argv) {
//     vint shapeA = {100,10,10,10,10,10};
//     int ndim = shapeA.size();
//     Tensor A;
//     for (int ii = 0; ii < ndim; ii++) {
//         A.shape.push_back(shapeA[ii]);
//     }
//     A.ndim = A.shape.size();
//     A.size = 1;
//     for(int ii = 0; ii < ndim; ii++)
//         A.size *= A.shape[ii];
//     A.data = (double *)std::malloc(sizeof(double) * A.size);
//     ref_count[A.data] = 1;

//     for(int ii = 0; ii < A.shape[0]; ii++) {
//         for(int jj = 0; jj < A.shape[1]; jj++)
//             for(int kk = 0; kk < A.shape[2]; kk++)
//                 for(int ll = 0; ll < A.shape[3];ll++)
//                 A.data[ii*A.shape[1]*A.shape[2]*A.shape[3]+jj*A.shape[2]*A.shape[3]+kk*A.shape[3]+ll] 
//                     = std::sin(ii+jj+kk+ll);
//     }
    
//     std::vector<Tensor> U;
//     for(int ii = 0; ii < ndim; ii++) {
//         Tensor u;
//         u.ndim = 1;
//         u.size = shapeA[ii];
//         u.shape.push_back(shapeA[ii]);
//         u.data = (double*)std::malloc(sizeof(double) * u.size);
//         ref_count[u.data] ++;
//         for(int jj = 0; jj < u.size; jj++)
//             u.data[jj] = randn();
//         double a = fnorm(u);
//         for(int jj = 0; jj < u.size; jj++)
//             u.data[jj] /= a;
//         U.push_back(u);
//     }
//     auto tt = tnow();
//     double lamb = scf(A, U, 1e-12, 100);
//     pti(tt);
// }
