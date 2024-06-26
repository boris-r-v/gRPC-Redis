
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>

#include <sw/redis++/co_redis++.h>
#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>

std::ostream& operator<<(std::ostream& s, cls::BalanceData const& d){
    s<<"{\"id\":" << d.id() << ", \"name\":\"" << d.name() <<"\", \"value\":" << d.value() <<"}";
    return s;
}

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
        int num_worker = 4; 
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = 3*num_worker; 
        pool_options.wait_timeout = std::chrono::milliseconds(100);
        pool_options.connection_lifetime = std::chrono::minutes(10);
        std::cout <<"Redis num connetion in pool: "<<pool_options.size<< std::endl;
        sw::redis::ConnectionOptions conn_options;
        conn_options.host = "127.0.0.1";  // Required.
        conn_options.port = 6379; 
        //redis_.reset( new sw::redis::CoRedis("tcp://127.0.0.1:6379", conn_options, pool_options ) );  //redis connection pool
        redis_.reset( new sw::redis::CoRedis(conn_options, pool_options ) );  //redis connection pool
        	    
        server_ = builder.BuildAndStart();     // Finally assemble the server.
	    std::cout << "Server listening on " << server_address << std::endl;
	    //HandleRpcs(); // Proceed to the server's main loop.

        std::vector<std::thread> pool;
        pool.reserve(num_worker);
        for (int i=0; i<num_worker; ++i){
            std::cout <<"Run " << i+1 << " worker"<< std::endl;
            pool.emplace_back( &ServerImpl::HandleRpcs, this );
        }

        for (auto& x : pool){
            x.join();
        }

	}


    private:
        class CallerBase {
            public:
                virtual void Proceed() = 0;  
                CallerBase(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::CoRedis> rs):
                    service_(service), cq_(cq), status_(CREATE), redis_(rs) {}
            protected:
                enum CallStatus { CREATE, PROCESS, FINISH, WAITASYNC };
                cls::BalanceRPC::AsyncService* service_;
                grpc::ServerCompletionQueue* cq_;
                grpc::ServerContext ctx_;
                CallStatus status_; 
                std::shared_ptr<sw::redis::CoRedis> redis_;

        };

        class CreateBalanceCaller final: public CallerBase {
	    public:
	        CreateBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::CoRedis> rs):
                    CallerBase(service, cq, rs), responder_(&ctx_)
            {
                Proceed();       
            }
	        void Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.
                    service_->RequestCreateBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new CreateBalanceCaller(service_, cq_, redis_ );

                    status_ = WAITASYNC;
                    std::string prefix ("Create ");
                    reply_.set_message(prefix + std::to_string( request_.id() ) );

                    std::string data;
                    request_.SerializeToString(&data);

                    cppcoro::sync_wait([this, &data]() -> cppcoro::task<> {
                            try {
                                co_await redis_->set(std::to_string(request_.id()), data );

                                status_ = FINISH;
                                responder_.Finish(reply_, grpc::Status::OK, this);
                            } catch (const sw::redis::Error &e) {
                                std::cout << "GreateBalanceCaller redis error occur "<< e.what() << std::endl;
                            }
                        }());                                    
                } else if(status_ == FINISH) {
                    //GPR_ASSERT(status_ == FINISH);
                    delete this; 
                }
                else {
                    std::cout << "call CreateBalanceCaller::Proceed while async from redis " << std::endl;
                }
            }
	    private:
            cls::BalanceData request_;
            cls::CreateBalanceResponce reply_;
            grpc::ServerAsyncResponseWriter<cls::CreateBalanceResponce> responder_;
        };

        class GetBalanceCaller final: public CallerBase {
	    public:

	        GetBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::CoRedis> rs):
                    CallerBase(service, cq, rs), responder_(&ctx_)
            {
                Proceed();       
            }
	        void Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.

                    service_->RequestGetBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new GetBalanceCaller(service_, cq_, redis_ );

                    // The actual processing.
                    reply_.set_name("???????????????");
                    reply_.set_value (0);
                    reply_.set_id( request_.id());
                    status_ = WAITASYNC;

                    cppcoro::sync_wait([this]() -> cppcoro::task<> {
                            try {
                                auto data = co_await redis_ -> get( std::to_string(request_.id()) );
                                if(data) reply_.ParseFromString(*data);
                                else std::cout << "GetBalanceCaller data by key " << request_.id() << " not exists" << std::endl;

                                status_ = FINISH;
                                responder_.Finish(reply_, grpc::Status::OK, this);
                                } catch (const sw::redis::Error &e) {
                                    std::cout << "GreateBalanceCaller redis error occur "<< e.what() << std::endl;
                                }
                        }());    
                } else if(status_ == FINISH) {
                    //GPR_ASSERT(status_ == FINISH);
                    delete this; 
                }else {
                    std::cout << "call GetBalanceCaller::Proceed while async from redis " << std::endl;
                }
            }
	    private:
            cls::GetBalanceRequest request_;
            cls::BalanceData reply_;
            grpc::ServerAsyncResponseWriter<cls::BalanceData> responder_;
        };

        // This can be run in multiple threads if needed.
        void HandleRpcs() {

            new CreateBalanceCaller(&service_, cq_.get(), redis_ );
            new GetBalanceCaller(&service_, cq_.get(), redis_ );
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
        std::shared_ptr<sw::redis::CoRedis> redis_;

};



int main(int argc, char** argv){

    std::cout << "RUN " << std::endl;
    ServerImpl server;
    server.Run( );
 
    return 0;
}
