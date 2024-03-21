#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <CLS.pb.h>
#include <CLS.grpc.pb.h>
#include <chrono>

std::atomic_ulong cntr;
class SetBalanceClientImpl {
  public:
    SetBalanceClientImpl(std::shared_ptr<grpc::Channel> channel): stub_(cls_gen::CounterRPC::NewStub(channel)) {}

    void create_balance( int _id ) {
        cls_gen::BalanceInfo request;
	      request.set_id ( _id );
		    request.set_version ( 24*10*_id );
		    request.set_profileid ( 600 + 10*_id);

        SetBalanceInfoAsync* call = new SetBalanceInfoAsync;
        ++cntr;
        call->response_reader = stub_->PrepareAsyncSetBalanceInfo(&call->context, request, &cq_);
        call->response_reader->StartCall();
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {
      SetBalanceInfoAsync* call = static_cast<SetBalanceInfoAsync*>(got_tag);
      GPR_ASSERT(ok);

      if (!(call->status.ok())) std::cout << "RPC failed" << std::endl;
      delete call;
      if ( --cntr <= 0 ) break;
    }
  }

 private:

  struct SetBalanceInfoAsync {
    cls_gen::BalanceId reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<cls_gen::BalanceId> > response_reader;
  };
  std::unique_ptr<cls_gen::CounterRPC::Stub> stub_;
  grpc::CompletionQueue cq_;
};

int main(int argc, char** argv) {
    if ( argc != 2) {
      std::cerr << "Usage set_async number_of_records\n";
      return 0;
    }

    std::string target_str {"127.0.0.1:5678"};
    ++cntr;

    SetBalanceClientImpl client( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::thread thread_ = std::thread(&SetBalanceClientImpl::AsyncCompleteRpc, &client);
    auto begin = std::chrono::steady_clock::now();

    for (int i = 1; i < std::stoi(argv[1]); i++) {
    //std::cout <<"Call: " << i << std::endl;
    client.create_balance( i );
    }
    --cntr;
    std::cout << "Press control-c to quit" << std::endl;
    thread_.join();  // blocks forever
    std::cout << "Ups - not block " << std::endl;

    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "async_create takes " << elapsed_ms.count() << "ms to create <"<< argv[1]<<"> records\n";
    

    return 0;
}
