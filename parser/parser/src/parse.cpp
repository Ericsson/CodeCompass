#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

#include <sys/stat.h>
#include <sys/types.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "config.h"

#include "model/statistics.h"
#include "model/statistics-odb.hxx"

#include <model/workspace.h>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <projectparser/xmlprojectparser.h>
#include <projectparser/jsonprojectparser.h>
#include <cxxparser/cxxparser.h>
#include <javaparser/javaparser.h>
#include <gitparser/gitparser.h>
#include <searchparser/searchparser.h>
#include <metricsparser/metricsparser.h>
#include <parser/commondefs.h>

#ifdef ENABLE_PYTHON_PARSER
#include <pythonparser/pythonparser.h>
#endif

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include <util/environment.h>
#include <util/filesystem.h>

namespace po = boost::program_options;
using namespace cc::parser;
using namespace cc::util;

namespace
{

typedef std::unordered_set<unsigned long> BuildActionIds;

BuildActionIds readBuildActionIds(const std::string& filePath_)
{
  BuildActionIds ids;

  std::ifstream file(filePath_.c_str());
  if (file.is_open())
  {
    BuildActionIds::value_type id;

    while (file >> id)
    {
      ids.emplace(id);
    }
  }

  return ids;
}

#if defined(DATABASE_SQLITE)

const std::string ConnectionPrefix   = "sqlite";
const std::string ConfiguredDatabase = "SQLite";

#elif defined(DATABASE_PGSQL)

const std::string ConnectionPrefix   = "pgsql";
const std::string ConfiguredDatabase = "PostgreSQL";

#endif

#define CC_MAX_PARSER_THREADS \
  (std::min(std::max(std::thread::hardware_concurrency(), 2u), 4u))


}

int main(int argc, char* argv[])
{
  std::string projectName;
  std::string xmlFile;
  std::string database;
  std::string projectDataDir;
  std::string loglevel;
  std::string skippableBuildActionsFile;
  std::string debugActionsFile;
  unsigned int maxParserThreads;
  std::string pythonMemLimit;
  std::string pythonDepsPath;
  std::string rootsStr;

  std::string username;
  const char* usr=getenv("USER");
  if (usr==NULL)
    username="unkown";
  else
    username=usr;

  po::options_description desc("Allowed options");
  desc.add_options()
    ( "help,h",
        "prints this help message")
    ("project,p",
        po::value<std::string>(&xmlFile),
        "absolute path or the xml or json description file of the project")
    ("database,d",
        po::value<std::string>(&database),
        "database connection string. For example: sqlite:database=sample.sqlite or pgsql:database=sample")
    ("projectDataDir,a",
        po::value<std::string>(&projectDataDir),
        "data directory of the parsed project. Search database and version control informations will be placed here")
    ("name,n",
        po::value<std::string>(&projectName),
        "name of the project")
    ("loglevel,l",
        po::value<std::string>(&loglevel)->default_value("error"),
        "log level of the parser. Possible values: debug, info, warning, error, critical")
    ("skippableBuildActions,k",
        po::value<std::string>(&skippableBuildActionsFile),
        "path to a file that contains the skippable build action ids")
    ("debugActions",
        po::value<std::string>(&debugActionsFile),
        "path to a file that contains the actions we want to debug (only these actions will be parsed)")
    ("skip-traverse",
        "Skip all traversal")
    ("skip-parser",
        "Skip all language parser")
    ("skip-pythonparser",
        "Skip Python parser")
    ("skip-version-control",
        "Skip version control integration")
    ("max-parser-threads",
        po::value<unsigned int>(&maxParserThreads)->default_value(CC_MAX_PARSER_THREADS),
        "maximum number of threads for parsers")
    ("python-mem-limit",
        po::value<std::string>(&pythonMemLimit)->default_value("1532"),
        "memory limit to JVM, please add it in megabytes. default is 1532")
    ("python-deps",
        po::value<std::string>(&pythonDepsPath),
        "path of project dependencies of the project we wish to parse, concatenated with colons (:)")
    ("no-search-ast-info",
        "skip adding extra search information based on AST")
    ("roots",
        po::value<std::string>(&rootsStr),
        "Labeled source roots");

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (po::error& e) {
    std::cerr
      << desc     << std::endl << std::endl
      << e.what() << std::endl;

    return 1;
  }

  if (argc < 2 || vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 1;
  }

  if (!vm.count("project") || !vm.count("database"))
  {
    std::cerr << "Project and database must be given!" << std::endl;
    return 1;
  }

  if (!vm.count("projectDataDir"))
  {
    std::cerr << "Data directory (-a parameter) is required!" << std::endl;
    return 1;
  }

  if (database.find(ConnectionPrefix) == std::string::npos)
  {
    std::cerr
      << "This binary of CodeCompass can only work with "
      << ConfiguredDatabase << std::endl;

    return 1;
  }

  // Process labelled paths
  cc::parser::ProjectParser::ProjectRoots projectRoots;
  if (!rootsStr.empty())
  {
    std::vector<std::string> labelPairs;
    boost::split(labelPairs, rootsStr, boost::is_any_of(":"));

    for (const std::string& labelPair : labelPairs)
    {
      size_t pos = labelPair.find('=');
      if (pos == std::string::npos)
      {
        std::cerr
          << "Label-path pairs must be separated by '=' sign: "
          << labelPair << std::endl;
        return 1;
      }
      else
      {
        std::string label = labelPair.substr(0, pos);
        std::string path  = labelPair.substr(pos + 1);
        if (path.back() != '/') path.push_back('/');
        projectRoots[label] = path;
      }
    }
  }

  // Init logging
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
    new StandardErrorLogStrategy()));
  StreamLog::setLogLevel(getLogLevelFromString(loglevel));

  // Init global / environment based variables
  cc::util::Environment::init();

  std::cout << "proj: " << xmlFile << std::endl;
  std::cout << "db: " << database << std::endl;

  // Open database
  std::shared_ptr<cc::model::Workspace> workspace =
    cc::model::Workspace::getCreateWorkspace(database);

  // BEGIN SETTING PARSER OPTIONS
    ParseProps pp = ProjectParser::createParseProperties(workspace);

    pp.options["python-mem-limit"] = pythonMemLimit;
    pp.options["python-deps"] = pythonDepsPath;

    pp.options["name"] = projectName;
    pp.options["database"] = database;

    if (vm.count("skip-traverse") || vm.count("no-search-ast-info"))
    {
      pp.options["no-search-ast-info"] = "";
    }

    {
      // Make projectDataDir absolute
      projectDataDir = cc::util::pathFromCurrentDirectory(projectDataDir);
      ::mkdir(projectDataDir.c_str(), 0775);

      pp.options["projectDataDir"] = projectDataDir;
      pp.options["searchIndexDir"] = projectDataDir + "/search";
      pp.options["skip-docparser"] = vm.count("skip-docparser") ? "true" : "false";
    }
  // END SETTING PARSER OPTIONS

  std::shared_ptr<GeneralProjectParser> parserGeneral;
  std::shared_ptr<XmlProjectParser>     parserXml;
  std::shared_ptr<JSonProjectParser>    parserJSon;
  FileParser&                           fileParser = FileParser::instance();
  Parser& parser = Parser::getParser(maxParserThreads);

  // Language parsers
  if (!vm.count("skip-parser"))
  {
    std::shared_ptr<CXXParser> parserCXX(new CXXParser(workspace));
    fileParser.registerParser(parserCXX);

    std::shared_ptr<JavaParser> parserJava(new JavaParser(workspace));
    fileParser.registerParser(parserJava);
  }

  // This is the shared source manager
  SourceManager sourceManager(workspace, pp);

  //  Project parsers
  parserGeneral.reset(new GeneralProjectParser(workspace, pp, sourceManager));
  parser.registerProjectParser(parserGeneral);

  parserXml.reset(new XmlProjectParser(workspace, pp, sourceManager));
  parserXml->skipActions(readBuildActionIds(skippableBuildActionsFile));
  parserXml->debugActions(readBuildActionIds(debugActionsFile));
  parser.registerProjectParser(parserXml);

  parserJSon.reset(new JSonProjectParser(workspace, pp, sourceManager,
    projectRoots));
  parserJSon->skipActions(readBuildActionIds(skippableBuildActionsFile));
  parserJSon->debugActions(readBuildActionIds(debugActionsFile));
  parser.registerProjectParser(parserJSon);

  // Traversal parsers
  if (!vm.count("skip-traverse"))
  {
    std::shared_ptr<SearchParser> parserSearch(new SearchParser(workspace));
    ProjectParser::registerTraversal(parserSearch);

    if (!vm.count("skip-version-control"))
    {
      std::shared_ptr<GitParser> parserGit(new GitParser(workspace));
      ProjectParser::registerTraversal(parserGit);
    }

    std::shared_ptr<MetricsParser> metricsParser(new MetricsParser(workspace));
    ProjectParser::registerTraversal(metricsParser);

#ifdef ENABLE_PYTHON_PARSER
    if (!vm.count("skip-pythonparser"))
    {
      std::shared_ptr<PythonParser> parserPython(new PythonParser(workspace));
      ProjectParser::registerTraversal(parserPython);
    }
#endif
  }

  parser.parseProject(xmlFile);
  
  auto writeTableHeader = [](const std::string& title, const std::string& c1, const std::string& c2){
    std::cout << std::endl;
    std::cout << title << ":" << std::endl;
    std::cout << std::string(42, '-') << std::endl;
    std::cout << std::setw(30) << std::left << "| " + c1;
    std::cout << std::setw(10) << std::left << "| " + c2 << " |" << std::endl;
    std::cout << std::string(42, '-') << std::endl;
  };
  
  auto writeTableRow = [](const std::string& r1, const int& r2){
    std::cout << std::setw(30) << std::left << "| " + r1;
    std::cout <<  "| ";
    std::cout << std::setw(8) << std::left << r2 << " |" << std::endl;
  };
  
  auto writeTableFooter = [](){
    std::cout << std::string(42, '-') << std::endl << std::endl;
  };
  
  typedef odb::query<cc::model::Statistics> SQ;
  
  std::string beforeGroup;  
  std::map<std::string, int> summary;
  std::map<std::string, std::vector<std::pair<std::string, int>>> tables;
  
  std::cout << "Statistics:" << std::endl; 
  cc::util::OdbTransaction trans(*workspace->getDb());
  trans ([&](){
  for (auto& stat : workspace->getDb()->query<cc::model::Statistics>("ORDER BY" + SQ::group))
    {        
      tables[stat.group].push_back(std::make_pair(stat.key, stat.value));
      summary[stat.key] += stat.value;
    }
  for (const auto& table : tables)
  {
    writeTableHeader(table.first, "Statistic", "Value");        
    for(const auto& row : table.second)
    {
      writeTableRow(row.first, row.second);
    }
    writeTableFooter();
  }
  writeTableHeader("Summary","Statistic", "Value");
  for(const auto& s : summary)
  {
    writeTableRow(s.first, s.second);
  }
  writeTableFooter();
  });
}
