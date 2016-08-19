#include "threadedmongoose.h"

namespace cc
{
namespace webserver
{

SignalChanger::SignalChanger(int signum_, SignalHandler newHandler_)
{
  _origSignum = signum_;
  _origHandler = signal(signum_, newHandler_);
}

SignalChanger::~SignalChanger()
{
  signal(_origSignum, _origHandler);
}

volatile int ThreadedMongoose::exitFlag = 0;

const unsigned ThreadedMongoose::DEFAULT_MAX_THREAD;

ThreadedMongoose::Handler ThreadedMongoose::handler;

ThreadedMongoose::ThreadedMongoose(int numThreads_) : _numThreads(numThreads_)
{
}

void ThreadedMongoose::setOption(
  const std::string& optName_,
  const std::string& value_)
{
  _options[optName_] = value_;
}

std::string ThreadedMongoose::getOption(const std::string& optName_)
{
  return _options[optName_];
}

void ThreadedMongoose::run(Handler handler_)
{
  run((void*)0, handler_);
}

void ThreadedMongoose::signalHandler(int sigNum_)
{ 
  ThreadedMongoose::exitFlag = sigNum_;
}

void* ThreadedMongoose::serve(void* server_)
{
  while (!exitFlag)
  {
    mg_poll_server((mg_server*)server_, 1000);
  }
  
  return nullptr;
}

int ThreadedMongoose::delegater(mg_connection *conn_, enum mg_event ev_)
{
  if (handler)
  {
    return handler(conn_, ev_);
  }
  
  return MG_FALSE;
}

} // webserver
} // cc
