#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/log/expressions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/attributes.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <util/dbutil.h>
#include <util/filesystem.h>
#include <util/logutil.h>

#include <parser/parsercontext.h>
#include <parser/pluginhandler.h>
#include <parser/sourcemanager.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace trivial = boost::log::trivial;

po::options_description commandLineArguments()
{
  po::options_description desc("CodeCompass options");

  desc.add_options()
    ("help,h",
      "Prints this help message.")
    ("workspace,w", po::value<std::string>()->required(),
      "Path to a workspace directory. Project log files and plugin specific "
      "databases go under this directory.")
    ("name,n", po::value<std::string>()->required(),
      "Project name.")
    ("description", po::value<std::string>(),
      "Short description of the project.")
    ("force,f",
      "If the project already exists with the given name in the workspace, "
      "then by default CodeCompass skips all parsing actions. Force parsing "
      "removes all previous results from the workspace and from the database "
      "for the project.")
    ("list,l",
      "List available plugins. Plugins come from shared objects stored in the "
      "lib/parserplugin directory.")
    ("input,i", po::value<std::vector<std::string>>(),
      "The input of the parsers can be a compilation database (see: "
      "http://clang.llvm.org/docs/JSONCompilationDatabase.html) or a path of a "
      "directory under which the files will be consumed. Here you can list the "
      "compilation databases and the paths: -i /path/one -i /path/two -i "
      "compilation_database.json")
    ("database,d", po::value<std::string>()->required(),
      // TODO: Provide a full connection string example.
      "The parsers can use a relational database to store information. This "
      "database can be given by a connection string. Keep in mind that the "
      "database type depends on the CodeCompass executable. CodeCompass can be "
      "build to support PostgreSQL or SQLite. Connection string has the "
      "following format: pgsql:database=name;user=user_name.")
    ("label", po::value<std::vector<std::string>>(),
      "The submodules of a large project can be labeled so it can be easier "
      "later to locate them. With this flag you can provide a label list in "
      "the following format: label1=/path/to/submodule1.")
    ("loglevel",
      po::value<trivial::severity_level>()->default_value(trivial::info),
      "Logging legel of the parser. Possible values are: debug, info, warning, "
      "error, critical.")
    ("jobs,j", po::value<int>()->default_value(4),
      "Number of threads the parsers can use.")
    ("skip,s", po::value<std::vector<std::string>>(),
      "This is a list of parsers which will be omitted during the parsing "
      "process. The possible values are the plugin names which can be listed "
      "by --list flag.");

  return desc;
}

/**
 * This function checks the existence of the workspace and project directory
 * based on the given command line arguments.
 * @return The path of the project directory.
 */
bool checkProjectDir(const po::variables_map& vm_)
{
  const std::string projDir
    = vm_["workspace"].as<std::string>() + '/'
      + vm_["name"].as<std::string>();

  return fs::exists(projDir) && fs::is_directory(projDir);
}

/**
 * This function prepares the workspace and project directory based on the given
 * command line arguments. If the project directory can't be created because of
 * permission issues then empty string returns which indicates the problem.
 * @return The path of the project directory.
 */
std::string prepareProjectDir(const po::variables_map& vm_)
{
  const std::string projDir
    = vm_["workspace"].as<std::string>() + '/'
    + vm_["name"].as<std::string>();

  boost::system::error_code ec;

  bool isNewProject = fs::create_directories(projDir, ec);

  if (ec)
  {
    LOG(error) << "Permission denied to create " + projDir;
    return std::string();
  }

  if (isNewProject)
    return projDir;

  if (vm_.count("force"))
  {
    fs::remove_all(projDir, ec);

    if (ec)
    {
      LOG(error) << "Permission denied to remove " + projDir;
      return std::string();
    }

    fs::create_directory(projDir, ec);

    if (ec)
    {
      LOG(error) << "Permission denied to create " + projDir;
      return std::string();
    }
  }

  return projDir;
}

int main(int argc, char* argv[])
{
  std::string compassRoot = cc::util::binaryPathToInstallDir(argv[0]);

  const std::string PARSER_PLUGIN_DIR = compassRoot + "/lib/parserplugin";
  const std::string SQL_DIR = compassRoot + "/share/codecompass/sql";

  cc::util::initLogger();

  //--- Process command line arguments ---//

  po::options_description desc = commandLineArguments();

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
    .options(desc).allow_unregistered().run(), vm);

  //--- Skip parser list ---//

  std::vector<std::string> skipParserList;
  if (vm.count("skip"))
    skipParserList = vm["skip"].as<std::vector<std::string>>();

  //--- Load parsers ---//

  cc::parser::PluginHandler pHandler(PARSER_PLUGIN_DIR);
  pHandler.loadPlugins(skipParserList);

  //--- Add arguments of parsers ---//

  po::options_description pluginOptions = pHandler.getOptions();
  desc.add(pluginOptions);

  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (argc < 2 || vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("loglevel"))
  {
    trivial::severity_level loglevel
      = vm["loglevel"].as<trivial::severity_level>();
    boost::shared_ptr<boost::log::core> logger = boost::log::core::get();
    logger->set_filter(boost::log::expressions::attr<
      trivial::severity_level>("Severity") >= loglevel);
    logger->add_global_attribute("Severity",
      boost::log::attributes::mutable_constant<trivial::severity_level>(
        loglevel));
  }

  if (vm.count("list"))
  {
    std::cout << "Available plugins:" << std::endl;

    for (const std::string& pluginName : pHandler.getPluginNames())
      std::cout << " - " << pluginName << std::endl;

    return 0;
  }

  try
  {
    po::notify(vm);
  }
  catch (const po::error& e)
  {
    LOG(error) << "Error in command line arguments: " << e.what();
    return 1;
  }

  //--- Check database and project directory existsence ---//

  bool isNewDb = cc::util::connectDatabase(
    vm["database"].as<std::string>(), false) ? false : true;
  bool isNewProject = !checkProjectDir(vm);

  if (isNewProject && !isNewDb || !isNewProject && isNewDb)
  {
    LOG(error)
      << "Database and working directory existence are inconsistent. Use -f for reparsing!";
    return 1;
  }

  //--- Prepare workspace and project directory ---//

  std::string projDir = prepareProjectDir(vm);
  if (projDir.empty())
    return 1;

  //--- Create and init database ---//

  std::shared_ptr<odb::database> db = cc::util::connectDatabase(
      vm["database"].as<std::string>(), true);

  if (!db)
  {
    LOG(error)
      << "Couldn't connect to database. Check the connection string.";
    return 1;
  }

  if (vm.count("force"))
    cc::util::removeTables(db, SQL_DIR);

  if (vm.count("force") || isNewDb)
    cc::util::createTables(db, SQL_DIR);

  //--- Start parsers ---//

  cc::parser::SourceManager srcMgr(db);
  cc::parser::ParserContext ctx(db, srcMgr, compassRoot, vm);
  pHandler.createPlugins(ctx);

  // TODO: Handle errors returned by parse().
  for (const std::string& parserName : pHandler.getTopologicalOrder())
  {
    LOG(info) << "[" << parserName << "] started!";
    pHandler.getParser(parserName)->parse();
  }

  //--- Add indexes to the database ---//

  if(vm.count("force") || isNewDb)
    cc::util::createIndexes(db, SQL_DIR);

  //--- Create project config file ---//

  boost::property_tree::ptree pt;

  if (vm.count("label"))
  {
    boost::property_tree::ptree labels;

    for (const std::string& label : vm["label"].as<std::vector<std::string>>())
    {
      std::size_t pos = label.find('=');

      if (pos == std::string::npos)
        LOG(warning)
          << "Label doesn't contain '=' for separating label and the path: "
          << label;
      else
        labels.put(label.substr(0, pos), label.substr(pos + 1));
    }

    pt.add_child("labels", labels);
  }

  std::string database
    = cc::util::connStrComponent(vm["database"].as<std::string>(), "database");

  pt.put(
    "database",
    database.empty() ? vm["name"].as<std::string>() : database);

  if (vm.count("description"))
    pt.put("description", vm["description"].as<std::string>());

  boost::property_tree::write_json(projDir + "/project_info.json", pt);

  // TODO: Print statistics.

  return 0;
}
