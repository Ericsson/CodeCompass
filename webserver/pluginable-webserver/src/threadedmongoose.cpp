#include <mongoose/threadedmongoose.h>

namespace cc
{
namespace mongoose
{

volatile int ThreadedMongoose::exitFlag = 0;

ThreadedMongoose::Handler ThreadedMongoose::handler;

void ThreadedMongoose::signalHandler(int sigNum_)
{ 
  ThreadedMongoose::exitFlag = sigNum_;
}

} // mongoose
} // cc