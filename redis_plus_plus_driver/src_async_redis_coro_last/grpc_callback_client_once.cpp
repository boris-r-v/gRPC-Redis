#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include <grpcpp/grpcpp.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>


class Client {
 public:
    Client(std::shared_ptr<grpc::Channel> channel): stub_(cls::BalanceRPC::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string CreateBalance(const std::string& user) {
    // Data we are sending to the server.
    cls::BalanceData request;
    request.set_id(1);
    request.set_name(user);
    request.set_value(100);

    // Container for the data we expect from the server.
    cls::CreateBalanceResponce reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    // The actual RPC.
    std::mutex mu;
    std::condition_variable cv;
    bool done = false;
    grpc::Status status;
    stub_->async()->CreateBalance(&context, &request, &reply,
                             [&mu, &cv, &done, &status](grpc::Status s) {
                                std::cout << "Got status " << status.ok() << std::endl;
                               status = std::move(s);
                               std::lock_guard<std::mutex> lock(mu);
                               done = true;
                               cv.notify_one();
                             });

    std::unique_lock<std::mutex> lock(mu);
    while (!done) {
      cv.wait(lock);
    }

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<cls::BalanceRPC::Stub> stub_;
};

int main(int argc, char** argv) {
    if ( argc < 2) {
        std::cout << "Usage: "<< argv[0] <<" ip:port\n";
        return 1;
    }

  
  Client greeter( grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials()));
  std::string user("some user");
  std::string reply = greeter.CreateBalance(user);
  std::cout << "CreateBalance received: " << reply << std::endl;

  return 0;
}
