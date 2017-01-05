#include <iostream>

#include <projectparser/xmlparser.h>

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <util/streamlog.h>

#include <map>
#include <stdexcept>

using namespace cc::parser;
using namespace xercesc;

namespace
{

std::string xmlStrToStdStr(const XMLCh* xmlStr_)
{
  char* str = XMLString::transcode(xmlStr_);

  std::string ret = str;

  XMLString::release(&str);

  return ret;
}

int xmlStrToInt(const XMLCh* xmlStr_)
{
  char *str = XMLString::transcode(xmlStr_);

  int ret = atoi(str);

  XMLString::release(&str);

  return ret;
}

class XmlString
{
public:
  XmlString(const char* text)
  {
    data = XMLString::transcode(text);
  }

  ~XmlString()
  {
    XMLString::release(&data);
  }

  operator XMLCh*()
  {
    return data;
  }

private:
  XMLCh* data;
};

XmlString operator"" _xml(const char* str, std::size_t /* length */)
{
  return XmlString(str);
}

class CcSAXParser : public HandlerBase
{
private:
  class CcSAXParserState : public HandlerBase
  {
  public:
    CcSAXParserState(XMLParser::BuildActions& actions_,
      std::map<std::string, std::string>& projectOptions_)
      : actions(actions_), projectOptions(projectOptions_)
    {
    }

  protected:
    XMLParser::BuildActions& actions;
    std::map<std::string, std::string>& projectOptions;
  };

  class OptionsState : public CcSAXParserState
  {
  public:
    OptionsState(XMLParser::BuildActions& actions_,
      std::map<std::string, std::string>& projectOptions_)
      : CcSAXParserState(actions_, projectOptions_)
    {
    }

    void startElement(const XMLCh* const name, AttributeList& attributes)
    {
      if (!XMLString::equals(name, _tagOption))
        return;

      lastKey.clear();

      auto len = attributes.getLength();
      for (XMLSize_t i = 0; i < len; ++i)
      {
          auto attrName = attributes.getName(i);

          if (XMLString::equals(attrName, _key))
          {
            lastKey = xmlStrToStdStr(attributes.getValue(i));
            break;
          }
      }
    }

    void characters(const XMLCh* const chars, const XMLSize_t length)
    {
      if (lastKey.empty())
        return;

      projectOptions[lastKey] = xmlStrToStdStr(chars);
      lastKey.clear();
    }

  private:
    XmlString _tagOption = "option";
    XmlString _key = "key";

    std::string lastKey;
  };

  class ConfigurationsState : public CcSAXParserState
  {
  public:
    ConfigurationsState(XMLParser::BuildActions& actions_,
      std::map<std::string, std::string>& projectOptions_)
      : CcSAXParserState(actions_, projectOptions_)
    {
    }

    void startElement(const XMLCh* const name, AttributeList& attributes)
    {
      if (XMLString::equals(name, _configurations))
      {
        auto len = attributes.getLength();
        for (XMLSize_t i = 0; i < len; ++i)
        {
            auto attrName = attributes.getName(i);

            if (XMLString::equals(attrName, _default))
            {
              defaultConfigName = xmlStrToStdStr(attributes.getValue(i));
            }
        }
      }
      else if (XMLString::equals(name, _name))
      {
        lastElement = Name;
      }
      else if (XMLString::equals(name, _root))
      {
        lastElement = Root;
      }
      else if (XMLString::equals(name, _path))
      {
        lastElement = Path;
      }
      else
      {
        lastElement = Other;
      }
    }

    void characters(const XMLCh* const chars, const XMLSize_t length)
    {
      if (lastElement == Other)
        return;

      if (lastElement == Name)
      {
        XmlString defaultName(defaultConfigName.c_str());

        active = XMLString::equals(chars, defaultName);

        return;
      }

      if (!active)
        return;

      if (lastElement == Root)
      {
        config.first = xmlStrToStdStr(chars);
      }
      else if (lastElement == Path)
      {
        config.second = xmlStrToStdStr(chars);

        if (config.second.empty() || config.second.back() != '/')
        {
          config.second += '/';
        }

        SLog(cc::util::INFO) << "rootconf " << config.first << ':' << config.second;
        projectOptions["paths"] += (projectOptions["paths"].empty() ? "" : "|")
                                +   config.first + ':' + config.second;

        actions.roots.insert(config);
      }
    }

    void endElement(const XMLCh* const name)
    {
      if (XMLString::equals(name, _conf))
      {
        active = false;
      }
      lastElement = Other;
    }

  private:
    XmlString _configurations = "configurations";
    XmlString _conf = "conf";
    XmlString _name = "name";
    XmlString _root = "root";
    XmlString _path = "path";
    XmlString _default = "default";

    enum LastElement {Root, Path, Name, Other};

    std::string defaultConfigName;
    std::pair<std::string, std::string> config;

    LastElement lastElement = Other;
    bool active = false;
  };

  class BuildState : public CcSAXParserState
  {
  private:
    struct File
    {
      std::string name;
      std::string root;
      std::string path;

      void clear()
      {
        name.clear();
        root.clear();
        path.clear();
      }
    };

    struct BuildOption
    {
      std::string key;
      std::string value;
      std::string value_root;
      std::string value_path;

      void clear()
      {
        key.clear();
        value.clear();
        value_root.clear();
        value_path.clear();
      }
    };

    enum ElementType { FileInfo, Option, Type, Other };
    enum SubElementType {Key, Root, Name, Path, Value, Typer, Otherr };

  public:
    BuildState(XMLParser::BuildActions& actions_,
      std::map<std::string, std::string>& projectOptions_)
      : CcSAXParserState(actions_, projectOptions_)
    {
    }

    void startElement(const XMLCh* const name, AttributeList& attributes)
    {
      if (XMLString::equals(name, _action))
      {
        action.sources.clear();
        action.targets.clear();
        action.options.clear();
        action.label.clear();

        auto len = attributes.getLength();
        for (XMLSize_t i = 0; i < len; ++i)
        {
            auto attrName = attributes.getName(i);

            if (XMLString::equals(attrName, _id))
            {
              action.id = xmlStrToInt(attributes.getValue(i));
              break;
            }
        }
      }
      else if (XMLString::equals(name, _source))
      {
        element = FileInfo;
        file.clear();
      }
      else if (XMLString::equals(name, _target))
      {
        element = FileInfo;
        file.clear();
      }
      else if (XMLString::equals(name, _option))
      {
        element = Option;
        option.clear();
      }
      else if (XMLString::equals(name, _type))
      {
        element = Type;
        subElement = Typer;
      }
      else if (XMLString::equals(name, _value))
      {
        subElement = Value;
      }
      else if (XMLString::equals(name, _key))
      {
        subElement = Key;
      }
      else if (XMLString::equals(name, _root))
      {
        subElement = Root;
      }
      else if (XMLString::equals(name, _path))
      {
        subElement = Path;
      }
      else if (XMLString::equals(name, _name))
      {
        subElement = Name;
      }
    }

    void characters(const XMLCh* const chars, const XMLSize_t length)
    {
      if (subElement == Otherr)
        return;

      std::string value = xmlStrToStdStr(chars);
      if (FileInfo == element)
      {
        switch (subElement)
        {
          case Name:
            file.name = value;
            break;
          case Path:
            file.path = value;
            break;
          case Root:
            file.root = value;
            break;
          default:
            break;
        }
      }
      else if (Option == element)
      {
        switch (subElement)
        {
          case Key:
            option.key = value;
            break;
          case Root:
            option.value_root = value;
            break;
          case Path:
            option.value_path = value;
            break;
          case Value:
            option.value = value;
            break;
          default:
            break;
        }
      }
      else if (Type == element)
      {
        action.type = value;
      }
    }

    void endElement(const XMLCh* const name)
    {
      if (XMLString::equals(name, _source))
      {
        action.sources.push_back(fullPath(file));
      }
      else if (XMLString::equals(name, _target))
      {
        action.targets.push_back(fullPath(file));
      }
      else if (XMLString::equals(name, _option))
      {
        action.options.push_back(optStr(option));
      }
      else if (XMLString::equals(name, _action))
      {
        actions.actions.push_back(std::move(action));
      }

      subElement = Otherr;
    }

  private:
    std::string fullPath(const File& file)
    {
      auto root = actions.roots[file.root];
      auto path = file.path;

      if (!path.empty() && path.back() != '/')
        path += '/';

      return root + path + file.name;
    }

    std::string optStr(const BuildOption& option)
    {
      if (option.value_root.empty())
        return option.key + option.value;

      auto root = actions.roots[option.value_root];

      return option.key + root + option.value_path;
    }

    XmlString _action = "action";
    XmlString _source = "source";
    XmlString _target = "target";
    XmlString _option = "option";
    XmlString _value = "value";
    XmlString _root = "root";
    XmlString _path = "path";
    XmlString _name = "name";
    XmlString _key = "key";
    XmlString _id = "id";
    XmlString _type = "type";

    XMLParser::BuildAction action;
    File file;
    BuildOption option;
    ElementType element = Other;
    SubElementType subElement = Otherr;
  };


public:
  CcSAXParser(XMLParser::BuildActions& actions_,
    std::map<std::string, std::string>& projectOptions_)
    : actions(actions_), projectOptions(projectOptions_), currentState(None)
  {
  }

  void startElement(const XMLCh* const name, AttributeList& attributes)
  {
    if (XMLString::equals(name, _tagConfigurations))
    {
      state.reset(new ConfigurationsState(actions, projectOptions));
      currentState = Configuration;
    }
    else if (XMLString::equals(name, _tagBuild))
    {
      state.reset(new BuildState(actions, projectOptions));
      currentState = Build;
    }
    else if (currentState != Build && XMLString::equals(name, _tagOptions))
    {
      state.reset(new OptionsState(actions, projectOptions));
      currentState = Options;
    }

    if (state)
      state->startElement(name, attributes);
  }

  void characters(const XMLCh* const chars, const XMLSize_t length)
  {
    if (state)
      state->characters(chars, length);
  }

  void endElement(const XMLCh* const name)
  {
    if (state)
      state->endElement(name);
  }

  void endDocument()
  {
    if (state)
      state->endDocument();
  }

private:
  XmlString _tagConfigurations = "configurations";
  XmlString _tagBuild          = "build";
  XmlString _tagOptions        = "options";

  XMLParser::BuildActions& actions;
  std::map<std::string, std::string>& projectOptions;

  enum StateEnum {None, Configuration, Options, Build};

  std::unique_ptr<CcSAXParserState> state;
  StateEnum currentState;
};

class XecesXMLParser
{
private:
  class XecesPlatformInit
  {
  public:
    XecesPlatformInit()
    {
      try
      {
        XMLPlatformUtils::Initialize();
      }
      catch (XMLException& ex)
      {
        char* message = XMLString::transcode(ex.getMessage());

        SLog(cc::util::CRITICAL) << "XML toolkit initialization error: "
                                << message;

        XMLString::release(&message);

        throw;
      }
    }

    ~XecesPlatformInit()
    {
      try
      {
        XMLPlatformUtils::Terminate();
      }
      catch (XMLException& ex)
      {
        char* message = XMLString::transcode(ex.getMessage());

        SLog(cc::util::ERROR) << "XML toolkit teardown error: "
                                << message;

        XMLString::release(&message);
      }
    }
  };

private:
  std::string                           _config;
  std::map<std::string,  std::string>   _rootConfigs;
  std::map<std::string,  std::string>   _projectOptions;

public:
  static
  bool parse(const std::string&       xmlPath_,
             const std::string&       config_,
             XMLParser::BuildActions& buildActions_,
             std::map<std::string, std::string>& projectOptions_)
  {
    XecesPlatformInit platform;
    try
    {

      CcSAXParser handler(buildActions_, projectOptions_);

      SAXParser* parser = new SAXParser();

      parser->setValidationScheme(SAXParser::Val_Never);
      parser->setDoNamespaces(false);
      parser->setDoSchema(false);
      parser->setLoadExternalDTD(false);

      parser->setDocumentHandler(&handler);
      parser->setErrorHandler(&handler);
      parser->parse(xmlPath_.c_str());
      delete parser;
    }
    catch (XMLException& ex)
    {
      char* message = XMLString::transcode(ex.getMessage());

      SLog(cc::util::ERROR) << "XML error: "
                              << message;

      XMLString::release(&message);
      return false;
    }
    catch (SAXException& ex)
    {
      char* message = XMLString::transcode(ex.getMessage());

      SLog(cc::util::ERROR) << "SAX error: "
                              << message;

      XMLString::release(&message);
      return false;
    }
    catch (const std::exception& ex)
    {
      SLog(cc::util::ERROR) << ex.what() << std::endl;
      return false;
    }
    catch (...)
    {
      SLog(cc::util::ERROR) << "Unknown exception caught." << std::endl;
      return false;
    }

    return true;
  }

};

} // anonymous namespace

XMLParser::XMLParser()
{
}

void XMLParser::setConfig(const std::string& configName)
{
  _config = configName;
}

bool XMLParser::parse(const std::string& xmlPath_, BuildActions& buildActions_,
                      std::map<std::string, std::string>& projectOptions_)
{
  return XecesXMLParser::parse(xmlPath_, _config, buildActions_,
                               projectOptions_);
}
