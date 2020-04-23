#ifndef TRUCK_H
#define TRUCK_H

#include <ros/ros.h>
#include <geometry_msgs/Pose2D.h>
#include <ros/timer.h>
#include "myjsonrpcserver.h"
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
class MyJsonrpcServer;
class Truck
{
public:
    Truck(ros::NodeHandle &n);
    void SetCurrentPos(geometry_msgs::Pose2D pos2d);
    void SetTargetPoints(geometry_msgs::Pose2D pos2d);
    void SetTarget(double x, double y, double theta);

private:
    MyJsonrpcServer* pJsonrpcServer;
    jsonrpc::HttpServer* pHttpServer;
    ros::NodeHandle nh;
    ros::Timer simTimer;
    ros::Publisher posePub;
    geometry_msgs::Pose2D curPos;
    int port;
    double maxVelocity;
    double velocityAcc;
    double targetX;
    double targetY;
    double targetTheta;
    double speedX;
    double speedZ;
    double moveSpeedX;
    bool isArrival;
    VEHICLE_STATUS vehicleStatus;

    void CallbackSimTimer(const ros::TimerEvent &e);
    void SpeedController();

};

#endif // TRUCK_H
