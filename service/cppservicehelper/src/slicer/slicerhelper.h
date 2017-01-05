#ifndef SERVICE_CPPSERVICEHELPER_SLICER_SLICER_HELPER_H
#define SERVICE_CPPSERVICEHELPER_SLICER_SLICER_HELPER_H

#include "pdg.h"


namespace clang
{
  class Decl;
  class SourceManager;
}


namespace cc
{ 
namespace service
{  
namespace language
{

/**
 * This class connects the PDG to the CppService.
 * The UI send us a FilePosition, while the build algorithm of the PDG requires  
 * the Clanf AST node of the proper function declaration. This class computes that value
 */
class SlicerHelper
{
public:
  SlicerHelper(std::shared_ptr<odb::database> db_) : db(db_) {}
  
  void getBackwardSlicePos(std::vector<core::Range>& _return, const core::FilePosition& filePos);  
  void getForwardSlicePos(std::vector<core::Range>& _return, const core::FilePosition& filePos);

private:
  void buildPDG(PDG& pdg, const core::FilePosition& filePos);
  std::pair<clang::Decl*, clang::SourceManager*> getFunction(const core::FilePosition& filePos) const;
  Node::id_t getNodeId(const core::FilePosition& filePos) const;
  
  
  std::shared_ptr<odb::database> db;
  /**
   * Clang does not get the ownership of virtual files, thus we need to store them.
   * (During slicing we reparse the required files. We read this files from the database
   * and we do not want to recreate them in the filesistem, thus we use the Clangs map virtaul
   * file feature.
   */
  std::map<std::string, std::string> virtualFileStorage; 
};

} // language
} // service
} // cc

#endif