#include "myjsonrpcserver.h"

template <class T>
std::string f2Str(T f, int pres)
{
    char buf[256];
    memset(buf, 0, sizeof (buf));
    std::string format = "%0."+std::to_string(pres)+"f";
    fprintf(buf, format.data(), f);
    std::string fStr(buf, sizeof (buf));
    return fStr;
}

MyJsonrpcServer::MyJsonrpcServer(jsonrpc::HttpServer &server):
    jsonrpc::AbstractServer<MyJsonrpcServer>(server)
{
    this->bindAndAddMethod(jsonrpc::Procedure("getVehicleStatus", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, NULL),
                           &MyJsonrpcServer::getVehicleStatus);

    this->bindAndAddMethod(jsonrpc::Procedure("setMoveTarget", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, NULL),
                           &MyJsonrpcServer::setMoveTarget);
    this->bindAndAddMethod(jsonrpc::Procedure("getVehiclePosition", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, NULL),
                           &MyJsonrpcServer::getCurrentPosition);


    this->bindAndAddNotification(jsonrpc::Procedure("notifyServer", jsonrpc::PARAMS_BY_NAME, NULL), &MyJsonrpcServer::notifyServer);
    send_msg_id = 0;
}

void MyJsonrpcServer::setMoveTarget(const Json::Value &request, Json::Value &response)
{
    Json::Value tarJson = request["move_target"];
    Json::FastWriter wt;
    std::string str = wt.write(tarJson);
    printf("move cmd: %s\n", str.data());
    double tarX = atof(tarJson["position_x"].asString().data())/1000.0;
    double tarY = atof(tarJson["position_y"].asString().data())/1000.0;
    double theta = atan2(tarY-vehicleStatus.posY, tarX-vehicleStatus.posX);
    if(theta>M_PI) theta-=2*M_PI;
    if(theta<-M_PI) theta+=2*M_PI;
    if(tarJson["orientation"].asString().size()>6)
    {
        theta = atof(tarJson["orientation"].asString().data());
    }

    printf("set move target: x: %0.8f, y: %0.8f, theta: %0.8f\n", tarX, tarY, theta);

    pTruck->SetTarget(tarX, tarY, theta);
    response = "recvOK";
}

void MyJsonrpcServer::getCurrentPosition(const Json::Value &request, Json::Value &response)
{
    uint msgID = request["msg_id"].asUInt();
    Json::Value root;
    root["vehicle_id"] =vehicleStatus.vehicle_id;
    root["msg_id"] = msgID;
    root["msg_type"]="vehicle_position";
    root["position"]=vehicleStatus.curPosition;

    Json::FastWriter jsonWriter;

    std::string jStr =  jsonWriter.write(root);

    printf("vehicle status: %s\n", jStr.data());

    response = jStr;
}

void MyJsonrpcServer::getVehicleStatus(const Json::Value &request, Json::Value &response)
{
    //printf("getVehicleStatus method be called\n");
    //返回车辆状态
    /*
     * {
     *  vehicle_id:0,
     *  msg_id:0,
     *  msg_type: vehicle_status,
     *  motion_status:
     *      {
     *          position_x:0,
     *          position_y:0,
     *          orientation:0,
     *          speed:0
     *      }
     *  isArrival: yes/no
     *  task_status:idle/excuting/
     *
     * }
    */
    uint msgID = request["msg_id"].asUInt();
    Json::Value root;
    root["vehicle_id"] =vehicleStatus.vehicle_id;
    root["msg_id"] = msgID;
    root["msg_type"]="vehicle_status";
    Json::Value motionJson;
    motionJson["position_x"]=vehicleStatus.posX;
    motionJson["position_y"]=vehicleStatus.posY;
    motionJson["orientation"]=vehicleStatus.theta;
    motionJson["speed"]=vehicleStatus.speed;
    root["motion_status"]=motionJson;
    root["isArrival"]=vehicleStatus.isArrival?"yes":"no";

    Json::FastWriter jsonWriter;

    std::string jStr =  jsonWriter.write(root);

    //printf("vehicle status: %s\n", jStr.data());

    response = jStr;
}

void MyJsonrpcServer::notifyServer(const Json::Value &request)
{
    (void)request;
    std::cout << "server received some Notification" << std::endl;

}

void MyJsonrpcServer::UpdateVehicleStatus(VEHICLE_STATUS &status)
{
    vehicleStatus = status;
}

void MyJsonrpcServer::SetTruck(Truck *parent)
{
    pTruck = parent;
}
