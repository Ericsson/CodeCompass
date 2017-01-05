#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <string>
#include <functional>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace po = boost::program_options;

struct LogCommand
{
  const static set<string> flags;
  const static set<string> pathFlags;
  const static set<char> delims;
  const static set<string> omitFlags;
  
  string compiler;
  vector<string> objects;
  vector<pair<string, string> > params;
};

const set<string> LogCommand::flags     = { "-I", "-L", "-o", "-include", "-D",
  "-MF", "-MQ", "-MT" };
const set<string> LogCommand::pathFlags = { "-I", "-L", "-o", "-include" };
const set<char>   LogCommand::delims    = { ' ', ':', '=' };
const set<string> LogCommand::omitFlags = { "-M", "-MM", "-MF", "-MG", "-MP",
  "-MQ", "-MT", "-MD", "-MMD" };


struct BuildAction
{
  string directory;
  string tool;
  string output;
  string time;
  string outputMd5;
  string sourceCount;
  vector<string> sources;
  vector<string> md5;
  LogCommand command;
};

struct PathParts
{
  string name;
  string root;
  string path;
};

/**
 * This function returns the file name of the path, i.e. the part after the
 * last '/' character. If the parameter doesn't contain '/' character, then
 * the parameter will be returned as it is.
 */
string getFilename(const string& path)
{
  size_t pos = path.rfind('/');
  return pos == string::npos ? path : path.substr(pos + 1);
}

/**
 * This function returns the extension of the file name given by a path,
 * i.e. the part after the last '.' character in the file name. If the file name
 * doesn't contain '.' character, then empty string will be returned.
 */
string getExtension(const string& path)
{
  string filename = getFilename(path);
  size_t pos = filename.rfind('.');
  return pos == string::npos ? "" : filename.substr(pos + 1);
}

/**
 * This function returns true if str contains prefix, otherwise returns false.
 */
bool isPrefix(const string& str, const string& prefix)
{
  return str.find(prefix) == 0;
}

/**
 * This function slices its parameter to words. Words are separated by one or
 * more spaces.
 */
vector<string> words(const string& line)
{
  vector<string> result;

  std::size_t begin = 0, end;
  while (begin < line.length())
  {
    begin = line.find_first_not_of(' ', begin);
    if (begin == std::string::npos)
      break;

    end = line.find_first_of(" \"", begin + 1);

    if (end != std::string::npos && line[end] == '"')
    {
      end = line.find('"', end + 1);
      if (end != std::string::npos)
      {
        ++end;
      }
    }

    result.push_back(line.substr(begin, end - begin));
    begin = end;
  }

  return result;
}



/**
 * This function returns true if the given parameter is a header file name,
 * i.e. it ends with .h, .hh, .hpp or .hxx.
 */
bool isHeaderFile(string filename)
{
  filename = getExtension(filename);
  
  return filename == "h"   || filename == "H"   ||
         filename == "hh"  || filename == "HH"  ||
         filename == "hpp" || filename == "HPP" ||
         filename == "hxx" || filename == "HXX";
}

/**
 * This function returns true if the given parameter is a source file name,
 * i.e. it ends with .c, .cc, .cpp, .cxx or .java.
 */
bool isSourceFile(string filename)
{
  filename = getExtension(filename);
  
  return filename == "c"   || filename == "C"   ||
         filename == "cc"  || filename == "CC"  ||
         filename == "cpp" || filename == "CPP" ||
         filename == "cxx" || filename == "CXX" ||
         filename == "java";
}

/**
 * This function returns true if the given parameter is an object file name,
 * i.e. it ends with .o.
 */
bool isObjectFile(string filename)
{
  // Sometimes object files are given with -Wl flag which is written without
  // space. Here we ignore these files, but this should be solved more
  // elegantly. (TODO)
  if (!filename.empty() && filename[0] == '-')
    return false;
  
  filename = getExtension(filename);
  
  return filename == "o"  || filename == "O" ||
         filename == "so" || filename == "SO";
}

/**
 * If LogCommand::flags contains a flag, which is prefix of the given
 * parameter, then the length of this flag will be returned, otherwise the
 * result is 0.
 */
int prefixParameterFlagLength(string s)
{
  for (string flag : LogCommand::flags)
    if (s.find(flag) == 0)
      return flag.length();
  
  return 0;
}

/**
 * This function parses a compiler invocation command (e.g. g++ -DXYZ main.cpp).
 * The resulting LogCommand object contains the compiler's name and the
 * parameters passed to it, as key-value pairs. If a given flag can be found in
 * LogCommand::flags vector, then the flag and its parameter will be divided to
 * key-value pair. The key and its value may be separated by one of the
 * delimiters in LogCommand::delims set. In this case the delimiter belongs to
 * the key part.
 */
LogCommand parseLogCommand(string command)
{
  LogCommand result;
  
  //--- Omitting compiler's name ---//
  size_t pos = command.find(' ');
  result.compiler = command.substr(0, pos);
  
  vector<string> params = words(command.substr(pos + 1));
  
  bool keyValueFound = false;
  string key, value;
  int length = 0;
  
  for (string param : params)
  {
    if (LogCommand::flags.find(param) != LogCommand::flags.end())
    {
      key = param;
      keyValueFound = false;
    }
    else if ((length = prefixParameterFlagLength(param)) > 0)
    {
      key = param.substr(0, length + (LogCommand::delims.find(param[length]) !=
                                      LogCommand::delims.end()));
      value = param.substr(key.length());
      keyValueFound = true;
    }
    else if (!key.empty())
    {
      key += ' ';
      value = param;
      keyValueFound = true;
    }
    else
    {
      key = param;
      keyValueFound = true;
    }
    
    if (keyValueFound)
    {
      if (LogCommand::omitFlags.find(key.substr(0, key.find(' '))) ==
          LogCommand::omitFlags.end())
      {
        if (isObjectFile(key))
          result.objects.push_back(key);
        
        result.params.push_back(make_pair(key, value));
      }

      key = value = "";
      keyValueFound = false;
    }
  }
  
  return result;
}

/**
 * We call a section from the log file a "log unit" which is divided by an empty
 * line. A log unit contains information about a compiler invocation action,
 * such as the compiler tool, compilation time, source files, their md5 hashes,
 * and so on.
 */
vector<BuildAction> readBuildActions(ifstream& file)
{
  vector<BuildAction> result;
  
  map<string, string BuildAction::*> buildActionMember;
  buildActionMember["directory:"]   = &BuildAction::directory;
  buildActionMember["tool:"]        = &BuildAction::tool;
  buildActionMember["output:"]      = &BuildAction::output;
  buildActionMember["time:"]        = &BuildAction::time;
  buildActionMember["outputMd5:"]   = &BuildAction::outputMd5;
  buildActionMember["sourceCount:"] = &BuildAction::sourceCount;
  
  unordered_set<size_t> commandHashes;
  hash<std::string> strHash;

  while (!file.eof())
  {
    BuildAction buildAction;
    
    bool needInsert = true;
    string line;
    getline(file, line);
    
    while (line != "") // read a unit
    {
      auto it = buildActionMember.find(line);
      if (it != buildActionMember.end())
      {
        string line2;
        getline(file, line2);
        boost::trim(line2);
        buildAction.*it->second = line2;
      }
      else if (line == "command:")
      {
        string line2;
        getline(file, line2);
        buildAction.command = parseLogCommand(line2);

        size_t paramHash = strHash(line2.substr(line2.find(' ') + 1));
        if (commandHashes.find(paramHash) != commandHashes.end())
          needInsert = false;
        else
          commandHashes.insert(paramHash);
      }
      else if (line == "sources:")
      {
        int count = atoi(buildAction.sourceCount.c_str());
        
        vector<string>& sources = buildAction.sources;
        vector<string>& md5     = buildAction.md5;
        
        for (int i = 0; i < count; ++i)
        {
          getline(file, line);
          sources.push_back(line);
          getline(file, line);
          md5.push_back(line);
        }
      }
      
      getline(file, line);
    }
    
    if (needInsert)
      result.push_back(buildAction);
  }
  
  result.pop_back(); // since the loop has an extra read because of eof()
  return result;
}

/**
 * This function returns all prefixes of a path in a directory hierarchy.
 * The prefixes of /one/two/three are: /, /one, /one/two. If "three" is also a
 * directory (i.e. the full path ends with slash: /one/two/three/), then
 * /one/two/three is also going to be a prefix of the path.
 */
vector<string> prefixes(const string& path)
{
  vector<string> result;
  
  for (size_t i = path.find('/'); i != string::npos; i = path.find('/', i + 1))
    result.push_back(path.substr(0, i + 1));
    
  return result;
}

/**
 * This function returns a vector of int-string pairs. These are all the
 * prefixes of paths used during the full compilation. The first part of the
 * pair is the frequency of occurrence of the prefix, in the second part of the
 * pair. The result is sorted by frequency of occurrence in reverse order.
 */
vector<pair<int, string> > prefixCandidates(const vector<BuildAction>& buildActions)
{
  map<string, int> prefixCounts;
  
  for (const BuildAction& buildAction : buildActions)
  {
    for (string prefix : prefixes(buildAction.output))
      ++prefixCounts[prefix];
    for (string prefix : prefixes(buildAction.command.compiler))
      ++prefixCounts[prefix];
    for (string source : buildAction.sources)
      for (string prefix : prefixes(source))
        ++prefixCounts[prefix];
  }
  
  vector<pair<int, string> > result;
  for (auto it = prefixCounts.cbegin(); it != prefixCounts.cend(); ++it)
    result.push_back(make_pair(it->second, it->first));
  
  sort(result.rbegin(), result.rend());
  
  return result;
}

/**
 * This function allows the user to name some of the paths with a label. The
 * 20 most frequent prefixes are offered. The resulting data structure maps the
 * paths to the labels. The resulting map always contains the root as path,
 * even if the user doesn't give it.
 */
map<string, string> userChoosePrefixes(const vector<pair<int, string> >& pc)
{
  const size_t candidateNum = 20;
  
  map<string, string> result;
  
  for (size_t i = 0; i < pc.size() && i < candidateNum; ++i)
    cout << i + 1 << ":\t" << pc[i].second << endl;

  for (size_t i = candidateNum; i < pc.size(); ++i)
  {
    const string& path = pc[i].second;
    if (isPrefix(path, "/vobs/") && count(path.begin(), path.end(), '/') == 3)
      cout << i + 1 << ":\t" << path << endl;
  }
  
  int num;
  cout << "Number of the source's root: "; cin >> num;
  result[pc[num - 1].second] = "source";
  
  do {
    cout << "Labeling further paths. Choose a number (1-" << candidateNum << ", 0: exit): ";
    cin >> num;
    
    if (num == 0)
      break;
    
    string label;
    cout << "Label of this path: ";
    cin >> label;
    
    result[pc[num - 1].second] = label;
  } while (true);
  
  result["/"] = "root";

  return result;
}

PathParts getPathParts(const map<string, string>& labels,
                       string fullName,
                       bool keepLast = false)
{
  PathParts result;
  
  result.name = keepLast ? "" : getFilename(fullName);
  string dir = fullName.substr(0, fullName.size() - result.name.size());
  
  auto it = labels.rbegin();
  for (; it != labels.rend(); ++it)
    if (isPrefix(dir, it->first))
      break;

  result.root = it == labels.rend() ? ""  : it->second;
  result.path = it == labels.rend() ? dir : dir.substr(it->first.length());
  
  return result;
}

/**
 * This function converts the log file to xml format.
 */
void convertLogToXml(bool logFileGiven,ifstream& logFile,
                     ofstream& xmlFile,
                     const po::variables_map& vm)
{
  //--- Project name ---//

  string projectName;
  
  if (vm["name"].as<string>().empty()) {
    cout << "Project name: ";
    getline(cin, projectName);
  } else
    projectName = vm["name"].as<string>();
  
  //--- Labels ---//
  
  vector<BuildAction> buildActions = !logFileGiven ? vector<BuildAction>() : readBuildActions(logFile);
  map<string, string> compilers;
  map<string, string> labels;

  string sourceFlag = vm["source"].as<string>();
  string labelsFlag = vm["labels"].as<string>();

  if (sourceFlag.empty() && labelsFlag.empty())
    // User action is needed only if none of "source" or "labels" is given.
    labels = userChoosePrefixes(prefixCandidates(buildActions));

  if (!sourceFlag.empty())
  {
    string srcDir = sourceFlag;
    if (srcDir.back() != '/') srcDir.push_back('/');
    labels[srcDir] = "source";
  }

  if (!labelsFlag.empty())
  {
    vector<string> labelPairs;
    boost::split(labelPairs, labelsFlag, boost::is_any_of(":"));

    for (const string& labelPair : labelPairs)
    {
      size_t pos = labelPair.find('=');
      if (pos == string::npos)
      {
        cerr << "Label-path pairs must be separated by '=' sign: " << labelPair << endl;
        continue;
      }
      else
      {
        string label = labelPair.substr(0, pos);
        string path  = labelPair.substr(pos + 1);
        if (path.back() != '/') path.push_back('/');
        labels[path] = label;
      }
    }
  }

  labels["/"] = "root";
  
  //--- Write output ---//

  int counter = 0;
  for (BuildAction buildAction : buildActions)
    compilers[buildAction.command.compiler] = "_tool" + to_string(++counter) + "_";
  
  auto writeSource = [&xmlFile](const PathParts& pathParts, const string& md5) {
    xmlFile << "        <source>"                                 << endl
            << "          <name>"  << pathParts.name << "</name>" << endl
            << "          <root>"  << pathParts.root << "</root>" << endl
            << "          <path>"  << pathParts.path << "</path>" << endl
            << "          <md5>"   << md5            << "</md5>"  << endl
            << "        </source>"                                << endl;
  };
  
  xmlFile << "<project>" << endl;
  
  xmlFile << "  <id></id>"                            << endl
          << "  <name>" << projectName << "</name>"   << endl
          << "  <branch></branch>"                    << endl
          << "  <comments>"                           << endl
          << "    <comment>comment1</comment>"        << endl
          << "    <comment>comment2</comment>"        << endl
          << "  </comments>"                          << endl
          << "  <codecompass></codecompass>"          << endl;
  
  xmlFile << "  <options>"                                                                                         << endl
          << "    <option key=\"name\">"                 << projectName                             << "</option>" << endl
          << "    <option key=\"searchIndexDir\">"       << vm["searchIndexDir"].as<string>()       << "</option>" << endl
          << "    <option key=\"searchFileExtensions\">" << vm["searchFileExtensions"].as<string>() << "</option>" << endl
          << "    <option key=\"projectDataDir\">"       << vm["projectDataDir"].as<string>()       << "</option>" << endl
          << "    <option key=\"model\">"                << vm["model"].as<string>()                << "</option>" << endl
          << "  </options>"                                                                                        << endl;
  
  xmlFile << "  <configurations default=\"def\">" << endl
          << "    <conf>"                         << endl
          << "      <name>def</name>"             << endl;
  
  for (auto it = labels.cbegin(); it != labels.cend(); ++it)
    xmlFile << "      <rootconf>"                          << endl
            << "        <root>" << it->second << "</root>" << endl
            << "        <path>" << it->first  << "</path>" << endl
            << "      </rootconf>"                         << endl;
  
  xmlFile << "    </conf>"         << endl
          << "  </configurations>" << endl;
  
  xmlFile << "  <tools>" << endl;
  
  for (auto it = compilers.cbegin(); it != compilers.cend(); ++it)
  {
    PathParts pathParts = getPathParts(labels, it->first, true);
    
    xmlFile << "    <tool>"                                  << endl
            << "      <name>" << it->second     << "</name>" << endl
            << "      <root>" << pathParts.path << "</root>" << endl
            << "      <path>" << pathParts.root << "</path>" << endl
            << "    </tool>"                                 << endl;
  }
  
  xmlFile << "  </tools>" << endl;
  
  xmlFile << "  <model>sqlite:model.sqlite</model>" << endl;
  
  xmlFile << "  <build>" << endl;
  
  int actionCounter = 0;
  for (const BuildAction& buildAction : buildActions)
  {
    bool isLinking = buildAction.sourceCount == "0";

    xmlFile << "    <action id=\"" << ++actionCounter << "\">" << endl;
    
    xmlFile << "      <tool>" << compilers[buildAction.command.compiler]
            << "</tool>" << endl;
    
    xmlFile << "      <type>" << (isLinking ? "link" : "compile")
            << "</type>" << endl;
    
    xmlFile << "      <time>" << buildAction.time << "</time>" << endl;
    
    xmlFile << "      <sources>" << endl;

    for (size_t i = 0; i < buildAction.sources.size(); ++i)
      writeSource(getPathParts(labels, buildAction.sources[i]), buildAction.md5[i]);
    for (const string& object : buildAction.command.objects)
      writeSource(getPathParts(labels, object), "");

    xmlFile << "      </sources>" << endl;
    
    PathParts pathParts = getPathParts(labels, buildAction.output);
    
    if (pathParts.name != "_noobj")
      xmlFile << "      <targets>"                                        << endl
              << "        <target>"                                       << endl
              << "          <name>" << pathParts.name    << "</name>"     << endl
              << "          <root>" << pathParts.root    << "</root>"     << endl
              << "          <path>" << pathParts.path    << "</path>"     << endl
              << "          <md5>"  << buildAction.outputMd5 << "</md5>"  << endl
              << "        </target>"                                      << endl
              << "      </targets>"                                       << endl;
    
    xmlFile << "      <options>" << endl;
    
    for (pair<string, string> param : buildAction.command.params)
    {
      if (isSourceFile(param.first) || isHeaderFile(param.first))
        continue;
      
      xmlFile << "        <option>" << endl;
      
      xmlFile << "          <key>" << param.first << "</key>" << endl;
      
      if (!param.second.empty())
      {
        if (LogCommand::pathFlags.find(param.first) ==
            LogCommand::pathFlags.end())
          xmlFile << "          <value><![CDATA[" << param.second << "]]></value>" << endl;
        else
        {
          PathParts pathParts = getPathParts(labels, param.second, true);
          xmlFile << "          <value>"                                 << endl
                  << "            <root>" << pathParts.root << "</root>" << endl
                  << "            <path>" << pathParts.path << "</path>" << endl
                  << "          </value>"                                << endl;
        }
      }
        
      xmlFile << "        </option>" << endl;
    }
    
    xmlFile << "      </options>" << endl;
            
    xmlFile << "    </action>" << endl;
  }
  
  xmlFile << "  </build>" << endl;
  
  xmlFile << "</project>" << endl;
}

int main(int argc, char* argv[])
{
  //--- Command line arguments ---//
  
  const char* usr = getenv("USER");
  string username = usr ? usr : "unknown";
  bool logFileGiven=true;
  
  string searchIndexDir = "/tmp/cc_" + username;
  string projectDataDir = "/tmp/cc_projectdata_" + username;
  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("input,i", po::value<string>(), "input build log file (optional)")
    ("output,o", po::value<string>(), "output project xml file (mandatory)")
    ("searchIndexDir,s", po::value<string>()->default_value(searchIndexDir),
      "full text search index database directory.")
    ("searchFileExtensions,e", po::value<string>()->default_value(""),
      "regular expression of files to make search index from")
    ("projectDataDir,p", po::value<string>()->default_value(projectDataDir),
      "data directory of the parsed project. version control informations will be placed here.")
    ("name,n", po::value<string>()->default_value(""), "name of the project")
    ("model,m", po::value<string>()->default_value("sqlite:database.sqlite"),
      "connection string")
    ("source", po::value<string>()->default_value(""),
      "deprecated. Use --labels instead. Specify source code path, equivalent to: --labels source=/path/to/source")
    ("labels,l", po::value<string>()->default_value(""),
      "Label source code directory paths in format: source_label1=/path/one:source_label2=/path/two\nAll labelled paths will be indexed.");
  
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  
  if (vm.count("help"))
  {
    cout << desc << endl;
    return 1;
  }
  
  if (!vm.count("input"))
  {
    logFileGiven=false;
  }
  
  if (!vm.count("output"))
  {
    cout << "No output xml file given" << endl;
    return 1;
  }
  
  string logFileName = !logFileGiven ? "":vm["input"].as<string>();
  string xmlFileName = vm["output"].as<string>();
  
  //--- Check if files can be opened ---//
  
  if (logFileName == xmlFileName)
  {
    cerr << "Input and output files can't be the same!" << endl;
    return 1;
  }
  
  ifstream logFile(logFileName.c_str());
  
  if (logFileGiven && !logFile.is_open())
  {
    cerr << "Error when opening log file: " << logFileName << endl;
    return 1;
  }
  
  ofstream xmlFile(xmlFileName.c_str());
  
  if (!xmlFile.is_open())
  {
    cerr << "Error when opening output file: " << xmlFileName << endl;
    return 1;
  }
  
  //--- Log to xml conversion ---//
  
  convertLogToXml(logFileGiven,logFile, xmlFile, vm);
  
  //--- Close files ---//
  
  logFile.close();
  xmlFile.close();
  
  return 0;
}
