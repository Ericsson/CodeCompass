#ifndef CC_MONGOOSE_THREADEDMONGOOSE_H
#define CC_MONGOOSE_THREADEDMONGOOSE_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <signal.h>

#include <mongoose/mongoose.h>

namespace cc
{
namespace mongoose
{

class SignalChanger
{
public:
  typedef sighandler_t SignalHandler;
  
  SignalChanger(int signum, SignalHandler newHandler)
  {
    origSignum = signum;
    origHandler = signal(signum, newHandler);
  }

  ~SignalChanger()
  {
    signal(origSignum, origHandler);
  }

private:
  int origSignum;
  SignalHandler origHandler;
};

class ThreadedMongoose
{
public:
  typedef std::function<int (mg_connection *, mg_event)> Handler;
  
  ThreadedMongoose(int numThreads_ = 0) : _numThreads(numThreads_) {}

  void setOption(const std::string& optName, const std::string& value)
  {
    _options[optName] = value;
  }
  
  std::string getOption(const std::string& optName)
  {
    return _options[optName];
  }
  
  void run(Handler handler_)
  {
    run((void*)0, handler_);
  }
  
  template <typename T>
  void run(T* serverData, Handler handler_)
  {
    typedef std::shared_ptr<mg_server> ServerPtr;
    
    handler = handler_;
    
    exitFlag = 0;

    SignalChanger termSig(SIGTERM, signalHandler);
    SignalChanger intSig(SIGINT, signalHandler);
    
    if (_numThreads < 1)
    {
      _numThreads = std::max(std::thread::hardware_concurrency(), 20u);
    }
    
    std::vector<ServerPtr> servers;
    servers.reserve(_numThreads);
    
    std::vector<std::thread> threads;
    threads.reserve(_numThreads - 1);
        
    for (int i = 0; i < _numThreads; ++i)
    {
      ServerPtr server = ServerPtr(
        mg_create_server((void*)serverData, delegater),
        [](mg_server* s)
        {
          mg_destroy_server(&s);
        });
      
      for (const auto& opt : _options)
      {
        // if we create more servers, we have to share the listening socket
        if (opt.first == "listening_port" && i != 0)
        {
          mg_set_listening_socket(
            server.get(), mg_get_listening_socket(servers[0].get()));
        }
        else
        {
          auto errormsg =
            mg_set_option(server.get(), opt.first.c_str(), opt.second.c_str());

          if (errormsg)
          {
            exitFlag = true;

            std::string error = errormsg;
            error += " Option: " + opt.first + " Value: " + opt.second;

            throw std::runtime_error(error);
          }
        }
      }
      
      // if this is not the last server, create a new thread
      if (i != _numThreads - 1)
      {
        threads.push_back(std::thread(serve, server.get()));
      }
      // this is the last server, run serve in current thread
      else
      {
        serve(server.get());
      }
      
      servers.push_back(std::move(server));
    }
    
    // ~joiner waits for all threads to finish
    // ~servers releases the servers' resources
    // ~termSig and ~intSig restores signal handlers
  }
  
private:
  static void signalHandler(int sigNum);
  
  static volatile int exitFlag;
  
  static void* serve(void* server)
  {
    while (!exitFlag)
    {
      mg_poll_server((mg_server*)server, 1000);
    }
    
    return nullptr;
  }
  
  static int delegater(mg_connection *conn, enum mg_event ev)
  {
    if (handler)
    {
      return handler(conn, ev);
    }
    
    return MG_FALSE;
  }
  
  static Handler handler;

  std::map<std::string, std::string> _options;
  int _numThreads = 0;
};

}
}

#endif /* CC_MONGOOSE_THREADEDMONGOOSE_H */