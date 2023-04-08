#ifndef CC_SERVICE_LSP_LSPSERVICE_H
#define CC_SERVICE_LSP_LSPSERVICE_H

#include <boost/property_tree/ptree.hpp>

#include <lspservice/lsp_types.h>

namespace cc
{ 
namespace service
{
namespace lsp
{

namespace pt = boost::property_tree;

class LspServiceHandler
{
public:
  virtual ~LspServiceHandler() = default;

  void virtual getDefinition(pt::ptree& responseTree_, const pt::ptree& params_);
  void virtual getImplementation(pt::ptree& responseTree_, const pt::ptree& params_);
  void virtual getReferences(pt::ptree& responseTree_, const pt::ptree& params_);
  void virtual getDiagramTypes(pt::ptree& responseTree_, const pt::ptree& params_);
  void virtual getDiagram(pt::ptree& responseTree_, const pt::ptree& params_);

  void getMethodNotFound(pt::ptree& responseTree_, const std::string& method_);
  void getParseError(pt::ptree& responseTree_, const std::exception& ex_);
  void getInternalError(pt::ptree& responseTree_, const std::exception& ex_);
  void getUnknownError(pt::ptree& responseTree_);

};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_LSPSERVICE_H