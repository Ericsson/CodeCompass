#ifndef CC_WEBSERVER_THREADEDMONGOOSE_H
#define CC_WEBSERVER_THREADEDMONGOOSE_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <signal.h>

#include <webserver/mongoose.h>

namespace cc
{
namespace webserver
{

class SignalChanger
{
public:
  typedef sighandler_t SignalHandler;

  SignalChanger(int signum_, SignalHandler newHandler_);
  ~SignalChanger();

private:
  int _origSignum;
  SignalHandler _origHandler;
};

class ThreadedMongoose
{
public:
  typedef std::function<int (mg_connection *, mg_event)> Handler;

  /**
   * Constructor for creating a multithreaded Mongoose server.
   * @param numThreads_ Number of threads to run the server on. If its value is
   * less than 1 then by default maximum the number of available cores and the
   * value of DEFAULT_MAX_THREAD will be used.
   */
  ThreadedMongoose(int numThreads_ = 0);

  /**
   * This function configures the Mongoose server with the given option. The
   * usable values can be found in this documentation:
   * https://backend.cesanta.com/docs/Options.shtml.
   */
  void setOption(const std::string& optName_, const std::string& value_);

  /**
   * This function returns a configured Mongoose option.
   * TODO: This function now only returns the options which are configured by
   * setOption() function. The options should be read by mg_get_option().
   */
  std::string getOption(const std::string& optName_);

  void run(Handler handler_);

  template <typename T>
  void run(T* serverData_, Handler handler_)
  {
    typedef std::shared_ptr<mg_server> ServerPtr;

    handler = handler_;

    exitFlag = 0;

    SignalChanger termSig(SIGTERM, signalHandler);
    SignalChanger intSig(SIGINT, signalHandler);

    if (_numThreads < 1)
    {
      _numThreads
        = std::max(std::thread::hardware_concurrency(), DEFAULT_MAX_THREAD);
    }

    std::vector<ServerPtr> servers;
    servers.reserve(_numThreads);

    std::vector<std::thread> threads;
    threads.reserve(_numThreads - 1);

    for (int i = 0; i < _numThreads; ++i)
    {
      ServerPtr server = ServerPtr(
        mg_create_server((void*)serverData_, delegater),
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
  static void signalHandler(int sigNum_);
  static void* serve(void* server_);
  static int delegater(mg_connection *conn_, enum mg_event ev_);

  static volatile int exitFlag;
  static Handler handler;
  static const unsigned DEFAULT_MAX_THREAD = 20u;

  std::map<std::string, std::string> _options;
  int _numThreads;
};

} // mongoose
} // cc

#endif /* CC_WEBSERVER_THREADEDMONGOOSE_H */
