#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <proto/CLS.pb.h>
#include <proto/CLS.grpc.pb.h>

using ltime = std::chrono::time_point<std::chrono::steady_clock>;

std::atomic_ulong cntr;
std::atomic_ullong duration_;

class CreateBalanceClientImpl {
  public:
    CreateBalanceClientImpl(std::shared_ptr<grpc::Channel> channel): stub_(cls::BalanceRPC::NewStub(channel)) {}

    void create_balance( const std::string& name, int id, int value) {
        cls::BalanceData request;
        request.set_name(name);
        request.set_id(id);
        request.set_value(value);

        CreateBalanceAsync* call = new CreateBalanceAsync;
        ++cntr;
        call->response_reader = stub_->PrepareAsyncCreateBalance(&call->context, request, &cq_);
        call->response_reader->StartCall();
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {
      CreateBalanceAsync* call = static_cast<CreateBalanceAsync*>(got_tag);
      GPR_ASSERT(ok);
      //if (call->status.ok()) std::cout << "Caller received: " << call->reply.message()<< std::endl;
      //else std::cout << "RPC failed" << std::endl;
      duration_ += call->getDur();
      if (!(call->status.ok())) std::cout << "RPC failed" << std::endl;
      delete call;
      if ( --cntr <= 0 ) break;
    }
  }

 private:

  struct CreateBalanceAsync {
    CreateBalanceAsync(): begin(std::chrono::steady_clock::now()) {}
    unsigned long getDur(){
      auto end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();         
    }
    ltime begin;
    cls::CreateBalanceResponce reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<cls::CreateBalanceResponce> > response_reader;
  };
  std::unique_ptr<cls::BalanceRPC::Stub> stub_;
  grpc::CompletionQueue cq_;
};

int main(int argc, char** argv) {

    if ( argc !=3) {
      std::cout << "Usage: "<< argv[0] <<" ip:port nm_msg\n";
      return 0;
    }

    std::string target_str {argv[1]};
    ++cntr;

    CreateBalanceClientImpl client( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::thread thread_ = std::thread(&CreateBalanceClientImpl::AsyncCompleteRpc, &client);
    auto begin = std::chrono::steady_clock::now();


    int num = atoi(argv[2]);

    for (int i = 1; i < num; i++) {
    //std::cout <<"Call: " << i << std::endl;
    client.create_balance( std::to_string(i), i, i );
    }
    --cntr;
    std::cout << "Press control-c to quit" << std::endl;
    thread_.join();  // blocks forever
    std::cout << "Ups - not block " << std::endl;

    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "async_create takes " << elapsed_ms.count() << "ms to create "<< num <<" records\n";
    std::cout << "Average time to one CreateBalance: " << 0.001*duration_/num << "ms\n";

    return 0;
}
