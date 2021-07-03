#ifndef CC_PARSER_COMPETENCEPARSER_H
#define CC_PARSER_COMPETENCEPARSER_H

#include <memory>

#include <boost/filesystem.hpp>
#include <odb/database.hxx>

#include "gitoperations.h"
#include "commitsampler.h"
#include "expertisecalculation.h"

#include <model/file.h>
#include <parser/abstractparser.h>
#include <parser/parsercontext.h>
#include <util/parserutil.h>
#include <util/threadpool.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

class CompetenceParser : public AbstractParser
{
public:
  CompetenceParser(ParserContext& ctx_);
  virtual ~CompetenceParser();
  virtual bool parse() override;

private:
  bool accept(const std::string& path_);

  std::shared_ptr<odb::database> _db;

  util::DirIterCallback getParserCallbackRepo(
    boost::filesystem::path& repoPath_);

  ExpertiseCalculation _expertise;
  CommitSampler _sampler;
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_COMPETENCEPARSER_H
