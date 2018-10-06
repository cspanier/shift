# shift.math

## Vectors
Vectors are always considered/written as column vectors  
![\vec{v} := \begin{pmatrix} v_{x} \\ v_{y} \\ v_{z} \\ v_{w} \end{pmatrix}](https://latex.codecogs.com/gif.latex?%5Cvec%7Bv%7D%20%3A%3D%20%5Cbegin%7Bpmatrix%7D%20v_%7Bx%7D%20%5C%5C%20v_%7By%7D%20%5C%5C%20v_%7Bz%7D%20%5C%5C%20v_%7Bw%7D%20%5Cend%7Bpmatrix%7D)

As such we have to multiply a vector with some matrix from right to left:
![\vec{w} = M \vec{v}](https://latex.codecogs.com/gif.latex?%5Cvec%7Bw%7D%20%3D%20M%20%5Cvec%7Bv%7D)

You can easily construct vectors using the helper function make_vector_from:
```c++
// This constructs a four dimensional vector of integers.
auto v = make_vector_from(0, 1,  2, 3);
```

## Matrices
All matrices are stored in column major order:  
![M := \begin{pmatrix} M_{00} & M_{04} & M_{08} & M_{12} \\ M_{01} & M_{05} & M_{09} & M_{13} \\ M_{02} & M_{06} & M_{10} & M_{14} \\ M_{03} & M_{07} & M_{11} & M_{15} \end{pmatrix}](https://latex.codecogs.com/gif.latex?M%20%3A%3D%20%5Cbegin%7Bpmatrix%7D%20M_%7B00%7D%20%26%20M_%7B04%7D%20%26%20M_%7B08%7D%20%26%20M_%7B12%7D%20%5C%5C%20M_%7B01%7D%20%26%20M_%7B05%7D%20%26%20M_%7B09%7D%20%26%20M_%7B13%7D%20%5C%5C%20M_%7B02%7D%20%26%20M_%7B06%7D%20%26%20M_%7B10%7D%20%26%20M_%7B14%7D%20%5C%5C%20M_%7B03%7D%20%26%20M_%7B07%7D%20%26%20M_%7B11%7D%20%26%20M_%7B15%7D%20%5Cend%7Bpmatrix%7D)

The memory layout of this matrix is ![\begin{bmatrix} M_{00} & M_{01} & M_{02} & M_{03} & M_{04} & \cdots & M_{14} & M_{15} \end{bmatrix}](https://latex.codecogs.com/gif.latex?%5Cbegin%7Bbmatrix%7D%20M_%7B00%7D%20%26%20M_%7B01%7D%20%26%20M_%7B02%7D%20%26%20M_%7B03%7D%20%26%20M_%7B04%7D%20%26%20%5Ccdots%20%26%20M_%7B14%7D%20%26%20M_%7B15%7D%20%5Cend%7Bbmatrix%7D)

It is important to note that matrices in source code are likely to be written in transposed form:
```c++
auto m = make_matrix_from_column_major<4, 4, int>(
   0,  1,  2,  3,
   4,  5,  6,  7,
   8,  9, 10, 11,
  12, 13, 14, 15);
```
To avoid confusion in some instances you may also use the alternative function `make_matrix_from_row_major`. It constructs a matrix from arguments in row major order but still stores all data in column major:
```c++
auto m = make_matrix_from_row_major<4, 4, int>(
   0,  4,  8, 12,
   1,  5,  9, 13,
   2,  6, 10, 14,
   3,  7, 11, 15);
```
