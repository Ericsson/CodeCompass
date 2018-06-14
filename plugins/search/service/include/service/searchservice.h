#ifndef CC_SERVICE_SEARCHSERVICE_H
#define CC_SERVICE_SEARCHSERVICE_H

#include <cstdio>
#include <memory>
#include <functional>
#include <mutex>

#include <boost/regex.hpp>
#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>
#include <webserver/servercontext.h>

#include <SearchService.h>

#include <service/serviceprocess.h>

namespace cc
{
namespace service
{
namespace search
{

class SearchServiceHandler : virtual public SearchServiceIf {
public:

  SearchServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void search(
    SearchResult& _return,
    const SearchParams& params_) override;

  void searchFile(
    FileSearchResult& _return,
    const SearchParams&     params_) override;

  void getSearchTypes(std::vector<SearchType> & _return) override;

  void pleaseStop() override;

  void suggest(SearchSuggestions& _return,
    const SearchSuggestionParams& params_) override;

private:
  /**
   * Validates a regluar expression. If the expression is invalid then a thrift
   * excepion is thrown.
   *
   * @param regexp_ a regluar expression to validate.
   */
  static void validateRegexp(const std::string& regexp_);

  std::shared_ptr<odb::database> _db;

  std::unique_ptr<ServiceProcess> _javaProcess;
  std::mutex _javaProcessMutex;
};

} // search
} // service
} // cc

#endif // CC_SERVICE_SEARCHSERVICE_H
