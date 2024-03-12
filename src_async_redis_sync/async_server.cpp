
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>

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
                CallerBase(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
                    service_(service), cq_(cq), status_(CREATE) {}
            protected:
                enum CallStatus { CREATE, PROCESS, FINISH };
                cls::BalanceRPC::AsyncService* service_;
                grpc::ServerCompletionQueue* cq_;
                grpc::ServerContext ctx_;
                CallStatus status_; 

        };

        class CreateBalanceCaller final: public CallerBase {
	    public:
	        CreateBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
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

                    std::string prefix ("Create ");
                    reply_.set_message(prefix + std::to_string( request_.id() ) );

                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);
                } else {
                    GPR_ASSERT(status_ == FINISH);
                    delete this; 
                }
            }
	    private:
            cls::BalanceData request_;
            cls::CreateBalanceResponce reply_;
            grpc::ServerAsyncResponseWriter<cls::CreateBalanceResponce> responder_;
        };

        class GetBalanceCaller final: public CallerBase {
	    public:

	        GetBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq):
                    CallerBase(service, cq), responder_(&ctx_)
            {
                Proceed();       
            }
	        void Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.

                    service_->RequestGetBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new GetBalanceCaller(service_, cq_);

                    // The actual processing.
                    reply_.set_name("blankstring");
                    reply_.set_value (111);
                    reply_.set_id( request_.id());

                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);
                } else {
                    GPR_ASSERT(status_ == FINISH);
                    delete this; // Once in the FINISH state, deallocate ourselves (CallData).
                }
            }
	    private:
            cls::GetBalanceRequest request_;
            cls::BalanceData reply_;
            grpc::ServerAsyncResponseWriter<cls::BalanceData> responder_;
        };

        // This can be run in multiple threads if needed.
        void HandleRpcs() {

            new CreateBalanceCaller(&service_, cq_.get());
            new GetBalanceCaller(&service_, cq_.get());
            void* tag; 
            bool ok;
            while (true) {

                GPR_ASSERT(cq_->Next(&tag, &ok));
                GPR_ASSERT(ok);
                
                static_cast<CallerBase*>(tag)->Proceed();
            }
        }

        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        cls::BalanceRPC::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;

};



int main(int argc, char** argv){

    std::cout << "RUN " << std::endl;
    ServerImpl server;
    server.Run( );
 
    return 0;
}
