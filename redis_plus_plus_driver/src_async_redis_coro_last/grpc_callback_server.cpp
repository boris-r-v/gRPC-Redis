
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
//#include <grpcpp/ext/proto_server_reflection_plugin.h>
//#include <grpcpp/health_check_service_interface.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>


class CreateBalanceImpl final: public cls::BalanceRPC::CallbackService{
    grpc::ServerUnaryReactor* CreateBalance(grpc::CallbackServerContext* context, const cls::BalanceData* request,  cls::CreateBalanceResponce* reply ) override {
        std::string prefix ("Create ");
        reply->set_message(prefix + std::to_string( request->id() ) + ":" + request->name() );
    
//        std::cout << "Handle request id: " << request->id() << std::endl;

	      grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
        reactor->Finish(grpc::Status::OK);
        return reactor;
    }
};

void RunServer( ) {
  std::string server_address ("0.0.0.0:5678");
  CreateBalanceImpl service;
  CreateBalanceImpl service1;

  grpc::EnableDefaultHealthCheckService(true);
//  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
//  builder.RegisterService(&service1);
  // Finally assemble the server.
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}


int main(int argc, char** argv){

    std::cout << "RUN " << std::endl;

    RunServer(); 
    return 0;
}
