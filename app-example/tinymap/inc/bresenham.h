/*
 * bresenham.h
 *
 *  Created on: May 16, 2024
 *      Author: mfxjx
 */

#ifndef TINYMAP_INC_BRESENHAM_H_
#define TINYMAP_INC_BRESENHAM_H_
#include <stdint.h>  // 为了使用标准整数类型

#define ARRAY_SIZE_TYPE uint8_t  // 使用宏来定义数组大小类型

typedef struct {
    float x;
    float y;
    float z;
} Point3D;

typedef struct {
    Point3D* points;
    uint8_t size;
    uint8_t capacity;
} Point3DArray;

void init_Point3DArray(Point3DArray* arr, uint8_t initialCapacity);
void add_Point3D(Point3DArray* arr, Point3D p);
void free_Point3DArray(Point3DArray* arr);
int sign(int x);
void bresenham_3d(float x0, float y0, float z0, float x1, float y1, float z1, float resolution, Point3DArray* points);
void bresenham_3d_multiple(const Point3D* startPoints, const Point3D* endPoints, ARRAY_SIZE_TYPE size, float resolution, Point3DArray* points);

#endif /* TINYMAP_INC_BRESENHAM_H_ */
