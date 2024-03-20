#include <caller_base.h>

cls_core::CallerBase::CallerBase( cls_gen::CounterRPC::AsyncService* service, grpc::ServerCompletionQueue* cq ):
        service_( service ),
        cq_( cq ),
        status_( Status::PROCESS )
{
 
}
