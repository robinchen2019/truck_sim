#include "truck.h"
#include <tf/tf.h>
Truck::Truck(ros::NodeHandle &n):
    nh(n)
{
    double initX = 0;
    double initY = 0;
    double initTheta = 0;
    nh.param<double>("initX", initX, 0.0);
    nh.param<double>("initY", initY, 0.0);
    nh.param<double>("initZ", initTheta, 0.0);
    nh.param<double>("maxVelocity", maxVelocity, 2.0);
    nh.param<double>("velocityAcc", velocityAcc, 0.1);
    nh.param<int>("port", port, 1200);
    std::string position;
    nh.getParam("position", position);

    printf("position: %s\n", position.data());

    vehicleStatus.curPosition = position;
    geometry_msgs::Pose2D pos2d;
    pos2d.x = initX;
    pos2d.y = initY;
    pos2d.theta = initTheta;
    SetCurrentPos(pos2d);

    speedX = 0;
    speedZ = 0;
    targetX = initX;
    targetY = initY;
    targetTheta = initTheta;
    moveSpeedX = 0;
    isArrival = false;

    std::string vehicle_id = nh.getNamespace();
    printf("this vehicle id: %s\n", vehicle_id.data());
    vehicleStatus.vehicle_id = vehicle_id;
    vehicleStatus.posX = initX;
    vehicleStatus.posY = initY;
    vehicleStatus.theta = initTheta;

    vehicleStatus.speed = 0;
    vehicleStatus.isArrival = true;
    vehicleStatus.vehicle_id = nh.getUnresolvedNamespace();


    simTimer = nh.createTimer(ros::Duration(0.01), &Truck::CallbackSimTimer, this);
    posePub = nh.advertise<visualization_msgs::MarkerArray>("pose", 10);

    try {
        pHttpServer = new jsonrpc::HttpServer(port);
        pJsonrpcServer = new MyJsonrpcServer(*pHttpServer);
        if(pJsonrpcServer->StartListening())
        {
            printf("json rpc server start success!\n");
        }
        else
        {
            printf("json rpc server start failed!\n");
        }

        pJsonrpcServer->SetTruck(this);


    } catch (jsonrpc::JsonRpcException &e) {
        printf("%s\n", e.what());
    }

}

void Truck::SetCurrentPos(geometry_msgs::Pose2D pos2d)
{
    curPos = pos2d;
    isArrival = false;

}

void Truck::SetTarget(double x, double y, double theta)
{
    targetX = x;
    targetY = y;
    targetTheta = theta;
    isArrival = false;
}

void Truck::SpeedController()
{

    double linearThres = 0.002;
    double angularThres = 0.01;
    double disToTar = fabs(targetX-vehicleStatus.posX)+fabs(targetY-vehicleStatus.posY);
    if(disToTar<linearThres)
    {
        speedX = 0;
        double angleDif = targetTheta - vehicleStatus.theta;
        if(angleDif>M_PI) angleDif-=2*M_PI;
        if(angleDif<-M_PI) angleDif+=2*M_PI;
        if(fabs(angleDif)<angularThres)
        {
            vehicleStatus.theta = targetTheta;
            speedZ = 0;
            isArrival=true;
        }
        else
        {
            if(angleDif<0)
            {
                speedZ = std::max(0.1*angleDif, -0.4);
                speedZ = std::min(-0.1, speedZ);
            }
            else
            {
                speedZ = std::min(0.1*angleDif, 0.4);
                speedZ = std::max(0.1, speedZ);
            }

        }
    }
    else
    {
        double oriAngle = atan2(targetY-vehicleStatus.posY, targetX-vehicleStatus.posX);
        double angleDif =oriAngle - vehicleStatus.theta;
        if(angleDif>M_PI) angleDif-=2*M_PI;
        if(angleDif<-M_PI) angleDif+=2*M_PI;
        if(fabs(angleDif)<angularThres)
        {
            vehicleStatus.theta = oriAngle;
            speedZ = 0;
            speedX = 10*disToTar;
            speedX = std::min(speedX, maxVelocity);
        }
        else
        {
            speedX = 0;
            if(angleDif<0)
            {
                speedZ = std::max(angleDif, -0.4);
                speedZ = std::min(-0.1, speedZ);
            }
            else
            {
                speedZ = std::min(angleDif, 0.4);
                speedZ = std::max(0.1, speedZ);
            }
        }

    }

}

void Truck::CallbackSimTimer(const ros::TimerEvent &e)
{

    static double lastTime = ros::Time::now().toSec();
    double curTime = ros::Time::now().toSec();
    double deltaT = curTime-lastTime;
    lastTime = curTime;

    SpeedController();

    moveSpeedX = speedX;

    //printf("speed Z: %0.8f\n", speedZ);
    vehicleStatus.theta += speedZ*deltaT;
    if(vehicleStatus.theta>M_PI)
    {
        vehicleStatus.theta -= 2*M_PI;
    }
    else if(vehicleStatus.theta<-M_PI)
    {
        vehicleStatus.theta += 2*M_PI;
    }
    vehicleStatus.posX += moveSpeedX*deltaT*cos(vehicleStatus.theta);
    vehicleStatus.posY += moveSpeedX*deltaT*sin(vehicleStatus.theta);

    geometry_msgs::Pose2D pose;
    pose.x = vehicleStatus.posX;
    pose.y = vehicleStatus.posY;
    pose.theta = vehicleStatus.theta;
    visualization_msgs::MarkerArray markArray;

    visualization_msgs::Marker poseMark, textMark;
    poseMark.id = 0;
    poseMark.ns = nh.getNamespace();
    poseMark.type = visualization_msgs::Marker::ADD;
    poseMark.color.a = 1.0;
    poseMark.color.b = 0.0;
    poseMark.color.g = 0.0;
    poseMark.color.r = 1.0;

    poseMark.scale.x = 4.0;
    poseMark.scale.y = 1.0;
    poseMark.scale.z = 1.0;

    poseMark.header.frame_id = "map";
    poseMark.header.stamp = ros::Time::now();
    poseMark.pose.position.x = vehicleStatus.posX;
    poseMark.pose.position.y = vehicleStatus.posY;
    poseMark.pose.position.z = 0;

    tf::Quaternion q = tf::createQuaternionFromYaw(vehicleStatus.theta);
    poseMark.pose.orientation.x = q.x();
    poseMark.pose.orientation.y = q.y();
    poseMark.pose.orientation.z = q.z();
    poseMark.pose.orientation.w = q.w();


    textMark.id = 1;
    textMark.ns = nh.getNamespace();
    textMark.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    textMark.color.a = 1.0;
    textMark.color.b = 0.0;
    textMark.color.g = 0.0;
    textMark.color.r = 1.0;

    textMark.scale.z = 1.0;
    textMark.header.frame_id = "map";
    textMark.header.stamp = ros::Time::now();
    textMark.pose.position.x = vehicleStatus.posX;
    textMark.pose.position.y = vehicleStatus.posY;
    textMark.pose.position.z = 0;

    textMark.pose.orientation.x = q.x();
    textMark.pose.orientation.y = q.y();
    textMark.pose.orientation.z = q.z();
    textMark.pose.orientation.w = q.w();
    textMark.text = vehicleStatus.vehicle_id;




    vehicleStatus.speed = moveSpeedX;
    vehicleStatus.isArrival = isArrival;
    pJsonrpcServer->UpdateVehicleStatus(vehicleStatus);

    markArray.markers.push_back(poseMark);
    markArray.markers.push_back(textMark);

    posePub.publish(markArray);


}
