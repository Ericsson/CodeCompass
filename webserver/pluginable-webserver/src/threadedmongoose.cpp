/*
 * threadedmongoose.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ezoltbo
 */

#include "threadedmongoose.h"

namespace cc
{
namespace mongoose
{

volatile int ThreadedMongoose::exitFlag = 0;

ThreadedMongoose::Handler ThreadedMongoose::handler;

void ThreadedMongoose::signalHandler(int sigNum)
{ 
  ThreadedMongoose::exitFlag = sigNum;
}

}
}



