/*
 * bresenham.c
 *
 *  Created on: May 16, 2024
 *      Author: mfxjx
 */

#include "bresenham.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void init_Point3DArray(Point3DArray* arr, uint8_t initialCapacity) {
    arr->points = (Point3D*)malloc(initialCapacity * sizeof(Point3D));
    arr->size = 0;
    arr->capacity = initialCapacity;
}

void add_Point3D(Point3DArray* arr, Point3D p) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->points = (Point3D*)realloc(arr->points, arr->capacity * sizeof(Point3D));
    }
    arr->points[arr->size++] = p;
}

void free_Point3DArray(Point3DArray* arr) {
    free(arr->points);
    arr->points = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

void bresenham_3d(float x0, float y0, float z0, float x1, float y1, float z1, float resolution, Point3DArray* points) {
    int ix0 = (int)round(x0 / resolution);
    int iy0 = (int)round(y0 / resolution);
    int iz0 = (int)round(z0 / resolution);
    int ix1 = (int)round(x1 / resolution);
    int iy1 = (int)round(y1 / resolution);
    int iz1 = (int)round(z1 / resolution);

    int dx = abs(ix1 - ix0);
    int dy = abs(iy1 - iy0);
    int dz = abs(iz1 - iz0);

    int sx = sign(ix1 - ix0);
    int sy = sign(iy1 - iy0);
    int sz = sign(iz1 - iz0);

    int err1, err2;

    if (dx >= dy && dx >= dz) {
        err1 = dy - dx / 2;
        err2 = dz - dx / 2;
        while (ix0 != ix1) {
            add_Point3D(points, (Point3D){ix0 * resolution, iy0 * resolution, iz0 * resolution});
            if (err1 > 0) {
                iy0 += sy;
                err1 -= dx;
            }
            if (err2 > 0) {
                iz0 += sz;
                err2 -= dx;
            }
            err1 += dy;
            err2 += dz;
            ix0 += sx;
        }
    } else if (dy >= dx && dy >= dz) {
        err1 = dx - dy / 2;
        err2 = dz - dy / 2;
        while (iy0 != iy1) {
            add_Point3D(points, (Point3D){ix0 * resolution, iy0 * resolution, iz0 * resolution});
            if (err1 > 0) {
                ix0 += sx;
                err1 -= dy;
            }
            if (err2 > 0) {
                iz0 += sz;
                err2 -= dy;
            }
            err1 += dx;
            err2 += dz;
            iy0 += sy;
        }
    } else {
        err1 = dx - dz / 2;
        err2 = dy - dz / 2;
        while (iz0 != iz1) {
            add_Point3D(points, (Point3D){ix0 * resolution, iy0 * resolution, iz0 * resolution});
            if (err1 > 0) {
                ix0 += sx;
                err1 -= dz;
            }
            if (err2 > 0) {
                iy0 += sy;
                err2 -= dz;
            }
            err1 += dx;
            err2 += dy;
            iz0 += sz;
        }
    }
    add_Point3D(points, (Point3D){ix1 * resolution, iy1 * resolution, iz1 * resolution});
}

void bresenham_3d_multiple(const Point3D* startPoints, const Point3D* endPoints, ARRAY_SIZE_TYPE size, float resolution, Point3DArray* points) {
    for (ARRAY_SIZE_TYPE i = 0; i < size; ++i) {
        bresenham_3d(startPoints[i].x, startPoints[i].y, startPoints[i].z, endPoints[i].x, endPoints[i].y, endPoints[i].z, resolution, points);
    }
}
