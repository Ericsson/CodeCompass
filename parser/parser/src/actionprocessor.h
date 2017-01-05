#ifndef CC_PARSER_PARSER_ACTION_PROCESSOR_H
#define CC_PARSER_PARSER_ACTION_PROCESSOR_H

#include <parser/commondefs.h>
#include <parser/projectparser.h>
#include <model/buildaction.h>

#include <atomic>
#include <thread>
#include <future>
#include <vector>
#include <list>

namespace cc
{
namespace parser
{

/**
 * This class is responsible for getting parse statuses from the futures
 * returned by parsers.
 */
class ActionProcessor
{
public:
  /**
   * Alias for a AsyncAction container.
   */
  using Actions = std::list<AsyncBuildAction>;

public:
  /**
   * Constructs an action processor. It also starts the worker thread with a
   * start() method call.
   *
   * @param db_ an open odb database.
   */
  ActionProcessor(odb::database& db_);

  /**
   * It calls stopAndWait() method.
   */
  ~ActionProcessor();

  /**
   * Starts the worker thread. First it calls stopAndWait() method so if the
   * worker thread already started, it will restarted.
   */
  void start();

  /**
   * Sets _cont to false, then waits for the worker thread to finish.
   */
  void stopAndWait();

  /**
   * Queues an async action for processing in the worker thread. This method
   * will return immediately.
   *
   * @param act_ an async actions.
   */
  void queue(AsyncBuildAction act_);

  /**
   * Processes the given action in the current thread.
   *
   * @param act_ an async actions.
   */
  void process(AsyncBuildAction& act_);

  /**
   * Sets the total number of actions. This is only for statistics.
   *
   * @param totalActions_ the number of actions.
   */
  void setTotalActionCount(std::size_t totalActions_);

private:
  /**
   * Processes the given action in the current thread. If waitComplete_ is true
   * it will wait for all result, otherwise it returns after a fixed amount of
   * time.
   *
   * @param act_ the action to process.
   * @param waitComplete_ if it true then it waits for all results.
   * @return true if we got all result, false on timeout.
   */
  bool processAction(AsyncBuildAction& act_, bool waitComplete_ = false);

  /**
   * Processes a given result. It will call the get() method on the result
   * future and also handles the exceptions. It writes out some diagnostic
   * messages to the log.
   *
   * @param act_ the action for the result.
   * @param res_ a result to process.
   */
  void processResult(const AsyncBuildAction& act_, AsyncBuildResult& res_);

  /**
   * Method of worker thread.
   */
  void processorThread();

private:
  /**
   * Mutex for protecting _newActions container.
   */
  std::mutex _newActionsMutex;
  /**
   * Action queue.
   */
  Actions _newActions;
  /**
   * If it set to false, the worker thread will exit.
   */
  std::atomic<bool> _cont;
  /**
   * The number of total actions set by setTotalActionCount(std::size_t).
   */
  std::atomic<std::size_t> _actionTotal;
  /**
   * The number of already finished actions.
   */
  std::atomic<std::size_t> _actionFinished;
  /**
   * Thread object of the worker thread.
   */
  std::thread _processorThread;
  /**
   * A database object.
   */
  odb::database& _db;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_PARSER_ACTION_PROCESSOR_H
