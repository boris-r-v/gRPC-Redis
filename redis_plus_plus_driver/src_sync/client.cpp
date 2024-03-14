
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>



class BalanceRpcClient{
	std::unique_ptr<bsan::BalanceRPC::Stub> stub_;
    public:
	BalanceRpcClient( std::shared_ptr<grpc::Channel> chanel ): stub_ (bsan::BalanceRPC::NewStub(chanel)) {}

	std::string GetMess( std::string const& user ){
	    bsan::CreateBalanceRequest request;
	    request.set_name (user);

	    bsan::CreateBalanceResponce responce;
	    grpc::ClientContext context;
	    grpc::Status status = stub_ -> CreateBalance( &context, request, &responce );

	    if ( status.ok() ) return responce.message();
	    else std::cout << "RPC failed" << std::endl;
	    return "RPC failed";
	}
};


int main(int argc, char** argv){

    std::cout << "RUN " << argc << " == 2" << std::endl;

    if (argc != 2 ) return 1;


    BalanceRpcClient mbsan( grpc::CreateChannel("127.0.0.1:4567", grpc::InsecureChannelCredentials()));
    std::string user("world");
    std::string reply = mbsan.GetMess( argv[1] );
    std::cout << "Greeter received: " << reply << std::endl;


    return 0;
}
