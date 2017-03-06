#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/log/expressions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/attributes.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <util/dbutil.h>
#include <util/logutil.h>

#include <parser/parsercontext.h>
#include <parser/pluginhandler.h>
#include <parser/sourcemanager.h>

namespace po = boost::program_options;
namespace trivial = boost::log::trivial;

po::options_description commandLineArguments()
{
  po::options_description desc("CodeCompass options");

  desc.add_options()
    ("help,h",
      "Prints this help message.")
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
    ("data-dir", po::value<std::string>()->required(),
      "The parsers can use a directory to store information. In this directory "
      "the parsers should create an own subdirectory where they can store "
      "their arbitrary temporary files or local databases.")
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

int main(int argc, char* argv[])
{
  cc::util::initLogger();

  //--- Process command line arguments ---//

  po::options_description desc = commandLineArguments();

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
    .options(desc).allow_unregistered().run(), vm);

  std::string binDir = boost::filesystem::canonical(
    boost::filesystem::path(argv[0]).parent_path()).string();
  std::string pluginDir = binDir + "/../lib/parserplugin";

  //--- Write out labels into a file to the data directory. ---//

  if (vm.count("label"))
  {
    std::ofstream fLabels(vm["data-dir"].as<std::string>() + "/labels.txt");
    for (const std::string& label : vm["label"].as<std::vector<std::string>>())
      fLabels << label << std::endl;
  }

  //--- Skip parser list ---//

  std::vector<std::string> skipParserList;
  if (vm.count("skip"))
    skipParserList = vm["skip"].as<std::vector<std::string>>();

  //--- Load parsers ---//

  cc::parser::PluginHandler pHandler(pluginDir);
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

  //--- Start parsers ---//

  std::shared_ptr<odb::database> db = cc::util::createDatabase(
    vm["database"].as<std::string>());
  if (!db)
  {
    LOG(error)
      << "Couldn't connect to database. Check the connection string.";

    return 1;
  }

  cc::parser::SourceManager srcMgr(db);
  cc::parser::ParserContext ctx(db, srcMgr, vm);
  pHandler.createPlugins(ctx);

  // TODO: Handle errors returned by parse().
  for (const std::string& parserName : pHandler.getTopologicalOrder())
  {
    LOG(info) << "[" << parserName << "] started!";
    pHandler.getParser(parserName)->parse();
  }

  // TODO: Print statistics.

  return 0;
}
