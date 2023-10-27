/*
 * point_engine.h
 *
 *  Created on: Oct 27, 2023
 *      Author: cgraf
 */

#ifndef POINT_ENGINE_H_
#define POINT_ENGINE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define num_points 256
#define TRUE 1
#define FALSE 0

typedef struct point{
    float latitude;
    float longitude;
} point;

typedef struct path{
    point cur_point;
    point past_point[num_points];
    uint16_t idx;
    uint32_t time_stamp;
} path;

int addPoint(uint8_t* packet, point* point, path* path);

#endif /* POINT_ENGINE_H_ */
