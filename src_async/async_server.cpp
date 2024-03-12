
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>



class ServerImpl{

    public:
        ~ServerImpl(){
            server_ -> Shutdown();
            cq_ -> Shutdown();
        }

	void  Run(){
	    std::string server_address ("0.0.0.0:5678" );
	    grpc::ServerBuilder builder;
	    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

	    builder.RegisterService(&service_);  // Register "service_" as the instance through which we'll communicate with clients. In this case it corresponds to an *asynchronous* service.
	    cq_ = builder.AddCompletionQueue(); // Get hold of the completion queue used for the asynchronous communication with the gRPC runtime.

	    server_ = builder.BuildAndStart();     // Finally assemble the server.
	    std::cout << "Server listening on " << server_address << std::endl;
	    HandleRpcs(); // Proceed to the server's main loop.
	}


    private:
        class CallerBase {
            public:
                virtual void Proceed() = 0;  
                CallerBase(bsan::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
                    service_(service), cq_(cq), status_(CREATE) {}
            protected:
                enum CallStatus { CREATE, PROCESS, FINISH };
                bsan::BalanceRPC::AsyncService* service_;
                grpc::ServerCompletionQueue* cq_;
                grpc::ServerContext ctx_;
                CallStatus status_; 

        };

        class CreateBalanceCaller final: public CallerBase {
	    public:

	        CreateBalanceCaller(bsan::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
                    CallerBase(service, cq), responder_(&ctx_)
            {
                Proceed();       
            }
	        void Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.

                    service_->RequestCreateBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new CreateBalanceCaller(service_, cq_);

                    // The actual processing.
                    std::string prefix("Hello ");
                    std::cout << "async CreateBalanceCaller: " << request_.name() << std::endl;
                    reply_.set_message(prefix + request_.name());

                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);
                } else {
                    GPR_ASSERT(status_ == FINISH);
                    delete this; // Once in the FINISH state, deallocate ourselves (CallData).
                }
            }
	    private:
            bsan::CreateBalanceRequest request_;
            bsan::CreateBalanceResponce reply_;
            grpc::ServerAsyncResponseWriter<bsan::CreateBalanceResponce> responder_;
        };

        class SetBalanceCaller final: public CallerBase {
	    public:

	        SetBalanceCaller(bsan::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
                    CallerBase(service, cq), responder_(&ctx_)
            {
                Proceed();       
            }
	        void Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.

                    service_->RequestSetBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new SetBalanceCaller(service_, cq_);

                    // The actual processing.
                    std::string prefix("Hello ");
                    std::cout << "async SetBalanceCaller: " << request_.name() << std::endl;
                    reply_.set_message(prefix + request_.name());

                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);
                } else {
                    GPR_ASSERT(status_ == FINISH);
                    delete this; // Once in the FINISH state, deallocate ourselves (CallData).
                }
            }
	    private:
            bsan::SetBalanceRequest request_;
            bsan::SetBalanceResponce reply_;
            grpc::ServerAsyncResponseWriter<bsan::SetBalanceResponce> responder_;
        };

        // This can be run in multiple threads if needed.
        void HandleRpcs() {

            new CreateBalanceCaller(&service_, cq_.get());
            new SetBalanceCaller(&service_, cq_.get());
            void* tag; 
            bool ok;
            while (true) {

                GPR_ASSERT(cq_->Next(&tag, &ok));
                GPR_ASSERT(ok);
                static_cast<CallerBase*>(tag)->Proceed();
            }
        }

        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        bsan::BalanceRPC::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;

};



int main(int argc, char** argv){

    std::cout << "RUN " << std::endl;
    ServerImpl server;
    server.Run( );
 
    return 0;
}