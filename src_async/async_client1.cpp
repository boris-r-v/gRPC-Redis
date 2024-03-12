
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>

class  CreateBalanceClient {
    public:
        CreateBalanceClient(std::shared_ptr<grpc::Channel> chanel): stub_(bsan::BalanceRPC::NewStub(chanel) ) { }
    
        std::string CreateBalance(const std::string& user) {

            bsan::CreateBalanceRequest request;
            request.set_name(user);

            bsan::CreateBalanceResponce reply;

            grpc::ClientContext context;

            grpc::CompletionQueue cq; 

            grpc::Status status;

            std::unique_ptr<grpc::ClientAsyncResponseReader<bsan::CreateBalanceResponce> > rpc( stub_->AsyncCreateBalance(&context, request, &cq) );

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
        std::unique_ptr<bsan::BalanceRPC::Stub> stub_;
};



int main(int argc, char** argv){

    std::cout << "RUN " << argc << " == 2" << std::endl;

    if (argc != 2 ) return 1;

    std::string target_str {"127.0.0.1:5678"};
    CreateBalanceClient asclient( grpc::CreateChannel( target_str, grpc::InsecureChannelCredentials() ) );
    std::string user( argv[1] );
    std::string reply = asclient.CreateBalance(user);  // The actual RPC call!
    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}