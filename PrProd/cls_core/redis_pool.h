#ifndef _redis_pool_h_define_
#define _redis_pool_h_define_

#include <list>
#include <memory>
#include <sw/redis++/co_redis++.h>
#include <mutex>

namespace cls_core
{
    typedef std::shared_ptr<sw::redis::CoRedis> redis_t;
    class redis_pool{
            std::list <std::shared_ptr<sw::redis::CoRedis>> list_;  //FIX ME - have to change to lock-free, thread-save queue 
            std::list <std::shared_ptr<sw::redis::CoRedis>>::iterator next_;
            std::mutex lock_;
            std::shared_ptr<sw::redis::EventLoop> event_loop_;

        public:
            redis_pool();
            static redis_pool& instance();
            std::shared_ptr<sw::redis::CoRedis> get(size_t num_of_conn);
            std::shared_ptr<sw::redis::CoRedis> pop();
            void push(std::shared_ptr<sw::redis::CoRedis>);
            
    };
} // namespace cls_core









#endif  //_redis_h_define_