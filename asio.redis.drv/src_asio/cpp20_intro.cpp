/* Copyright (c) 2018-2022 Marcelo Zimbres Silva (mzimbres@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE.txt)
 */

#include <boost/redis/connection.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/consign.hpp>
#include <iostream>



namespace asio = boost::asio;
using boost::redis::request;
using boost::redis::response;
using boost::redis::config;
using boost::redis::connection;
using boost::redis::ignore_t;



auto co_main(config cfg) -> asio::awaitable<void>
{
   auto conn = std::make_shared<connection>(co_await asio::this_coro::executor);
   conn->async_run(cfg, {}, asio::consign(asio::detached, conn));

   // A request containing only a ping command.
   request req;
   req.push("SET", "x55", "New mesage text");
   req.push("GET", "x55");

   // Response where the PONG response will be stored.
   response<ignore_t, std::string> resp;

   // Executes the request.
   co_await conn->async_exec(req, resp, asio::deferred);

   std::cout << "PING: " << std::get<1>(resp).value() << std::endl;

   auto data = std::get<1>(resp).value();
   data += ", add some";
   // A request containing only a ping command.
   request req1;
   req1.push("SET", "x55", data);
   req1.push("GET", "x55" );
   // Response where the PONG response will be stored.
   response<ignore_t, std::string> resp1;

   // Executes the request.
   co_await conn->async_exec(req1, resp1, asio::deferred);

   conn->cancel();

   std::cout << "PING: " << std::get<1>(resp1).value() << std::endl;
}


