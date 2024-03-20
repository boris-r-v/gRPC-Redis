#include <iostream>
#include <app.h>
#include <grpc_server.h>

void cls_core::Main::run(int argc, char** argv){
    gRPC::ServerImpl server_;
    server_.run();
}