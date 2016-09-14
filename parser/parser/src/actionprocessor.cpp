#include "actionprocessor.h"

#include <model/buildaction-odb.hxx>

#include <util/streamlog.h>
#include <util/util.h>
#include <util/odbtransaction.h>
#include <sstream>

namespace cc
{
namespace parser
{

ActionProcessor::ActionProcessor(odb::database& db_) :
  _cont(false),
  _actionTotal(0),
  _actionFinished(0),
  _db(db_)
{
  start();
}

ActionProcessor::~ActionProcessor()
{
  stopAndWait();
}

void ActionProcessor::start()
{
  stopAndWait();

  _cont = true;
  _processorThread = std::thread(&ActionProcessor::processorThread, this);
}

void ActionProcessor::stopAndWait()
{
  _cont = false;
  if (_processorThread.joinable())
  {
    _processorThread.join();
  }
}

void ActionProcessor::queue(AsyncBuildAction act_)
{
  std::unique_lock<std::mutex> lock(_newActionsMutex);
  _newActions.emplace_back(std::move(act_));
}

void ActionProcessor::process(AsyncBuildAction& act_)
{
  processAction(act_, true);
}

void ActionProcessor::setTotalActionCount(std::size_t totalActions_)
{
  _actionTotal = totalActions_;
}

bool ActionProcessor::processAction(AsyncBuildAction& act_, bool waitComplete_)
{
  static const std::chrono::milliseconds futureWaitTime(0);

  auto resIter = act_.parseResults.begin();
  while (resIter != act_.parseResults.end())
  {
    // Process the results inside an action
    std::future_status futstat = std::future_status::ready;
    if (waitComplete_)
    {
      resIter->pres.wait();
    }
    else
    {
      futstat = resIter->pres.wait_for(futureWaitTime);
    }

    if (futstat == std::future_status::ready)
    {
      // We found a finished parser result
      processResult(act_, *resIter);

      // We can drop this result
      resIter = act_.parseResults.erase(resIter);
    }
    else
    {
      ++resIter;
    }
  } // end of parse result iteration

  if (act_.parseResults.empty())
  {
    util::OdbTransaction trans(_db);
    trans([&act_, this]()
    {
      act_.action->state = model::BuildAction::StParsed;
      _db.update(*act_.action);
    });

    ++_actionFinished;
    return true;
  }

  return false;
}

void ActionProcessor::processResult(
  const AsyncBuildAction& act_,
  AsyncBuildResult& res_)
{
  ParseResult pres;
  bool hadException = false;
  std::string exMessage = "unknown exception";

  try
  {
    pres = res_.pres.get();
  }
  catch (const std::exception& ex)
  {
    hadException = true;
    exMessage = ex.what();
  }
  catch (...)
  {
    hadException = true;
  }

  if (hadException)
  {
    SLog(util::CRITICAL)
      << "Exception while parsing " << res_.sourcePath << ": " << exMessage
      << std::endl << "  in action " << act_.action->id << "";
    pres = PARSE_FAIL;
  }

  std::stringstream logMessage;
  switch(pres)
  {
    case PARSE_SUCCESS:
      logMessage << "Successfully parsed";
      break;

    case PARSE_FAIL:
      logMessage << "Failed to parse";
      break;

    case PARSE_DEFERRED:
      logMessage << "Deferred";
      break;
  }

  logMessage
    << " " << res_.sourcePath
    << std::endl << "  in action " << act_.action->id;
  if (_actionTotal > 0)
  {
    logMessage
      << ", total: " << _actionFinished << " of " << _actionTotal
      << " action already finished";
  }

  SLog(util::STATUS) << logMessage.str();
}

void ActionProcessor::processorThread()
{
  const std::chrono::seconds checkReq(2);

  Actions workingSet;
  do
  {
    if (workingSet.empty())
    {
      std::this_thread::sleep_for(checkReq);
    }

    {
      // Get new elements (if any)
      std::unique_lock<std::mutex> lock(_newActionsMutex);
      workingSet.splice(workingSet.end(), _newActions);
    }

    auto actIter = workingSet.begin();
    while (actIter != workingSet.end())
    {
      // Process async actions
      if (processAction(*actIter))
      {
        // All parser action is finished to this action so we can delete it
        // after setting its state.
        actIter = workingSet.erase(actIter);
      }
      else
      {
        ++actIter;
      }
    } // end of parse action iteration
  }
  while (_cont || !workingSet.empty());
}

} // namespace parser
} // namespace cc
