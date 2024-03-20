#ifndef _callers_h_define_
#define _callers_h_define_

#include <caller_base.h>

/**
 * @brief Пространсво имен для описания бизнес логики прлиожения
 * Для добавления нового обработчика gRPC нужно добавить сюда класс его обработки, по аналогии 
 * И не забыть создать на куче один его экземпляр в методе void ServerImpl::HandleRpcs()
 */
namespace cls_bl
{

    class SetBalanceInfo final: public cls_core::CallerBase {
	    public:
            SetBalanceInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*);
            SetBalanceInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*, cls_core::redis_t );
            virtual cls_core::Task Proceed() override;

        private:
            cls_gen::BalanceInfo request_;
            cls_gen::BalanceId reply_;
            grpc::ServerAsyncResponseWriter<cls_gen::BalanceId> responder_;
    };    

    class GetBalanceInfo final: public cls_core::CallerBase {
	    public:
            GetBalanceInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*);
            GetBalanceInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*, cls_core::redis_t);
            virtual cls_core::Task Proceed() override;

        private:
            cls_gen::BalanceId request_;
            cls_gen::BalanceInfo reply_;
            grpc::ServerAsyncResponseWriter<cls_gen::BalanceInfo> responder_;
    };    

    class GetBalanceTechnicalInfo final: public cls_core::CallerBase {
	    public:
            GetBalanceTechnicalInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*);
            GetBalanceTechnicalInfo(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*, cls_core::redis_t );
            virtual cls_core::Task Proceed() override;
            
        private:
            cls_gen::BalanceId request_;
            cls_gen::BalanceTechnicalInfo reply_;
            grpc::ServerAsyncResponseWriter<cls_gen::BalanceTechnicalInfo> responder_;
    };    

} // namespace cls_cmd


#endif //_callers_h_define
    
