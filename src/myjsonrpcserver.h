#ifndef MYJSONRPCSERVER_H
#define MYJSONRPCSERVER_H
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <iostream>
#include "datatype.h"
#include "truck.h"
#include <jsoncpp/json/json.h>



class Truck;
class MyJsonrpcServer: public jsonrpc::AbstractServer<MyJsonrpcServer>
{
public:
    MyJsonrpcServer(jsonrpc::HttpServer &server);
    // method
    void getCurrentPosition(const Json::Value &request, Json::Value &response);
    void getVehicleStatus(const Json::Value &request, Json::Value &response);
    void setMoveTarget(const Json::Value &request, Json::Value &response);

    // notification
    void notifyServer(const Json::Value &request);
    void UpdateVehicleStatus(VEHICLE_STATUS &status);
    void SetTruck(Truck *parent);

private:
    VEHICLE_STATUS vehicleStatus;
    Truck* pTruck;
    uint send_msg_id;
};

#endif // MYJSONRPCSERVER_H
