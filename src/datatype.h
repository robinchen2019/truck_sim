#ifndef DATATYPE_H
#define DATATYPE_H
#include <string>
typedef unsigned char uchar;

struct MSG_HEADER
{
    char start[2];
    uchar resLen[2];
    uchar type;
};

struct VEHICLE_STATUS
{
    std::string vehicle_id;
    std::string curPosition;
    float posX;
    float posY;
    float theta;
    float speed;
    bool isArrival;
};
typedef VEHICLE_STATUS VehicleStatus;

struct MSG_VEHICLE_STATUS
{
    MSG_HEADER header;
    uchar posX[4];
    uchar posY[4];
    uchar posTheta[4];
    uchar speed[4];
    uchar arrival;
};
typedef  MSG_VEHICLE_STATUS MsgVehicleStatus;

struct MSG_MOVE_TARGET
{
    MSG_HEADER header;
    uchar tarX[4];
    uchar tarY[4];
    uchar theta[4];
};
typedef  MSG_MOVE_TARGET MsgMoveTarget;


#endif // DATATYPE_H
