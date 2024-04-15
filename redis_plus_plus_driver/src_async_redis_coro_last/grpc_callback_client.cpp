#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>

#include <atomic>


std::atomic_ullong gl_duration_;

std::atomic_llong gl_cntr_;
std::mutex mu;
std::condition_variable cv;
bool done = false;  
std::atomic_bool gl_check_ = false;

using ltime = std::chrono::time_point<std::chrono::steady_clock>;

class Client {
    public:
        Client(std::shared_ptr<grpc::Channel> channel)
            :stub_(cls::BalanceRPC::NewStub(channel))
            ,begin(std::chrono::steady_clock::now())    {}

        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        void CreateBalance(const std::string& user, int id ) {
            // Data we are sending to the server.
            //cls::BalanceData request;
            request.set_id(id);
            request.set_name(user);
            request.set_value(100);

            // Container for the data we expect from the server.
            //cls::CreateBalanceResponce reply;

            // Context for the client. It could be used to convey extra information to
            // the server and/or tweak certain RPC behaviors.
            //grpc::ClientContext context;

            // The actual RPC.
            stub_->async()->CreateBalance(&context, &request, &reply,
                                    [this](grpc::Status status) {
                                        if ( !status.ok()) {
                                            std::cout << "RPC Fail" << status.error_code() << ": " << status.error_message() << std::endl;
                                        }
                                        auto end = std::chrono::steady_clock::now();
                                        auto dr = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();         
                                        gl_duration_ += dr;
                                        

                                        //std::cout << reply.message() << std::endl;
                                        
                                        if ( --gl_cntr_ ==0 ){
                                            std::lock_guard<std::mutex> lock(mu);
                                            done = true;
                                            cv.notify_one();
                                        }
                                        
                                    });

            // Act upon its status.
        }
    private:
        std::unique_ptr<cls::BalanceRPC::Stub> stub_;
        cls::BalanceData request;
        cls::CreateBalanceResponce reply;
        grpc::ClientContext context;
        ltime begin;
    
};

int main(int argc, char** argv) {
    if ( argc < 3) {
        std::cout << "Usage: "<< argv[0] <<" ip:port nm_msg\n";
        return 1;
    }
    std::string user("some user");
    size_t num = atoi(argv[2]);

    auto chan = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());
    std::list<Client> list;
    auto begin = std::chrono::steady_clock::now();

    ++gl_cntr_;
    std::cout << "Start sending RPC requests <"<<num<<">\n";
    for ( size_t i=0; i < num; ++i ){
      
        list.emplace_back(chan);
        list.back().CreateBalance( user, i );

        ++gl_cntr_;
    }
    --gl_cntr_;

    //std::cout << "All requests has being sended\n";

    std::unique_lock<std::mutex> lk(mu);
    cv.wait(lk, []{return done;});

    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "CreateBalance takes " << elapsed_ms.count() << "ms to create "<< num <<" records\n";
    std::cout << "Average time to one CreateBalance: " << 0.001*gl_duration_/num << "ms\n";

    return 0;
}
