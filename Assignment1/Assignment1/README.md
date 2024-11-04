# 作业1 - MVP变换

## 要求

填写一个旋转矩阵和一个透视投影矩阵。给定三维下三个点：

$v_0 (2.0,0.0,−2.0), v_1 (0.0,2.0,−2.0), v_2 (−2.0,0.0,−2.0)$, 

你需要将这三个点的坐标变换为屏幕坐标并在屏幕上绘制出对应的线框三角形 (在代码框架中已经提供了 draw_triangle 函数，所以只需要去构建变换矩阵即可)。简而言之，需要进行模型、视图、投影、视口等变换来将三角形显示在屏幕上。在提供的代码框架中，需要完成模型变换和投影变换的部分。

## 思路

- 实现如下`main.cpp`中如下接口：
```c++
// Create the model matrix for rotating the triangle around the Z axis.
Matrix4f get_model_matrix(float rotation_angle);

// Create the projection matrix for the given parameters.
Matrix4f get_projection_matrix(float eye_fov, 
				float aspect_ratio,
 				float zNear,
				float zFar);

```

即 MVP 变换中的 Model Transform 和 Project Transform。只需要分别实现对应的矩阵即可。