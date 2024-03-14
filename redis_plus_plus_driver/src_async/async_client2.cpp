#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <BalanceStorageRPC.pb.h>
#include <BalanceStorageRPC.grpc.pb.h>


class CustomBalanceRpcClient {
  public:
    CustomBalanceRpcClient(std::shared_ptr<grpc::Channel> channel): stub_(bsan::BalanceRPC::NewStub(channel)) {}

    void CreateBalance(const std::string& user) {
      // Data we are sending to the server.
      bsan::CreateBalanceRequest request;
      request.set_name(user);

      // Call object to store rpc data
      AsyncClientCall* call = new AsyncClientCall;

      // stub_->PrepareAsyncSayHello() creates an RPC object, returning
      // an instance to store in "call" but does not actually start the RPC
      // Because we are using the asynchronous API, we need to hold on to
      // the "call" instance in order to get updates on the ongoing RPC.
      call->response_reader = stub_->PrepareAsyncCreateBalance(&call->context, request, &cq_);

      // StartCall initiates the RPC call
      call->response_reader->StartCall();

      // Request that, upon completion of the RPC, "reply" be updated with the
      // server's response; "status" with the indication of whether the operation
      // was successful. Tag the request with the memory address of the call
      // object.
      call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      if (call->status.ok()) std::cout << "Caller received: " << call->reply.message() << std::endl;
      else std::cout << "RPC failed" << std::endl;
      // Once we're complete, deallocate the call object.
      delete call;
    }
  }

 private:
  // struct for keeping state and data information
  struct AsyncClientCall {
    // Container for the data we expect from the server.
    bsan::CreateBalanceResponce reply;

    // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    // Storage for the status of the RPC upon completion.
    grpc::Status status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<bsan::CreateBalanceResponce> > response_reader;
  };

  // Out of the passed in Channel comes the stub, stored here, our view of the server's exposed services.
  std::unique_ptr<bsan::BalanceRPC::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the gRPC runtime.
  grpc::CompletionQueue cq_;
};

int main(int argc, char** argv) {
  std::string target_str {"127.0.0.1:5678"};
  // We indicate that the channel isn't authenticated (use of InsecureChannelCredentials()).
  CustomBalanceRpcClient client( grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // Spawn reader thread that loops indefinitely
  std::thread thread_ = std::thread(&CustomBalanceRpcClient::AsyncCompleteRpc, &client);

  for (int i = 0; i < 100; i++) {
    std::string user("world " + std::to_string(i));
    client.CreateBalance(user);  // The actual RPC call!
  }

  std::cout << "Press control-c to quit" << std::endl << std::endl;
  thread_.join();  // blocks forever

  return 0;
}
