#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include <CLS.pb.h>
#include <CLS.grpc.pb.h>

class BalanceRpcClient{
	std::unique_ptr<cls_gen::CounterRPC::Stub> stub_;
    public:
	BalanceRpcClient( std::shared_ptr<grpc::Channel> chanel ): stub_ (cls_gen::CounterRPC::NewStub(chanel)) {}

	int64_t create( int _id ){
	    cls_gen::BalanceInfo request;
	    request.set_id ( _id );
		request.set_version ( 24*10*_id );
		request.set_profileid ( 600 + 10*_id);
		
	    cls_gen::BalanceId responce;
	    grpc::ClientContext context;
	    grpc::Status status = stub_ -> SetBalanceInfo( &context, request, &responce );

	    if ( status.ok() ) return responce.id();
	    else std::cout << "RPC failed" << std::endl;
	    return 0;
	}
};


void runner(int st, int end ) {
	BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));
	for (int i=st; i<end; ++i ){
		md.create( i );
	}
}

int main(int argc, char** argv){

	if (argc != 2){
		std::cerr << "Usage: sync_create counter_id" << std::endl;
		return 1;
	}

	BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));
	std::cout << "Create balanceInfo id:<" << md.create( std::stoi(argv[1]) ) << ">\n";

    return 0;
}
