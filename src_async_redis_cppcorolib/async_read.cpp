#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <CustomerLimitStorageRPC.pb.h>
#include <CustomerLimitStorageRPC.grpc.pb.h>

std::atomic_ulong cntr;

class GetBalanceClientImpl {
  public:
    GetBalanceClientImpl(std::shared_ptr<grpc::Channel> channel): stub_(cls::BalanceRPC::NewStub(channel)) {}

    void get_balance( int _id ) {

      cls::GetBalanceRequest request;
      request.set_id( _id );
      ++cntr;
      GetBalanceAsync* call = new GetBalanceAsync;
      call->response_reader = stub_->PrepareAsyncGetBalance(&call->context, request, &cq_);
      call->response_reader->StartCall();
      call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {
      GetBalanceAsync* call = static_cast<GetBalanceAsync*>(got_tag);
      GPR_ASSERT(ok);
      /*
      if (call->status.ok()) std::cout << "Caller received: {id:" << call->reply.id() << ",name:'" << call->reply.name() <<", value: " << call->reply.value() <<"}"<< std::endl;
      else std::cout << "RPC failed" << std::endl;
      */
      if (!(call->status.ok())) std::cout << "RPC failed" << std::endl;
      delete call;
      if ( --cntr <= 0 ) break;
    }
  }

 private:

  struct GetBalanceAsync {
    cls::BalanceData reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<cls::BalanceData> > response_reader;
  };
  std::unique_ptr<cls::BalanceRPC::Stub> stub_;
  grpc::CompletionQueue cq_;
};

int main(int argc, char** argv) {
  std::string target_str {"127.0.0.1:5678"};
  ++cntr;
  GetBalanceClientImpl client( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::thread thread_ = std::thread(&GetBalanceClientImpl::AsyncCompleteRpc, &client);
  auto begin = std::chrono::steady_clock::now();

  for (int i = 1; i < 50*10000; i++) {
    //std::cout <<"Call: " << i << std::endl;
    client.get_balance( i );
  }
  --cntr;
  std::cout << "Press control-c to quit" << std::endl;
  thread_.join();  // blocks forever
  std::cout << "Ups - not block " << std::endl;

  auto end = std::chrono::steady_clock::now();

  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
  std::cout << "async_read takes " << elapsed_ms.count()/10 << "ms to read 50k records\n";

  return 0;
}
