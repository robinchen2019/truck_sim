
#include <ros/ros.h>
#include "truck.h"
int main(int argc, char** argv)
{
    ros::init(argc, argv, "truck_sim");
    ros::NodeHandle nh("~");
    Truck truck(nh);

    ros::spin();
}
