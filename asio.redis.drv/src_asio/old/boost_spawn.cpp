#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

boost::asio::io_service io_service;

void other_work()
{
  std::cout << "Other work" << std::endl;
}

void other_work2()
{
  std::cout << "Other work2" << std::endl;
}

void my_work(boost::asio::yield_context yield_context)
{
  // Add more work to the io_service.
  io_service.post(&other_work);

  // Wait on a timer within the coroutine.
  boost::asio::deadline_timer timer(io_service);
  timer.expires_from_now(boost::posix_time::seconds(1));
  std::cout << "Start wait" << std::endl;
  timer.async_wait(yield_context);
  std::cout << "Woke up" << std::endl;    
}

void my_work2(boost::asio::yield_context yield_context)
{
  // Add more work to the io_service.
  io_service.post(&other_work2);

  // Wait on a timer within the coroutine.
  boost::asio::deadline_timer timer(io_service);
  timer.expires_from_now(boost::posix_time::seconds(2));
  std::cout << "Start2 wait" << std::endl;
  timer.async_wait(yield_context);
  std::cout << "Woke2 up" << std::endl;    
}

class A{
   public:
      void work(boost::asio::yield_context yield_context){
         boost::asio::deadline_timer timer(io_service);
         timer.expires_from_now(boost::posix_time::seconds(2));
         std::cout << "Start3 wait" << std::endl;
         timer.async_wait(yield_context);
         std::cout << "Woke3 up" << std::endl;    
      }
};

int main ()
{
   A a;
  std::cout << "1" << std::endl;
  boost::asio::spawn(io_service, &my_work);
  std::cout << "2" << std::endl;
  boost::asio::spawn(io_service, &my_work2);
  
  boost::asio::spawn(io_service, [&a](boost::asio::yield_context yield_context){a.work(yield_context);} );

  std::cout << "3" << std::endl;
  io_service.run();
  std::cout << "4" << std::endl;
  return 0;
}

/*
boost::asio::spawn(my_strand, do_echo, boost::asio::detached);

// ...

void do_echo(boost::asio::yield_context yield)
{
  try
  {
    char data[128];
    for (;;)
    {
      std::size_t length = my_socket.async_read_some(boost::asio::buffer(data), yield);

      boost::asio::async_write(my_socket, boost::asio::buffer(data, length), yield);
    }
  }
  catch (std::exception& e)
  {
    // ...
  }
}
*/