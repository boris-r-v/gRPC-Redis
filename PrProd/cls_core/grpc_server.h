#ifndef _grps_server_define_
#define _grps_server_define_

#include <memory>

#include <grpcpp/grpcpp.h>
#include <CLS.grpc.pb.h>


namespace cls_core
{
    namespace gRPC
    {   /**
     * @brief Implementation of gRPC one thread server as describer in official documentation without any upgrade
     * 
     */
        class ServerImpl 
        {
            public:
                ~ServerImpl();
                void run();
                void HandleRpcs();
                void stop();
            private:
                std::unique_ptr<grpc::ServerCompletionQueue> cq_;
                cls_gen::CounterRPC::AsyncService service_;
                std::unique_ptr<grpc::Server> server_;
                bool isWorking;                
        };
    } // namespace grpc
} // namespace cls



#endif // _grps_server_define_