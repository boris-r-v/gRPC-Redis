#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>



class BalanceRpcClient{
	std::unique_ptr<cls::BalanceRPC::Stub> stub_;
    public:
	BalanceRpcClient( std::shared_ptr<grpc::Channel> chanel ): stub_ (cls::BalanceRPC::NewStub(chanel)) {}

	std::string CreateBalanceSync( int _id, std::string const& _name, int _value ){
	    cls::BalanceData request;
	    request.set_id ( _id );
		request.set_name ( _name );
		request.set_value ( _value );

	    cls::CreateBalanceResponce responce;
	    grpc::ClientContext context;
	    grpc::Status status = stub_ -> CreateBalance( &context, request, &responce );

	    if ( status.ok() ) return responce.message();
	    else std::cout << "RPC failed" << std::endl;
	    return "RPC failed";
	}
};


int main(){

    BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));
	for (int i=1; i<100; ++i ){
		std::cout << "call <" << i << "> resp <" << md.CreateBalanceSync(i, std::to_string(i), 100*i+10*i%9) << std::endl; 
	}

    return 0;
}
