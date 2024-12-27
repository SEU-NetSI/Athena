/*
 * calibration.c
 *
 *  Created on: May 15, 2024
 *  Author: SRTP--PENGYAO NIU /ZEHAO SHAO /MINGYAN LI /YUCHAO GE
 *
 */

#include "calibration.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static XYZ_ZoneCoordinates XYZ_ZoneCoordinate;
//定义全局变量
float SinOfPitch[SIZE][SIZE]={
	{0.857167, 	0.898794, 	0.923880,	0.939693,	0.939693,	0.923880,	0.898794,	0.857167},
	{0.898794, 	0.939693,	0.955793,	0.965473, 	0.965473,	0.955793,	0.939693,	0.898794},
	{0.923880,	0.955793,	0.975917,	0.986286,	0.986286,	0.975917,	0.955793,	0.923880},
	{0.939693, 	0.965473, 	0.986286, 	0.997250, 	0.997250,	0.986286, 	0.965473, 	0.939693},
	{0.939693, 	0.965473, 	0.986286, 	0.997250,	0.997250, 	0.986286, 	0.965473, 	0.939693},
	{0.923880, 	0.955793, 	0.975917, 	0.986286, 	0.986286, 	0.975917, 	0.955793, 	0.923880},
	{0.898794, 	0.939693, 	0.955793,	0.965473, 	0.965473, 	0.955793, 	0.939693, 	0.898794},
	{0.857167, 	0.898794, 	0.923880, 	0.939693, 	0.939693,	0.923880, 	0.898794, 	0.857167}
};

float CosOfPitch[SIZE][SIZE]={
    {0.515038, 0.438371, 0.382683, 0.342020, 0.342020, 0.382683, 0.438371, 0.515038},
    {0.438371, 0.342020, 0.294040, 0.260505, 0.260505, 0.294040, 0.342020, 0.438371},
    {0.382683, 0.294040, 0.218143, 0.165048, 0.165048, 0.218143, 0.294040, 0.382683},
    {0.342020, 0.260505, 0.165048, 0.074108, 0.074108, 0.165048, 0.260505, 0.342020},
    {0.342020, 0.260505, 0.165048, 0.074108, 0.074108, 0.165048, 0.260505, 0.342020},
    {0.382683, 0.294040, 0.218143, 0.165048, 0.165048, 0.218143, 0.294040, 0.382683},
    {0.438371, 0.342020, 0.294040, 0.260505, 0.260505, 0.294040, 0.342020, 0.438371},
    {0.515038, 0.438371, 0.382683, 0.342020, 0.342020, 0.382683, 0.438371, 0.515038}
};

float SinOfYaw[SIZE][SIZE]={
    {0.707107, 0.815128, 0.919135, 0.989950, 0.989950, 0.919135, 0.815128, 0.707107},
    {0.579281, 0.707107, 0.857527, 0.980580, 0.980580, 0.857527, 0.707107, 0.579281},
    {0.393942, 0.514440, 0.707107, 0.948600, 0.948600, 0.707107, 0.514440, 0.393942},
    {0.141420, 0.196117, 0.316477, 0.707107, 0.707107, 0.316477, 0.196117, 0.141420},
    {-0.141420, -0.196117, -0.316477, -0.707107, -0.707107, -0.316477, -0.196117, -0.141420},
    {-0.393942, -0.514440, -0.707107, -0.948600, -0.948600, -0.707107, -0.514440, -0.393942},
    {-0.393942, -0.707107, -0.857527, -0.980580, -0.980580, -0.857527, -0.707107, -0.579281},
    {-0.707107, -0.815128, -0.919135, -0.989950, -0.989950, -0.919135, -0.815128, -0.707107}
};

float CosOfYaw[SIZE][SIZE]={
    {-0.707107, -0.579281, -0.393942, -0.141420, 0.141420, 0.393942, 0.579281, 0.707107},
    {-0.815128, -0.707107, -0.514440, -0.196117, 0.196117, 0.514440, 0.707107, 0.815128},
    {-0.919135, -0.857527, -0.707107, -0.316477, 0.316477, 0.707107, 0.857527, 0.919135},
    {-0.989950, -0.980580, -0.948600, -0.707107, 0.707107, 0.948600, 0.980580, 0.989950},
    {-0.989950, -0.980580, -0.948600, -0.707107, 0.707107, 0.948600, 0.980580, 0.989950},
    {-0.919135, -0.857527, -0.707107, -0.316477, 0.316477, 0.707107, 0.857527, 0.919135},
    {-0.919135, -0.707107, -0.514440, -0.196117, 0.196117, 0.514440, 0.707107, 0.815128},
    {-0.707107, -0.579281, -0.393942, -0.141420, 0.141420, 0.393942, 0.579281, 0.707107}
};


// 初始化XYZ_ZoneCoordinates
void init_XYZ_ZoneCoordinates(XYZ_ZoneCoordinates* coords) {
    memset(coords->Xpos, 0, sizeof(coords->Xpos));
    memset(coords->Ypos, 0, sizeof(coords->Ypos));
    memset(coords->Zpos, 0, sizeof(coords->Zpos));
    memset(coords->reflect, 0, sizeof(coords->reflect));
}

// 初始化ResultsData
void init_ResultsData(ResultsData* data) {
    memset(data->distance_mm, 0, sizeof(data->distance_mm));
    memset(data->target_status, 0, sizeof(data->target_status));
    memset(data->target_reflectance, 0, sizeof(data->target_reflectance));
}

// 判断数据是否有效
int Judge(ResultsData* results, int i, int j) {
    if (results->distance_mm[i][j] > 0 &&
        (results->target_status[i][j] == 5 ||
         results->target_status[i][j] == 6 ||
         results->target_status[i][j] == 9)) {
        return 1;
    }
    return 0;
}

void setResultData(ResultsData* results,
		int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t target_status[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)])
{
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            results->distance_mm[i][j]=distance_mm[i*8+j+1];
            results->target_reflectance[i][j]=reflectance[i*8+j+1];
            results->target_status[i][j]=target_status[i*8+j+1];
        }
}

//返回标定后的激光雷达坐标
XYZ_ZoneCoordinates ConvertDist2XYZCoords8x8( ResultsData* results) {
    //XYZ_ZoneCoordinates XYZ_ZoneCoordinate;
    init_XYZ_ZoneCoordinates(&XYZ_ZoneCoordinate);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (Judge(results, i, j)) {
                double Hyp = results->distance_mm[i][j] / SinOfPitch[i][j];
                XYZ_ZoneCoordinate.Xpos[i][j] = CosOfYaw[i][j] * CosOfPitch[i][j] * Hyp;
                XYZ_ZoneCoordinate.Ypos[i][j] = SinOfYaw[i][j] * CosOfPitch[i][j] * Hyp;
                XYZ_ZoneCoordinate.Zpos[i][j] = results->distance_mm[i][j];
                XYZ_ZoneCoordinate.reflect[i][j]=results->target_reflectance[i][j];
            } else {
                XYZ_ZoneCoordinate.Xpos[i][j] = 0;
                XYZ_ZoneCoordinate.Ypos[i][j] = 0;
                XYZ_ZoneCoordinate.Zpos[i][j] = 0;
                XYZ_ZoneCoordinate.reflect[i][j]=0;
            }
        }
    }

    return XYZ_ZoneCoordinate;
}

//无人机世界坐标标定
void ground_coordinate(XYZ_ZoneCoordinates* Points, float position[3], float yaw, float pitch, float roll, XYZ_ZoneCoordinates* ground_coordinates) {

    //将角度改为弧度
    yaw = yaw * M_PI / 180.0;
    pitch = -pitch * M_PI / 180.0;
    roll = roll * M_PI / 180.0;

    float rotation_matrix[3][3] = {
        {cosf(yaw) * cosf(pitch), cosf(yaw) * sinf(pitch) * sinf(roll) - sinf(yaw) * cosf(roll), cosf(yaw) * sinf(pitch) * cosf(roll) + sinf(yaw) * sinf(roll)},
        {sinf(yaw) * cosf(pitch), sinf(yaw) * sinf(pitch) * sinf(roll) + cosf(yaw) * cosf(roll), sinf(yaw) * sinf(pitch) * cosf(roll) - cosf(yaw) * sinf(roll)},
        {-sinf(pitch), cosf(pitch) * sinf(roll), cosf(pitch) * cosf(roll)}
    };
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float relative_x = Points->Xpos[i][j];
            float relative_y = Points->Ypos[i][j];
            float relative_z = Points->Zpos[i][j];

            ground_coordinates->Xpos[i][j] = position[0] + rotation_matrix[0][0] * relative_x + rotation_matrix[0][1] * relative_y + rotation_matrix[0][2] * relative_z;
            ground_coordinates->Ypos[i][j] = position[1] + rotation_matrix[1][0] * relative_x + rotation_matrix[1][1] * relative_y + rotation_matrix[1][2] * relative_z;
            ground_coordinates->Zpos[i][j] = position[2] + rotation_matrix[2][0] * relative_x + rotation_matrix[2][1] * relative_y + rotation_matrix[2][2] * relative_z;
            ground_coordinates->reflect[i][j]=Points->reflect[i][j];
        }
    }
}

//process总函数
void process(int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t target_status[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
							  float position[3], float yaw, float pitch, float roll)
{
	ResultsData results;

	setResultData(&results, distance_mm,reflectance,target_status);
	XYZ_ZoneCoordinates XYZ_ZoneCoordinate=ConvertDist2XYZCoords8x8(&results);
	XYZ_ZoneCoordinates* ground_coordinates = (XYZ_ZoneCoordinates*)malloc(sizeof(XYZ_ZoneCoordinates));
	if (ground_coordinates == NULL) {
	        // 处理内存分配失败
	        return;
	    }

	ground_coordinate(&XYZ_ZoneCoordinate, position, yaw, pitch, roll, ground_coordinates);

	free(ground_coordinates);


}

