#include <string>
#include <iostream>
#include <memory>


#include <boost/redis.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;


class sync_connection {
   public:
      sync_connection(): 
         ioc_{1}, 
         conn_{std::make_shared<boost::redis::connection>(ioc_)}
      { 

      } 

      ~sync_connection()
      {   
         thread_.join();
      }   

      void run(boost::redis::config cfg)
      {   
         // Starts a thread that will can io_context::run on which the
         // connection will run.
         thread_ = std::thread{[this, cfg]() {
            conn_->async_run(cfg, {}, boost::asio::detached);
            ioc_.run();
         }}; 
      }   

      void stop()
      {   
         boost::asio::dispatch(ioc_, [this]() { conn_->cancel(); }); 
      }   

      template <class Response>
      auto exec(boost::redis::request const& req, Response& resp)
      {
         boost::asio::dispatch(
            conn_->get_executor(),
            boost::asio::deferred([this, &req, &resp]() { return conn_->async_exec(req, resp, boost::asio::deferred); }))
            (boost::asio::use_future).get();
      }

   private:
      boost::asio::io_context ioc_{1};
      std::shared_ptr<boost::redis::connection> conn_;
      std::thread thread_;
};



auto main(int argc, char * argv[]) -> int 
{
   try {
      boost::redis::config cfg;

      if (argc == 3) {
         cfg.addr.host = argv[1];
         cfg.addr.port = argv[2];
      }   

      sync_connection conn;
      conn.run(cfg);

      boost::redis::request req;
      req.push("PING");

      boost::redis::response<std::string> resp;

      conn.exec(req, resp);
      conn.stop();

      std::cout << "Response: " << std::get<0>(resp).value() << std::endl;

   } catch (std::exception const& e) {
      std::cerr << e.what() << std::endl;
   }   
}
  
