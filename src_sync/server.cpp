
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include <grpcpp/grpcpp.h>
//#include <grpcpp/health_check_service_interface.h>
//#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>

class BalanceRpcImp final : public bsan::BalanceRPC::Service{

    grpc::Status CreateBalance(grpc::ServerContext* _context, bsan::CreateBalanceRequest const* _req, bsan::CreateBalanceResponce* _resp){
	std::string str("Hi ");
	std::cout << "Get request: " << _req->name() << std::endl;
	_resp->set_message(str+_req->name() );
	return grpc::Status::OK;
    }

};


void RunServer(){
    BalanceRpcImp service;
    std::string server_addres("0.0.0.0:4567");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_addres, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_addres << std::endl;
    server->Wait();

}

int main(){

    std::cout << "RUN" << std::endl;
    RunServer( );
    return 0;
}
