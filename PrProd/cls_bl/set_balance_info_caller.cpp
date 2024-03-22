#include <callers.h>

cls_bl::SetBalanceInfo::SetBalanceInfo(cls_gen::CounterRPC::AsyncService* _as, grpc::ServerCompletionQueue* _cq ):
        CallerBase(_as, _cq),
        responder_(&ctx_)
{
        service_->RequestSetBalanceInfo(&ctx_, &request_, &responder_, cq_, cq_,this);
}

cls_bl::SetBalanceInfo::SetBalanceInfo(cls_gen::CounterRPC::AsyncService* _as, grpc::ServerCompletionQueue* _cq, cls_core::redis_t _rd ):
        CallerBase(_as, _cq, _rd),
        responder_(&ctx_)
{
        service_->RequestSetBalanceInfo(&ctx_, &request_, &responder_, cq_, cq_,this);
}

cls_core::Task cls_bl::SetBalanceInfo::Proceed() {
        if( Status::PROCESS == status_){

                //new SetBalanceInfo(service_, cq_, redis_ );
                new SetBalanceInfo(service_, cq_ );

                std::string key ("key:");
                key += std::to_string(request_.id());
                std::string data;
                request_.SerializeToString(&data);

                auto ret = co_await redis_->set(key, data );
for(int x=0; x<10; ++x){
                key += "_1";
                ret &= co_await redis_->set(key, data );
}

                status_ = Status::FINISH;
                reply_.set_id(request_.id());
                if ( ret )        
                        responder_.Finish(reply_, grpc::Status::OK, this);                
                else
                        responder_.Finish(reply_, grpc::Status::CANCELLED, this);      
        }
        else{
                delete this;
        }        
        co_return ;
}       


