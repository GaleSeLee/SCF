#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <mpi.h>

#include "ttvc.h"
#include "offload_task.h"

void offload_task(Tensor *A, Tensor *U) {
    extern int mpi_rank;
    int ndim = A->ndim;
    std::vector<std::vector<int>> block_time;
    int block_num = 0;
    for (int ii = 0; ii < ndim; ii++)
        block_num += ii;

    int ttvc_time_buffer[block_num*3];
    int idx_buffer = 0;

    int total_time = 0;
    if (mpi_rank == 0) {
        for (int ii = 0; ii < ndim - 1; ii++) {
            for (int jj = ii+1; jj < ndim; jj++) {
                Tensor block({U[ii].size, U[jj].size});
                auto start = std::chrono::system_clock::now();
                ttvc_except_dim_mpi(A, U, block.data, ii, jj);
                auto end = std::chrono::system_clock::now();
                std::vector<int> elem{ii, jj,
                    static_cast<int>(
                    std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()
                    / 1000.0)
                };
                ttvc_time_buffer[idx_buffer++] = elem[0];
                ttvc_time_buffer[idx_buffer++] = elem[1];
                ttvc_time_buffer[idx_buffer++] = elem[2];
                total_time += elem[2];
                block_time.push_back(elem);
            }
        }
        MPI_Send(ttvc_time_buffer, block_num * 3, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        if (mpi_rank != 1) {
            std::cout << "Only support 2 processes" << std::endl;
            exit(1);
        }
        MPI_Recv(ttvc_time_buffer, block_num * 3, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int ii = 0; ii < block_num; ii++) {
                std::vector<int> elem{ttvc_time_buffer[ii*3],
                    ttvc_time_buffer[ii*3+1],
                    ttvc_time_buffer[ii*3+2]}; 
                block_time.push_back(elem);
                total_time += elem[2];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // std::cout << "finish ttvc" << std::endl;
    // 20000 is greater than half_time
    int dp[28][20000][2] = {0};

    int half_time = total_time / 2;
    for(int jj = 0; jj < half_time + 1; jj++) {
        for (int ii = 0; ii < block_num; ii++) {
            if (ii == 0) {
                if (jj >= block_time[ii][2]) {
                    dp[ii][jj][0] = block_time[ii][2];
                    dp[ii][jj][1] = 1;
                } else {
                    dp[ii][jj][0] = 0;
                    dp[ii][jj][1] = 0;
                }
                continue;
            }

            int val0 = dp[ii-1][jj][0];
            int val1 = 0;
            if (jj > block_time[ii][2]) {
                val1 = dp[ii-1][jj-block_time[ii][2]][0] + block_time[ii][2];
            }
            if (val1 >= val0) {
                dp[ii][jj][0] = val1;
                dp[ii][jj][1] = 1;
            } else {
                dp[ii][jj][0] = val0;
                dp[ii][jj][1] = 0;
            }
        }
    }
    std::vector<std::vector<int>> task_cpu0;
    std::vector<std::vector<int>> task_cpu1;
    int time_idx = half_time;
    for (int ii = block_num - 1; ii >= 0; ii--) {
        if (dp[ii][time_idx][1]) {
            task_cpu0.push_back(block_time[ii]);
            time_idx -= block_time[ii][2];
        } else {
            task_cpu1.push_back(block_time[ii]);
        }
    }
    tasks_list.push_back(task_cpu0);
    tasks_list.push_back(task_cpu1);
    rank_offset.push_back(0);
    int offset = 0;
    for (int ii = 0; ii < task_cpu0.size(); ii++) {
        offset += U[task_cpu0[ii][0]].size * U[task_cpu0[ii][1]].size;
    }
    rank_offset.push_back(offset);
}