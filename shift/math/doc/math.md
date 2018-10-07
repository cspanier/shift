# shift.math

## Vectors

Vectors are always considered/written as column vectors  
![\vec{v} := \begin{pmatrix} v_{x} \\ v_{y} \\ v_{z} \\ v_{w} \end{pmatrix}](vector.gif)

As such we have to multiply a vector with some matrix from right to left:
![\vec{w} = M \vec{v}](mul_matrix_vector.gif)

You can easily construct vectors using the helper function `make_vector_from`:
```c++
// This constructs a four dimensional vector of integers.
auto v = make_vector_from(0, 1,  2, 3);
```

## Matrices

All matrices are stored in column major order:  
![M := \begin{pmatrix} M_{00} & M_{04} & M_{08} & M_{12} \\ M_{01} & M_{05} & M_{09} & M_{13} \\ M_{02} & M_{06} & M_{10} & M_{14} \\ M_{03} & M_{07} & M_{11} & M_{15} \end{pmatrix}](matrix_column_major.gif)

The memory layout of this matrix is ![\begin{bmatrix} M_{00} & M_{01} & M_{02} & M_{03} & M_{04} & \cdots & M_{14} & M_{15} \end{bmatrix}](mul_matrix_vector.gif)

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

## Quaternions

(ToDo)

## Math in Vulkan

We consistently use a right handed coordinate system to store model and scene data. The x-y axis span a horizontal plane whereas the z axis points upwards.
### Transformations

#### Object Space
Local coordinates ![\vec{v}_{object}](v_object.gif) in object space are those stored in vertex buffers.

#### World Space
By multiplying each ![\vec{v}_{object}](v_object.gif) with a model matrix ![M_{model}](m_model.gif) you get world space coordinates ![\vec{v}_{world}](v_world.gif):

![\vec{v}_{world} = M_{model} \vec{v}_{object}](m_model_times_v_object.gif)

#### View Space
From world space each vector ![\vec{v}_{world}](v_world.gif) needs to be transformed into the virtual camera's view space:

![\vec{v}_{view} = M_{view} \vec{v}_{world}](m_view_times_v_world.gif)

A view matrix ![M_{view}](m_view.gif) is nothing more than the inverse of the model matrix of the virtual camera object:

![M_{view} = M_{model,camera}^{-1}](m_view_1.gif)

However, some view matrices can be computed without using an expensive inverse function:

![M_{view} = M_{rotation}^T M_{nt}](m_view_2.gif)

with ![M_{rotation}^T](m_rot_transpose.gif) being the transpose of the camera's rotation matrix (which is cheap to calculate) and ![M_{nt}](m_nt.gif) being a translation matrix of the negative camera position (moving the camera in one direction is like moving the world in the opposite direction).

#### Clip Space
By transforming view space using a projection matrix ![M_{proj}](m_proj.gif) we get homogeneous device coordinates (HDC) ![\vec{v}_{clip}](v_clip.gif):

![\vec{v}_{clip} = M_{proj} \vec{v}_{view}](v_clip_1.gif)

A vector ![\vec{v}_{clip}](v_clip.gif) passes clip space if and only if all of the following conditions are met:

![\\ -\vec{v}_w <= \vec{v}_x <= \vec{v}_w \\ -\vec{v}_w <= \vec{v}_y <= \vec{v}_w \\ 0 <= \vec{v}_z <= \vec{v}_w](v_clip_conditions.gif)

#### Normalized Device Coordinate Space
Each vector that passes clip space is transformed from homogeneous device coordinates to normalized device coordinates (NDC) by dividing the x, y, and z components with the fourth w component.

![\vec{v}_{ndc} = \vec{v}_{clip,xyz} / \vec{v}_{clip,w}](v_ndc_1.gif)

The resulting components are used for bounding, storage, and testing.

#### Screen/Window/Framebuffer Space
Finally ![\vec{v}_{ndc}](v_ndc.gif) is transformed into screen/window/framebuffer space by multiplying the x and y components with the target size:

![\vec{v}_{framebuffer} = \begin{pmatrix} \frac{\vec{v}_{ndc,x} + 1}{2} \\ \frac{\vec{v}_{ndc,y} + 1}{2} \\ \vec{v}_{ndc,z} \end{pmatrix} \begin{pmatrix} framebuffer{width} \\ framebuffer{height} \\ 1 \end{pmatrix}](v_framebuffer_1.gif)

We usually don't get ![\vec{v}_{framebuffer}](v_framebuffer.gif) automatically and have to compute it separately if we need pixel coordinates.

### Matrices

#### Projection Matrices
The projection matrix may be one of the following:

* an infinite perspective projection matrix  
![M_{projection} = \begin{pmatrix} \frac{1}{a\tan{\frac{fov_y}{2}}} & 0 & 0 & 0 \\ 0 & \frac{1}{\tan{\frac{fov_y}{2}}} & 0 & 0 \\ 0 & 0 & -1 & -2 d_{far} \\ 0 & 0 & -1 & 0 \end{pmatrix}](m_proj_1.gif)

* a finite perspective projection matrix  
![M_{projection} = \begin{pmatrix} \frac{1}{a\tan{\frac{fov_y}{2}}} & 0 & 0 & 0 \\ 0 & \frac{1}{\tan{\frac{fov_y}{2}}} & 0 & 0 \\ 0 & 0 & \frac{d_{far}}{d_{near}-d_{far}} & \frac{-d_{far} d_{near}}{d_{far}-d_{near}} \\ 0 & 0 & -1 & 0 \end{pmatrix}](m_proj_2.gif)

* an infinite orthographic projection matrix  
![M_{projection} = \begin{pmatrix} \frac{2}{d_{right}-d_{left}} & 0 & 0 & \frac{-(d_{right}+d_{left})}{d_{right}-d_{left}} \\ 0 & \frac{2}{d_{top}-d_{bottom}} & 0 & \frac{-(d_{top}+d_{bottom})}{d_{top}-d_{bottom}} \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}](m_proj_3.gif)

* a finite orthographic projection matrix  
![M_{projection} = \begin{pmatrix} \frac{2}{d_{right}-d_{left}} & 0 & 0 & \frac{-(d_{right}+d_{left})}{d_{right}-d_{left}} \\ 0 & \frac{2}{d_{top}-d_{bottom}} & 0 & \frac{-(d_{top}+d_{bottom})}{d_{top}-d_{bottom}} \\ 0 & 0 & \frac{-1}{d_{far}-d_{near}} & \frac{-d_{near}}{d_{far}-d_{near}} \\ 0 & 0 & 0 & 1 \end{pmatrix}](m_proj_4.gif)
