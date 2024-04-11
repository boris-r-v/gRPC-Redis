#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>

#include <chrono>


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


void runner(int st, int end ) {
	BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));
	for (int i=st; i<end; ++i ){
		md.CreateBalanceSync(i, std::to_string(i), i);
	}
}

int main(){


   	
   	//BalanceRpcClient md( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));
	//BalanceRpcClient md2( grpc::CreateChannel("127.0.0.1:5678", grpc::InsecureChannelCredentials()));

	int max=50*10000;
   	auto begin = std::chrono::steady_clock::now();
	auto tr1 = std::thread(runner, 0, 10*10000);
	auto tr2 = std::thread(runner, 10*10000, 20*10000);
	auto tr3 = std::thread(runner, 20*10000, 30*10000);
	auto tr4 = std::thread(runner, 30*10000, 40*10000);
	auto tr5 = std::thread(runner, 40*10000, 50*10000);

	tr1.join();
	tr2.join();
	tr3.join();
	tr4.join();
	tr5.join();


	auto end = std::chrono::steady_clock::now();
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	std::cout << " sync_create takes " << elapsed_ms.count()/10 << "ms to create 50k records\n";
    return 0;
}
