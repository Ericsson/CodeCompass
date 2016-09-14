/*
 * tsqueue.h
 *
 *  Created on: Aug 7, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_TSQUEUE_H_
#define UTIL_TSQUEUE_H_

#include <thread>
#include <memory>
#include <condition_variable>

namespace cc 
{
namespace util 
{

/**
 * Thread-safe implementation of a queue
 * based on Anthony Williams: C++ concurrency in action
 * 6.2.3.: A thread-safe queue using fine-grained locking
 */
template<typename T>
class TSQueue
{
public:
  TSQueue()
    : head(new node), tail(head.get())
  {
  }

  TSQueue(const TSQueue& other) = delete;
  TSQueue& operator=(const TSQueue& other) = delete;

  std::shared_ptr<T> tryPop()
  {
    std::unique_ptr<node> oldHead = tryPopHead();
    return oldHead ? oldHead->data : std::shared_ptr<T>();
  }

  bool tryPop(T& value)
  {
    std::unique_ptr<node> const old_head = tryPopHead(value);
    return old_head;
  }

  std::shared_ptr<T> waitAndPop()
  {
    std::unique_ptr<node> const oldHead = waitPopHead();
    return oldHead->data;
  }

  void waitAndPop(T& value)
  {
    std::unique_ptr<node> const oldHead = waitPopHead(value);
  }

  void push(T newValue)
  {
    std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));
    std::unique_ptr<node> p(new node);
    {
      std::lock_guard<std::mutex> tailLock(tailMutex);
      tail->data = newData;
      node* const newTail = p.get();
      tail->next = std::move(p);
      tail = newTail;
    }
    dataCond.notify_one();
  }

  bool empty()
  {
    std::lock_guard<std::mutex> headLock(headMutex);
    return (head.get() == getTail());
  }

private:
  struct node
  {
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
  };

  std::mutex headMutex;
  std::unique_ptr<node> head;
  std::mutex tailMutex;
  node* tail;
  std::condition_variable dataCond;

  node* getTail()
  {
    std::lock_guard<std::mutex> tailLock(tailMutex);
    return tail;
  }

  std::unique_ptr<node> popHead()
  {
    std::unique_ptr<node> oldHead = std::move(head);
    head = std::move(oldHead->next);
    return oldHead;
  }

  std::unique_lock<std::mutex> waitForData()
  {
    std::unique_lock<std::mutex> headLock(headMutex);

    while (head.get() == getTail())
      dataCond.wait(headLock);

    return std::move(headLock);
  }

  std::unique_ptr<node> waitPopHead()
  {
    std::unique_lock<std::mutex> headLock(waitForData());
    return popHead();
  }

  std::unique_ptr<node> waitPopHead(T& value)
  {
    std::unique_lock<std::mutex> head_lock(waitForData());
    value = std::move(*head->data);
    return popHead();
  }

  std::unique_ptr<node> tryPopHead()
  {
    std::lock_guard<std::mutex> headLock(headMutex);
    if (head.get() == getTail())
    {
      return std::unique_ptr<node>();
    }
    return popHead();
  }

  std::unique_ptr<node> tryPopHead(T& value)
  {
    std::lock_guard<std::mutex> headLock(headMutex);
    if (head.get() == getTail())
    {
      return std::unique_ptr<node>();
    }
    value = std::move(*head->data);
    return popHead();
  }
};

} // util
} // cc


#endif /* TSQUEUE_H_ */
