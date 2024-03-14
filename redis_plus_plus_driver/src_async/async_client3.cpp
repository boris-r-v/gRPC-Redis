#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>



class CreateBalanceRpcClient {
  public:
    CreateBalanceRpcClient(std::shared_ptr<grpc::Channel> channel): stub_(bsan::BalanceRPC::NewStub(channel)) {}

    void CreateBalance(const std::string& user) {
      bsan::CreateBalanceRequest request;
      request.set_name(user);


      CreateAsyncCall* call = new CreateAsyncCall;
      call->response_reader = stub_->PrepareAsyncCreateBalance(&call->context, request, &cq_);
      call->response_reader->StartCall();
      call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {

      CreateAsyncCall* call = static_cast<CreateAsyncCall*>(got_tag);
      GPR_ASSERT(ok);
      if (call->status.ok()) std::cout << "CreateBalanceAsyncCall received: " << call->reply.message() << std::endl;
      else std::cout << "RPC failed" << std::endl;
      delete call;
    }
  }

 private:
  struct CreateAsyncCall {
    bsan::CreateBalanceResponce reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<bsan::CreateBalanceResponce> > response_reader;
  };

  std::unique_ptr<bsan::BalanceRPC::Stub> stub_;
  grpc::CompletionQueue cq_;
};

class SetBalanceRpcClient {
  public:
    SetBalanceRpcClient(std::shared_ptr<grpc::Channel> channel): stub_(bsan::BalanceRPC::NewStub(channel)) {}

    void SetBalance(const std::string& user) {
      bsan::SetBalanceRequest request;
      request.set_name(user);


      SetAsyncCall* call = new SetAsyncCall;
      call->response_reader = stub_->PrepareAsyncSetBalance(&call->context, request, &cq_);
      call->response_reader->StartCall();
      call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {

      SetAsyncCall* call = static_cast<SetAsyncCall*>(got_tag);
      GPR_ASSERT(ok);
      if (call->status.ok()) std::cout << "SetBalanceAsyncCall received: " << call->reply.message() << std::endl;
      else std::cout << "RPC failed" << std::endl;
      delete call;
    }
  }

 private:
  struct SetAsyncCall {
    bsan::SetBalanceResponce reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<bsan::SetBalanceResponce> > response_reader;
  };

  std::unique_ptr<bsan::BalanceRPC::Stub> stub_;
  grpc::CompletionQueue cq_;
};


int main(int argc, char** argv) {
  std::string target_str {"127.0.0.1:5678"};
  // We indicate that the channel isn't authenticated (use of InsecureChannelCredentials()).
  CreateBalanceRpcClient client( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  SetBalanceRpcClient client2( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // Spawn reader thread that loops indefinitely
  std::thread thread_ = std::thread(&CreateBalanceRpcClient::AsyncCompleteRpc, &client);
  std::thread thread2_ = std::thread(&SetBalanceRpcClient::AsyncCompleteRpc, &client2);

  for (int i = 0; i < 100; i++) {
    std::string user("world " + std::to_string(i));
    client.CreateBalance(user);  // The actual RPC call!
    client2.SetBalance(user);  // The actual RPC call!
  }

  std::cout << "Press control-c to quit" << std::endl << std::endl;
  thread_.join();  // blocks forever
  thread2_.join();  // blocks forever

  return 0;
}
