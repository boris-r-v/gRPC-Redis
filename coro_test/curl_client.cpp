
#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <coroutine>

#include <curl/curl.h>

class WebClient
{
    public:
    WebClient();
    ~WebClient();

    struct Result
    {
        int code;
        std::string data;
    };

    using CallbackFn = std::function<void(Result result)>;
    void runLoop();
    void stopLoop();
    void performRequest(const std::string& url, CallbackFn cb);

    private:
    struct Request
    {
        CallbackFn callback;
        std::string buffer;
    };

    static size_t writeToBuffer(char* ptr, size_t, size_t nmemb, void* tab)
    {
        auto r = reinterpret_cast<Request*>(tab);
        r->buffer.append(ptr, nmemb);
        return nmemb;
    }

    CURLM* m_multiHandle;
    std::atomic_bool m_break{false};
};

WebClient::WebClient()
{
    m_multiHandle = curl_multi_init();
}
WebClient::~WebClient()
{
    curl_multi_cleanup(m_multiHandle);
}

void WebClient::performRequest(const std::string& url, CallbackFn cb)
{
    Request* requestPtr = new Request{std::move(cb), {}};
    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &WebClient::writeToBuffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, requestPtr);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, requestPtr);
    curl_multi_add_handle(m_multiHandle, handle);
}

void WebClient::stopLoop()
{
    m_break = true;
    curl_multi_wakeup(m_multiHandle);
}

void WebClient::runLoop()
{
  int msgs_left;
  int still_running = 1;
 
  while (!m_break) {
    curl_multi_perform(m_multiHandle, &still_running);
    curl_multi_poll(m_multiHandle, nullptr, 0, 1000, nullptr);

    CURLMsg* msg;
    while (!m_break && (msg = curl_multi_info_read(m_multiHandle, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            CURL* handle = msg->easy_handle;
            int code;
            Request* requestPtr;
            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
            curl_easy_getinfo(handle, CURLINFO_PRIVATE, &requestPtr);
            
            requestPtr->callback({code, std::move(requestPtr->buffer)});
            curl_multi_remove_handle(m_multiHandle, handle);
            curl_easy_cleanup(handle);
            delete requestPtr;
        }
    }
  }
}
int main(void)
{
  WebClient client;
  std::thread worker(std::bind(&WebClient::runLoop, &client));
  
  client.performRequest("https://postman-echo.com/get", [](WebClient::Result res)
  {
      std::cout << "Req0 Code: " << res.code << std::endl;
      std::cout << "Req0 Data: '" << res.data << "'" << std::endl << std::endl;
  });
  
  client.performRequest("http://www.gstatic.com/generate_204", [&](WebClient::Result res1)
  {
      std::cout << "Req1 Code: " << res1.code << std::endl;
      std::cout << "Req1 Data: '" << res1.data << "'" << std::endl << std::endl;
      client.performRequest("http://httpbin.org/user-agent", [](WebClient::Result res2)
      {
          std::cout << "Req1-2 Code: " << res2.code << std::endl;
          std::cout << "Req1-2 Data: '" << res2.data << "'" << std::endl << std::endl;
      });
  });
  
  client.performRequest("http://httpbin.org/ip", [](WebClient::Result res)
  {
      std::cout << "Req2 Code: " << res.code << std::endl;
      std::cout << "Req2 Data: '" << res.data << "'" << std::endl << std::endl;
  });
  
  std::cin.get();
  client.stopLoop();
  worker.join();

  return 0;
}