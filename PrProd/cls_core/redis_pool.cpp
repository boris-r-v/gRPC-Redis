#include <redis_pool.h>
#include <iostream>


cls_core::redis_pool& cls_core::redis_pool::instance(){
    static redis_pool instance_;
    return instance_;
}

void cls_core::redis_pool::push( std::shared_ptr<sw::redis::CoRedis> _ptr ){
    std::lock_guard l(lock_);
    list_.push_back( _ptr );    
    std::cout << "redis_pool::pop - return unused connection size:" << list_.size() << "\n";
}
std::shared_ptr<sw::redis::CoRedis> cls_core::redis_pool::pop(){
    std::lock_guard l(lock_);    
    if (false == list_.empty()){
        std::cout << "redis_pool::pop - take from queue\n";
        auto itr = list_.front();
        list_.pop_front();
        return itr;
    }
    else{
        std::cout << "redis_pool::pop - create new one\n";
        sw::redis::ConnectionOptions conn_options;
        conn_options.host = "127.0.0.1";  // Required.
        conn_options.port = 6379; 
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = 1; 
        pool_options.wait_timeout = std::chrono::milliseconds(100);
        pool_options.connection_lifetime = std::chrono::minutes(10);
        return std::make_shared<sw::redis::CoRedis>(sw::redis::CoRedis(conn_options, pool_options));
    }
    
}