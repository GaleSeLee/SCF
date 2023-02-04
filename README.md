1. Here is the scalability result of single ttvc operation on the single numa node
```
scale: 32 x 32 x 32 x 32 x 32 x 32
```
| Threads    | Time/ms | scalability(based on num_thread=1)|
| -------- | ------- | --------------|
| 1 | 35338  | - |
| 2 | 17916.3 | 98.61%|
| 4 | 8895.6 | 99.31% |
| 8 | 4657.5 | 94.84%|
| 16 | 2326.4 | 94.94%|
| 32 | 1138.6  | 96.99%
| 64 | 697.85 | 79.12%|
2. scf on s single numa node(without O3 optimization)
| Threads    | Time/ms | scalability(based on num_thread=1)|
| -------- | ------- | --------------|
| 1 | 1.9361e+06 | - |
| 2 | 1.00471e+06 |  |
| 4 |   |  |
| 8 |   |  |
| 16 |  |  |
| 32 | 77877.9 |   |
| 64 |  56695.4 | |