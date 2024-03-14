
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>

class  CreateBalanceClient {
    public:
        CreateBalanceClient(std::shared_ptr<grpc::Channel> chanel): stub_(cls::BalanceRPC::NewStub(chanel) ) { }
    
        std::string CreateBalance(const std::string& name, int id, int value ) {

            cls::BalanceData request;
            request.set_name(name);
            request.set_id(id);
            request.set_value(value);

            cls::CreateBalanceResponce reply;
            grpc::ClientContext context;
            grpc::CompletionQueue cq; 
            grpc::Status status;
            std::unique_ptr<grpc::ClientAsyncResponseReader<cls::CreateBalanceResponce> > rpc( stub_->AsyncCreateBalance(&context, request, &cq) );

            // Request that, upon completion of the RPC, "reply" be updated with the
            // server's response; "status" with the indication of whether the operation
            // was successful. Tag the request with the integer 1.
            rpc->Finish(&reply, &status, (void*)1);
            void* got_tag;
            bool ok = false;
            // Block until the next result is available in the completion queue "cq".
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or the cq_ is shutting down.
            GPR_ASSERT(cq.Next(&got_tag, &ok));

            // Verify that the result from "cq" corresponds, by its tag, our previous request.
            GPR_ASSERT(got_tag == (void*)1);
            // ... and that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().
            GPR_ASSERT(ok);

            // Act upon the status of the actual RPC.
            if (status.ok()) return reply.message();
            else return "RPC failed";   
        }

    private:
        // Out of the passed in Channel comes the stub, stored here, our view of the server's exposed services.
        std::unique_ptr<cls::BalanceRPC::Stub> stub_;
};



int main(int argc, char** argv){
    
    std::string target_str {"127.0.0.1:5678"};
    CreateBalanceClient client( grpc::CreateChannel( target_str, grpc::InsecureChannelCredentials() ) );
    std::string user( "User_" );
	for (int i=1; i<100; ++i ){
		std::cout << "call <" << i << "> resp <" << client.CreateBalance(user+std::to_string(i), i, 10*i+i%9 ) << ">" << std::endl;
	}

    return 0;
}