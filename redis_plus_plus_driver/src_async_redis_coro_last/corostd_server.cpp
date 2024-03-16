
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>

#include <coroutine>
#include <sw/redis++/async_redis++.h>

#include <iostream>
#include <syncstream>
#include <thread>

struct promise;
struct Task: std::coroutine_handle<promise>
{
    using promise_type = ::promise;
};
struct promise{
        Task get_return_object() { return Task{}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
};

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
        int num_worker = 2; 
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = num_worker; 
        pool_options.wait_timeout = std::chrono::milliseconds(100);
        pool_options.connection_lifetime = std::chrono::minutes(10);
        std::cout <<"Redis num connetion in pool: "<<pool_options.size<< std::endl;
        sw::redis::ConnectionOptions conn_options;
        conn_options.host = "127.0.0.1";  // Required.
        conn_options.port = 6379; 
        redis_.reset( new sw::redis::AsyncRedis(conn_options, pool_options ) );  //redis connection pool
        	    
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
                virtual Task Proceed() = 0;  
                CallerBase(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::AsyncRedis> rs):
                    service_(service), cq_(cq), status_(CREATE), redis_(rs) {}
            protected:
                enum CallStatus { CREATE, PROCESS, FINISH, WAITASYNC };
                cls::BalanceRPC::AsyncService* service_;
                grpc::ServerCompletionQueue* cq_;
                grpc::ServerContext ctx_;
                CallStatus status_; 
                std::shared_ptr<sw::redis::AsyncRedis> redis_;
                auto get_redis_data(std::string const& _key ){
                    std::osyncstream(std::cout) << "get_redis_data key:<" << _key<< "> thread_id<"<< std::this_thread::get_id() <<"> \n";
                    struct awaitable
                    {
                        std::string const& key;
                        std::shared_ptr<sw::redis::AsyncRedis> redis;
                        std::string ret;
                        bool await_ready() { return false; }
                        void await_suspend(std::coroutine_handle<> h){
                            redis-> get( key ,
                                        [h, this](sw::redis::Future<sw::redis::OptionalString>&& fut){
                                            try{
                                                ret = *(fut.get());
                                            }
                                            catch(sw::redis::Error const& e ){
                                                std::cout << "GetBalanceCaller redis error occur " <<e.what() << std::endl;
                                            }
                                            std::osyncstream(std::cout) << "get_redis_data in callback key:<" << key<< "> thread_id<"<< std::this_thread::get_id() <<"> \n";                                            
                                            h.resume();
                                        } );
                        }
                        std::string await_resume() { return std::move( ret );}
                    };
                    return awaitable{_key, redis_};
                }
                auto set_redis_data(std::string const& _key, std::string const& _data ){
                    struct awaitable
                    {
                        std::string const& key, data;
                        std::shared_ptr<sw::redis::AsyncRedis> redis;
                        bool ret;
                        bool await_ready() { return false; }
                        void await_suspend(std::coroutine_handle<> h){
                            redis-> set( key, data,
                                        [h, this](sw::redis::Future<bool>&& fut){
                                            try{
                                                ret = fut.get();
                                            }
                                            catch (sw::redis::Error const& e ){
                                                std::cout << "CreateBalanceCaller redis error occur " <<e.what() << std::endl;
                                                ret = false;
                                            }
                                            h.resume();
                                        } );
                        }
                        bool await_resume() { return ret;}
                    };
                    return awaitable{_key, _data, redis_};
                }

        };

        class CreateBalanceCaller final: public CallerBase {
	    public:
	        CreateBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::AsyncRedis> rs):
                    CallerBase(service, cq, rs), responder_(&ctx_)
            {
                Proceed();       
            }
	        Task Proceed() override {
	            if (status_ == CREATE) {
                    status_ = PROCESS; // Make this instance progress to the PROCESS state.
                    service_->RequestCreateBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
                } else if (status_ == PROCESS) {
                    new CreateBalanceCaller(service_, cq_, redis_ );

                    status_ = WAITASYNC;
                    std::string prefix ("-create");
                    reply_.set_message( std::to_string( request_.id() ) + prefix  );

                    std::string data;
                    request_.SerializeToString(&data);
                    auto ret = co_await set_redis_data( std::to_string(request_.id()), data );

                    ret = co_await set_redis_data( reply_.message(), data );

                    if ( ret ) ;//fixme do some useful with bool retuirn value
                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);

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

	        GetBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, std::shared_ptr<sw::redis::AsyncRedis> rs):
                    CallerBase(service, cq, rs), responder_(&ctx_)
            {
                Proceed();       
            }
	        Task Proceed() override {
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

                    try{
                        std::string key (std::to_string(request_.id()));
                        std::osyncstream(std::cout) << "proceed1 key:<" << key<< "> thread_id<"<< std::this_thread::get_id() <<"> \n";
                        auto data = co_await get_redis_data( key );
                        std::osyncstream(std::cout) << "proceed2 key:<" << key<< "> thread_id<"<< std::this_thread::get_id() <<"> \n";
                        if (!data.empty()) reply_.ParseFromString(data);
                        else std::cout << "GetBalanceCaller data by key " << request_.id() << " not exists" << std::endl;


                        std::string prefix (reply_.name()+"-create");
                        std::osyncstream(std::cout) << "proceed3 key:<" << prefix << "> thread_id<"<< std::this_thread::get_id() <<"> \n";
                        auto val = co_await get_redis_data( prefix );
                        std::osyncstream(std::cout) << "proceed4 key:<" << prefix << "> thread_id<"<< std::this_thread::get_id() <<"> \n";
                        if (!val.empty()) {
                            reply_.ParseFromString(val);
                            reply_.set_name( prefix );
                        }
                        else std::cout << "GetBalanceCaller 222 data by key " << request_.id() << " not exists" << std::endl;

                    }
                    catch(sw::redis::Error const& e ){
                        std::cout << "GetBalanceCaller redis error occur " <<e.what() << std::endl;
                    }

                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);

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
        std::shared_ptr<sw::redis::AsyncRedis> redis_;

};



int main(int argc, char** argv){

    std::cout << "RUN " << std::endl;
    ServerImpl server;
    server.Run( );
 
    return 0;
}
