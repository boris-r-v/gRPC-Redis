/* Copyright (c) 2018-2022 Marcelo Zimbres Silva (mzimbres@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE.txt)
 */

#include <boost/redis/connection.hpp>
#include <boost/redis/config.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>

namespace asio = boost::asio;
using boost::redis::config;
using boost::redis::logger;


extern asio::awaitable<void> co_main(config);

auto main(int argc, char * argv[]) -> int
{
   try {
      config cfg;

      if (argc == 3) {
         cfg.addr.host = argv[1];
         cfg.addr.port = argv[2];
      }
      else{
         cfg.addr.host="127.0.0.1";
         cfg.addr.port="6379";
      }
      asio::io_context ioc;
      asio::co_spawn(ioc, co_main(cfg), [](std::exception_ptr p) {
         if (p)
            std::rethrow_exception(p);
      });
      ioc.run();

   } catch (std::exception const& e) {
      std::cerr << "(main) " << e.what() << std::endl;
      return 1;
   }
}

