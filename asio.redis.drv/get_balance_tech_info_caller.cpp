#include <callers.h>

cls_bl::GetBalanceTechnicalInfo::GetBalanceTechnicalInfo(cls_gen::CounterRPC::AsyncService* _as, grpc::ServerCompletionQueue* _cq ):
        CallerBase(_as, _cq),
        responder_(&ctx_)
{
        service_->RequestGetBalanceTechnicalInfo(&ctx_, &request_, &responder_, cq_, cq_,this);
}

cls_bl::GetBalanceTechnicalInfo::GetBalanceTechnicalInfo(cls_gen::CounterRPC::AsyncService* _as, grpc::ServerCompletionQueue* _cq, cls_core::redis_t _rd ):
        CallerBase(_as, _cq, _rd ),
        responder_(&ctx_)
{
        service_->RequestGetBalanceTechnicalInfo(&ctx_, &request_, &responder_, cq_, cq_,this);
}

cls_core::Task cls_bl::GetBalanceTechnicalInfo::Proceed() {
        if( Status::PROCESS == status_){
                //new GetBalanceTechnicalInfo(service_, cq_, redis_ );
                new GetBalanceTechnicalInfo(service_, cq_ );

                //co_await redis_->set(std::to_string(request_.id()), data );

                status_ = Status::FINISH;
                responder_.Finish(reply_, grpc::Status::OK, this);                
        }
        else{
                delete this;
        }
        co_return ;
}       


