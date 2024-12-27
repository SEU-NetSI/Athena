/*
 * calibration.h
 *
 *  Created on: May 15, 2024
 *      Author: SRTP--PENGYAO NIU /ZEHAO SHAO /MINGYAN LI /YUCHAO GE
 */

#ifndef TINYMAP_INC_CALIBRATION_H_
#define TINYMAP_INC_CALIBRATION_H_

#include"vl53l5cx_api.h"

// 定义常量
#define M_PI 3.14159
#define SIZE 8

// XYZ坐标结构体
typedef struct {
    double Xpos[SIZE][SIZE];
    double Ypos[SIZE][SIZE];
    double Zpos[SIZE][SIZE];
    double reflect[SIZE][SIZE];
} XYZ_ZoneCoordinates;

typedef struct {
    double distance_mm[SIZE][SIZE];//距离
    int target_status[SIZE][SIZE];//状态
    double target_reflectance[SIZE][SIZE];//反射率
} ResultsData;

//函数定义
void init_XYZ_ZoneCoordinates(XYZ_ZoneCoordinates* coords);// 初始化XYZ_ZoneCoordinates
void init_ResultsData(ResultsData* data);// 初始化ResultsData
int Judge(ResultsData* results, int i, int j) ;// 判断数据是否有效
void setResultData(ResultsData* results,
		int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t target_status[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)]);//设定ResultData
XYZ_ZoneCoordinates ConvertDist2XYZCoords8x8( ResultsData* results);//返回标定后的激光雷达坐标
void ground_coordinate(XYZ_ZoneCoordinates* Points, float position[3],
		float yaw, float pitch, float roll, XYZ_ZoneCoordinates* ground_coordinates) ;//无人机世界坐标标定
void process(int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		uint8_t target_status[(VL53L5CX_RESOLUTION_8X8*VL53L5CX_NB_TARGET_PER_ZONE)],
		float position[3], float yaw, float pitch, float roll);//process总函数


#endif /* TINYMAP_INC_CALIBRATION_H_ */
