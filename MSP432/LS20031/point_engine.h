/*
 * point_engine.h
 *
 *  Created on: Oct 27, 2023
 *      Author: cgraf
 */

#ifndef POINT_ENGINE_H_
#define POINT_ENGINE_H_

#define num_points 256

typedef struct point{
    float latitude;
    float longitude;
} point;

typedef struct path{
    point cur_point;
    point[num_points] past_point;
    uint16_t idx;
    uint32_t time_stamp;
} path;

#endif /* POINT_ENGINE_H_ */
