#include "point_engine.h"
#define UPDATE_PATH_TIME 100

uint32_t timeStamp = 0;

int charsToFloat(uint8_t* ptr, uint8_t lat, uint8_t orientation, float* toReturn) {
    float minutes = 0;
    if (lat) {
        if (ptr[4] != '.') {
            return FALSE;
        }
        *toReturn = 60*(10*((int)ptr[0] - 48) + ((int)ptr[1] - 48)); // get degrees to minutes
        minutes = 10*((int)ptr[2] - 48) + ((int)ptr[3] - 48) + 0.1*((int)ptr[5] - 48) + 0.01*((int)ptr[6] - 48)
                    + 0.001*((int)ptr[7] - 48) + 0.0001*((int)ptr[8] - 48);
        *toReturn = *toReturn + minutes;
        if (orientation == 0)
            *toReturn = -1 * (*toReturn);
    } else {
        if (ptr[5] != '.') {
            return FALSE;
        }
        *toReturn = 60*(100*((int)ptr[0] - 48) + 10*((int)ptr[1] - 48) + ((int)ptr[2] - 48)); // get degrees to minutes
        minutes = 10*((int)ptr[3] - 48) + ((int)ptr[4] - 48) + 0.1*((int)ptr[6] - 48) + 0.01*((int)ptr[7] - 48)
                    + 0.001*((int)ptr[8] - 48) + 0.0001*((int)ptr[9] - 48);
        *toReturn = *toReturn + minutes;
        if (orientation == 0)
            *toReturn = -1 * (*toReturn);
    }
    return TRUE;
}

int updatePath(path* path, point* point) {
    if (path->idx == num_points) return FALSE;
    path->past_point[path->idx] = path->cur_point;
    path->cur_point = *point;
    path->idx++;
    path->time_stamp = timeStamp;
    return TRUE;
}


int addPoint(uint8_t* packet, point* point, path* path) {
    int i = 0;
    uint8_t latNum[9];
    uint8_t longNum[10];
    uint8_t isNorth;
    uint8_t isEast;
    if ((packet == '\0') || (packet[i] == '\0')) return FALSE;
    if (packet[5] != ',') return FALSE;
    if (packet[12] != '.') return FALSE;
    if (packet[16] != ',') return FALSE;

    for (int i = 0; i < 9; i++) {
        latNum[i] = packet[17 + i];
    }
    if (packet[26] != ',') return FALSE;
    
    isNorth = (packet[27] == 'N') ? TRUE : FALSE;
    if (packet[28] != ',') return FALSE;

    for (int i = 0; i < 10; i++) {
        longNum[i] = packet[29 + i];
    }
    if (packet[39] != ',') return FALSE;

    isEast = (packet[40] == 'E') ? TRUE : FALSE;
    
    if (!charsToFloat(latNum, TRUE, isNorth, &(point->latitude)))
        return FALSE;
    if (!charsToFloat(longNum, FALSE, isEast, &(point->longitude)))
        return FALSE;
    
    timeStamp++;
    if (timeStamp % UPDATE_PATH_TIME == 0) {
        if (!updatePath(path, point))
            return FALSE;
    }
    return TRUE;
}

