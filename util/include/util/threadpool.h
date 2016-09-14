/*
 * threadpool.h
 *
 *  Created on: Aug 7, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_THREADPOOL_H_
#define UTIL_THREADPOOL_H_

#include <thread>
#include <functional>
#include <future>
#include <vector>

#include "tsqueue.h"

namespace cc 
{
namespace util 
{

class FunctionWrapper
{
  struct ImplBase
  {
    virtual void call()=0;
    virtual ~ImplBase()
    {
    }
  };

  std::unique_ptr<ImplBase> impl;

  template<typename F>
  struct ImplType : ImplBase
  {
    F f;
    ImplType(F&& f_)
      : f(std::move(f_))
    {
    }

    void call()
    {
      f();
    }
  };

public:
  template<typename F>
  FunctionWrapper(F&& f)
    : impl(new ImplType<F>(std::move(f)))
  {
  }
  void operator()()
  {
    impl->call();
  }
  FunctionWrapper() = default;
  FunctionWrapper(FunctionWrapper&& other)
    : impl(std::move(other.impl))
  {
  }

  FunctionWrapper& operator=(FunctionWrapper&& other)
  {
    impl = std::move(other.impl);
    return *this;
  }

  FunctionWrapper(const FunctionWrapper&) = delete;
  FunctionWrapper(FunctionWrapper&) = delete;
  FunctionWrapper& operator=(const FunctionWrapper&) = delete;
};

class JoinThreads
{
  std::vector<std::thread>& threads;
public:
  explicit JoinThreads(std::vector<std::thread>& threads_)
    : threads(threads_)
  {
  }

  ~JoinThreads()
  {
    for (auto &t : threads)
    {
      if (t.joinable())
        t.join();
    }
  }
};

/**
 * Implementation of a thread pool
 * based on Anthony Williams: C++ concurrency in action
 * 9.1.: Thread pools
 */
class ThreadPool
{
public:
  ThreadPool(unsigned int threadCount) : joiner(threads)
  {
    init(threadCount);
  }
  
  ThreadPool() : joiner(threads)
  {
    auto threadCount = std::max(std::thread::hardware_concurrency(), 2u);
    
    init(threadCount);
  }

  ~ThreadPool()
  {
    try {
      for (unsigned i = 0; i < threads.size(); ++i)
        submit([this]{done = true;});
    } catch (...)
    {
      done = true;
    }
  }

  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> submit(
    FunctionType f)
  {
    typedef typename std::result_of<FunctionType()>::type result_type;
    
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());
    workQueue.push(std::move(task));
    
    return res;
  }
  
private:
  void init(unsigned int threadCount)
  {
    done = false;
    
    threads.reserve(threadCount);
    try
    {
      for (unsigned i = 0; i < threadCount; ++i)
      {
        threads.push_back(std::thread(&ThreadPool::workerThread, this));
      }
    } catch (...)
    {
      done = true;
      throw;
    }
  }

  void workerThread()
  {
    FunctionWrapper task;
    while (!done)
    {
      workQueue.waitAndPop(task);
      task();
    }
  }

  std::atomic_bool done;
  TSQueue<FunctionWrapper> workQueue;
  std::vector<std::thread> threads;
  JoinThreads joiner;
};

} // util
} // cc

#endif /* THREADPOOL_H_ */
