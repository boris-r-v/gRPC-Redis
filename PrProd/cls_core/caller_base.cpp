#include <caller_base.h>

//#include <atomic>
//static std::atomic_ulong cntr;

cls_core::CallerBase::CallerBase( cls_gen::CounterRPC::AsyncService* service, grpc::ServerCompletionQueue* cq )
        :service_( service )
        ,cq_( cq )
        ,status_( Status::PROCESS )
        ,redis_(cls_core::redis_pool::instance().get(0)) //- this to work connection pool
{
/*
        sw::redis::ConnectionOptions conn_options;
        conn_options.host = "127.0.0.1";  // Required.
        conn_options.port = 6379; 
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = 1; 
        pool_options.wait_timeout = std::chrono::milliseconds(100);
        pool_options.connection_lifetime = std::chrono::minutes(10);
std::cout << "connection num:" << pool_options.size << std::endl;
        redis_.reset( new sw::redis::CoRedis(conn_options, pool_options) );
//        ++cntr;
*/
}
cls_core::CallerBase::CallerBase( cls_gen::CounterRPC::AsyncService* service, grpc::ServerCompletionQueue* cq, redis_t _rds):
        service_( service ),
        cq_( cq ),
        status_( Status::PROCESS ),
        redis_(_rds)
{
//        ++cntr;
}

cls_core::CallerBase::~CallerBase(){
        //cls_core::redis_pool::instance().push( redis_ );  //- this to work connection pool      
//        --cntr;        std::cout << "num:" << cntr << std::endl;
}