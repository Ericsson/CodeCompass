#ifndef CC_UTIL_THREADPOOL_H
#define CC_UTIL_THREADPOOL_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace cc
{
namespace util
{

/**
 * @brief A simple thread pool iterating a list of jobs. This is a base class
 * to support overloading based on whether or not we want actual multithreading.
 *
 * @tparam JobData    Jobs are represented in a custom, user-defined structure.
 */
template <typename JobData>
class JobQueueThreadPool
{
  // This class implements the 'Curiously Recurring Template Pattern'
  // to provide an optimised overload for actually single-threaded execution.
public:
  virtual ~JobQueueThreadPool() {}

  /**
   * @brief Enqueue a new job to be executed by the thread pool.
   *
   * @warning Job execution might start immediately at enqueue's return!
   *
   * @param jobInfo  The job object to work on.
   */
  virtual void enqueue(JobData jobInfo) = 0;

  /**
   * @brief Notify all workers to exit after doing the remaining work
   * and wait for the threads to die.
   */
  virtual void wait() = 0;
};

/**
 * @brief Single-thread optimised, synchronous version of JobQueueThreadPool.
 *
 * This class does not create any workers, but rather executes every incoming
 * job synchronously before giving back control to the client code.
 *
 * @tparam JobData   Jobs are represented in a custom, user-defined structure.
 * @tparam Function  A user defined functor which the workers call to do the
 * actual work. This functor must accept a JobData as its argument.
 */
template <typename JobData, typename Function = std::function<void (JobData)>>
class SingleThreadJobQueue : public JobQueueThreadPool<JobData>
{
public:
  /**
   * Create a single-thread optimised "pool" object which executes jobs in
   * a synchronous way.
   *
   * @param func         The function to execute on the jobs.
   */
  SingleThreadJobQueue(Function func_) : _func(func_)
  {}

  /**
   * @brief Execute the thread pool's function on the given job.
   *
   * The single-threaded "pool" synchronously runs the job immediately.
   *
   * @param jobInfo  The job object to work on.
   */
  void enqueue(JobData jobInfo_)
  {
    _func(jobInfo_);
  }

  /**
   * @brief Has no effect in single-threaded operation as enqueue()
   * automatically runs the job function.
   */
  void wait()
  {}

private:
  /**
   * The function which is executed on incoming jobs.
   */
  Function _func;
};

/**
 * @brief A simple thread pool which iterates a set of jobs dynamically.
 *
 * This class creates N worker threads in the background which are waken up
 * as jobs are added to the queue. Each worker takes a single job and executes
 * it, and the threads return to sleep.
 *
 * @tparam JobData   Jobs are represented in a custom, user-defined structure.
 * @tparam Function  A user defined functor which the workers call to do the
 * actual work. This functor must accept a JobData as its argument.
 */
template <typename JobData, typename Function = std::function<void (JobData)>>
class PooledJobQueue : public JobQueueThreadPool<JobData>
{
public:
  /**
   * Create a new thread pool with the given number of threads and using the
   * given function as its work logic.
   *
   * @param threadCount  The number of worker threads to create.
   * @param func         The function to execute on the enqueued jobs.
   */
  PooledJobQueue(size_t threadCount_, Function func_)
    : _threadCount(threadCount_), _die(false)
  {
    for (size_t i = 0; i < threadCount_; ++i)
      _threads.emplace_back(std::thread(
        &PooledJobQueue<JobData, Function>::worker,
        this, func_));
  }

  ~PooledJobQueue()
  {
    if (!_die)
      wait();
  }

  /**
   * @brief Enqueue a new job to be executed by the thread pool.
   *
   * @warning Job execution might start immediately at enqueue's return!
   *
   * @param jobInfo  The job object to work on.
   */
  void enqueue(JobData jobInfo_)
  {
    {
      std::lock_guard<std::mutex> lock(_lock);
      _queue.push(jobInfo_);
    }

    _signal.notify_one();
  }

  /**
   * @brief Notify all workers to exit after doing the remaining work
   * and wait for the threads to die.
   */
  void wait()
  {
    _die = true;
    _signal.notify_all();

    for (std::thread& t : _threads)
    {
      // Keep nudging the threads so that the t.join() don't grind the
      // system to a halt.
      _signal.notify_all();
      if (t.joinable())
        t.join();
    }
  }

private:
  /**
   * @brief The worker method loops and waits for jobs to come and executes
   * function on them.
   */
  void worker(Function function_)
  {
    while (!(_die && _queue.empty()))  // The race condition here is known and
                                       // allowed deliberately.
    {
      // Lock on the mutex so that the queue access is safe.
      std::unique_lock<std::mutex> lock(_lock);
      if (_queue.empty())
      {
        // If the queue is empty, we have to wait for new work to be enqueued.
        // The thread is randomly woken up from time to time to ensure that
        // work is being done, even if an enqueue() call could not notify
        // any thread, because noone was waiting.
        _signal.wait_for(lock, std::chrono::seconds(1), [this]()
        {
          return !_queue.empty();
        });

        // Signal hit: work has been given to us, lock is reacquired.
        // Release the lock and notify other thread that we will be working.
        lock.unlock();
        _signal.notify_one();

        continue; // Next cycle will take the work if noone snatches it from us.
      }
      else
      {
        // The queue was not empty, we can do actual work.
        JobData job = _queue.front();
        _queue.pop();

        // After popping, we are out of the critical section.
        // Give the lock back and signal another thread.
        lock.unlock();
        _signal.notify_one();

        // Do work.
        function_(job);
      }
    }
  }

  /**
   * The number of worker threads created when the class is instantiated.
   */
  const size_t _threadCount;

  /**
   * std::mutex for accessing the _queue.
   */
  std::mutex _lock;

  /**
   * Condition variable to wake up worker threads.
   */
  std::condition_variable _signal;

  /**
   * _die controls whether or not executing workers must stop forever
   * waiting for new job to be queued and should stop after doing work.
   */
  std::atomic_bool _die;

  /**
   * The queue contains the JobData objects which define the jobs the pool
   * executes.
   */
  std::queue<JobData> _queue;

  /**
   * Contains the worker threads.
   */
  std::vector<std::thread> _threads;
};

/**
 * @brief Create an std::unique_ptr for a thread pool with the given number of
 * threads.
 *
 * @tparam JobData   Jobs are represented in a custom, user-defined structure.
 * @tparam Function  A user defined functor which the workers call to do the
 * actual work. This functor must accept a JobData as its argument.
 * @param threadCount  The number of threads the threadpool should use.
 * @param func         The function to execute on the enqueued jobs.
 * @param forceAsync   If threadCount is 1, ThreadPool can use a synchronous,
 * single-thread optimised version. However, there can be the case that the
 * client code specifically wants an async pool, which can be requested by
 * setting this variable to True. This variable has no effect if threadCount is
 * more than 1.
 * @return An std::unique_ptr containing a thread pool.
 */
template<typename JobData, typename Function>
std::unique_ptr<JobQueueThreadPool<JobData>> make_thread_pool(
    const size_t threadCount_,
    Function func_,
    bool forceAsync_ = false)
{
  if (threadCount_ == 1 && !forceAsync_)
    // Optimise for single-threaded execution!
    return std::make_unique<SingleThreadJobQueue<JobData, Function>>(func_);
  else
    return std::make_unique<PooledJobQueue<JobData, Function>>(threadCount_,
                                                               func_);
}

} // namespace util
} // namespace cc

#endif // CC_UTIL_THREADPOOL_H
