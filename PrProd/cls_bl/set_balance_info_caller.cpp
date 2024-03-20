#include <callers.h>

cls_bl::SetBalanceInfo::SetBalanceInfo(cls_gen::CounterRPC::AsyncService* _as, grpc::ServerCompletionQueue* _cq ):
        CallerBase(_as, _cq),
        responder_(&ctx_)
{
        service_->RequestSetBalanceInfo(&ctx_, &request_, &responder_, cq_, cq_,this);
}

cls_core::Task cls_bl::SetBalanceInfo::Proceed() {
        if( Status::PROCESS == status_){
                std::cout << "Status::PROCESS\n";
                new SetBalanceInfo(service_, cq_ );

                std::string key ("key:");
                key += std::to_string(request_.id());
                std::string data;
                request_.SerializeToString(&data);

                auto ret = co_await redis_->set(key, data );

                std::cout <<"set ret " << std::boolalpha<< ret << std::endl;

                reply_.set_id(request_.id());
                                                        
                status_ = Status::FINISH;
                responder_.Finish(reply_, grpc::Status::OK, this);                
        }
        else{
                std::cout << "Status::FINISH\n";
                delete this;
        }        
        co_return ;
}       


