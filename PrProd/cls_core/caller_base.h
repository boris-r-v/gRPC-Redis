#ifndef _caller_base_define_
#define _caller_base_define_

#include <coroutine>
#include <grpcpp/grpcpp.h>
#include <CLS.grpc.pb.h>

#include <redis_pool.h>

namespace cls_core
{
    struct promise;
    struct Task: std::coroutine_handle<promise>
    {
        using promise_type = cls_core::promise;
    };
    struct promise{
            Task get_return_object() { return Task{}; }
            std::suspend_never initial_suspend() noexcept { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {}
    };

    class CallerBase{
        public:
            CallerBase(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*);
            CallerBase(cls_gen::CounterRPC::AsyncService*, grpc::ServerCompletionQueue*, redis_t );
            ~CallerBase();
            virtual Task Proceed() = 0;

        protected:
            
            enum class Status { CREATE, PROCESS, FINISH };
            cls_gen::CounterRPC::AsyncService* service_;
            grpc::ServerCompletionQueue* cq_;
            grpc::ServerContext ctx_;
            Status status_; 
            redis_t redis_;
    };

} // namespace cls_core

#endif // _caller_base_define_