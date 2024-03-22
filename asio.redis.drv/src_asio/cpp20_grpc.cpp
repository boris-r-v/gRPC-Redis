/* Copyright (c) 2018-2022 Marcelo Zimbres Silva (mzimbres@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE.txt)
 */

#include <boost/redis/connection.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/consign.hpp>
#include <iostream>



namespace asio = boost::asio;
using boost::redis::request;
using boost::redis::response;
using boost::redis::config;
using boost::redis::connection;
using boost::redis::ignore_t;


class ServerImpl{

    public:
        ~ServerImpl(){
            server_ -> Shutdown();
            cq_ -> Shutdown();
        }

	   ServerImpl(){
         std::string server_address ("0.0.0.0:5678" );
         grpc::ServerBuilder builder;
         builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

         builder.RegisterService(&service_);  // Register "service_" as the instance through which we'll communicate with clients. In this case it corresponds to an *asynchronous* service.
         cq_ = builder.AddCompletionQueue(); 
               
         server_ = builder.BuildAndStart();     
         std::cout << "Server listening on " << server_address << std::endl;
	}


    private:
        class CallerBase {
            public:
               virtual Task Proceed() = 0;  
               CallerBase(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq ):
                     :service_(service)
                     ,cq_(cq)
                     ,status_(PROCESS)
               {}
            protected:
                enum CallStatus { CREATE, PROCESS, FINISH, WAITASYNC };
                cls::BalanceRPC::AsyncService* service_;
                grpc::ServerCompletionQueue* cq_;
                grpc::ServerContext ctx_;
                CallStatus status_; 
        

        };

        
        class GetBalanceCaller final: public CallerBase {
	    public:
	        GetBalanceCaller(cls::BalanceRPC::AsyncService* service, grpc::ServerCompletionQueue* cq )
                  :CallerBase(service, cq)
                  ,responder_(&ctx_)
            {
               service_->RequestGetBalance(&ctx_, &request_, &responder_, cq_, cq_,this);
            }
	        Task Proceed() override {
               if (status_ == PROCESS) {
                  new GetBalanceCaller(service_, cq_, redis_ );

                  // The actual processing.
                  reply_.set_name("???????????????");
                  reply_.set_value (0);
                  reply_.set_id( request_.id());
                  status_ = WAITASYNC;
                  std::string key(std::to_string(request_.id()));
                  std::cout << "proceed0 key:<" << key<< "> \n";

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
      public:
        
         auto HandleRpcs() -> asio::awaitable<void>
         {
            std::osyncstream(std::cout) << "HandleRpcs thread_id<"<< std::this_thread::get_id() <<"> \n";
            new GetBalanceCaller(&service_, cq_.get() );
            void* tag; 
            bool ok;
            while (true) {

               GPR_ASSERT(cq_->Next(&tag, &ok));
               GPR_ASSERT(ok);
                
               static_cast<CallerBase*>(tag)->Proceed();
            }
         }
      private:   
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        cls::BalanceRPC::AsyncService service_;
        std::unique_ptr<grpc::Server> server_;
        std::shared_ptr<sw::redis::CoRedis> redis_;

};

auto co_main(config cfg) -> asio::awaitable<void>
{
   auto conn = std::make_shared<connection>(co_await asio::this_coro::executor);
   conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

   // A request containing only a ping command.
   request req;
   req.push("SET", "x55", "New mesage text");
   req.push("GET", "x55");

   // Response where the PONG response will be stored.
   response<ignore_t, std::string> resp;

   // Executes the request.
   co_await conn->async_exec(req, resp, asio::deferred);

   std::cout << "PING: " << std::get<1>(resp).value() << std::endl;

   auto data = std::get<1>(resp).value();
   data += ", add some";
   // A request containing only a ping command.
   request req1;
   req1.push("SET", "x55", data);
   req1.push("GET", "x55" );
   // Response where the PONG response will be stored.
   response<ignore_t, std::string> resp1;

   // Executes the request.
   co_await conn->async_exec(req1, resp1, asio::deferred);

   conn->cancel();

   std::cout << "PING: " << std::get<1>(resp1).value() << std::endl;
}


