#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <grpcpp/grpcpp.h>

#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>

#include <chrono>

std::ostream& operator<<(std::ostream& s, cls::BalanceData const& d){
    s<<"{\"id\":" << d.id() << ", \"name\":\"" << d.name() <<"\", \"value\":" << d.value() <<"}";
    return s;
}

class BalanceRpcClient{
	std::unique_ptr<cls::BalanceRPC::Stub> stub_;
    public:
	BalanceRpcClient( std::shared_ptr<grpc::Channel> chanel ): stub_ (cls::BalanceRPC::NewStub(chanel)) {}

	std::string GetBalance( int _id){
        cls::GetBalanceRequest req;
	    req.set_id ( _id );

	    cls::BalanceData resp;
	    grpc::ClientContext context;
	    grpc::Status status = stub_ -> GetBalance( &context, req, &resp );

	    if ( status.ok() ) {
            std::stringstream ss;
            ss << resp;
            return ss.str();
        }
	    else std::cout << "RPC failed" << std::endl;
	    return "RPC failed";
	}
};


int main(int argc, char** argv){

    if (argc != 2) {
        std::cerr << "Usage: read_one balance_id" << std::endl;
    }
   	
   	BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));\
    std::cout << md.GetBalance( std::stoi( argv[1] ) ) << std::endl; 
	
    return 0;
}
