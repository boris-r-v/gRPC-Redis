#include <redis_pool.h>
#include <iostream>


cls_core::redis_pool::redis_pool()
    :next_(list_.end())
    ,event_loop_( std::make_shared<sw::redis::EventLoop>() )
{}


cls_core::redis_pool& cls_core::redis_pool::instance(){
    static redis_pool instance_;
    return instance_;
}
std::shared_ptr<sw::redis::CoRedis> cls_core::redis_pool::get(size_t num_of_conn){
    
    if (list_.empty()){
        if (num_of_conn == 0) num_of_conn = 10;
        for (size_t i=0; i<num_of_conn; ++i){
            sw::redis::ConnectionOptions conn_options;
            conn_options.host = "127.0.0.1";  // Required.
            conn_options.port = 6379; 
            sw::redis::ConnectionPoolOptions pool_options;
            pool_options.size = 1; 
            pool_options.wait_timeout = std::chrono::milliseconds(100);
            pool_options.connection_lifetime = std::chrono::minutes(10);
            list_.push_back( std::make_shared<sw::redis::CoRedis>(sw::redis::CoRedis(conn_options, pool_options ) ) );
            std::cout << "create connection number: " << list_.size() << "\n";
        }
        next_ = list_.begin();
    }
    if( list_.end() == next_ ) next_ = list_.begin();
    //std::cout << "return: " <<std::hex << (&(*next_)) << std::endl;
    return *(next_++); 

}
void cls_core::redis_pool::push( cls_core::redis_t _ptr ){
    list_.push_back( _ptr );    
    std::cout << "redis_pool::pop - return unused connection size:" << list_.size() << "\n";
}
cls_core::redis_t cls_core::redis_pool::pop(){
    if (false == list_.empty()){
        std::cout << "redis_pool::pop1 - take from queue size:" << list_.size() << "\n";
        auto itr = list_.front();
        list_.pop_front();
        std::cout << "redis_pool::pop2 - take from queue size:" << list_.size() << "\n";
        return itr;
    }
    else {
        std::cout << "redis_pool::pop - create new one\n";
        sw::redis::ConnectionOptions conn_options;
        conn_options.host = "127.0.0.1";  // Required.
        conn_options.port = 6379; 
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = 1; 
        pool_options.wait_timeout = std::chrono::milliseconds(100);
        pool_options.connection_lifetime = std::chrono::minutes(10);
        return std::make_shared<sw::redis::CoRedis>(sw::redis::CoRedis(conn_options, pool_options ));// event_loop_) );
    }
     
}