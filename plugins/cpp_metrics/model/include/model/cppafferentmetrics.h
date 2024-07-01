#ifndef CC_MODEL_AFFERENTMETRICS_H
#define CC_MODEL_AFFERENTMETRICS_H

#include <model/cppentity.h>
#include <model/cpprecord.h>
#include <model/cppastnode.h>

namespace cc
{
namespace model
{
 #pragma db view \
        object(CppRecord) \
        object(CppAstNode : CppRecord::astNodeId == CppAstNode::id) \
        object(File : CppAstNode::location.file) \
        object(CppMemberType : CppMemberType::memberAstNode) 
        struct AfferentRecordView
        {
            #pragma db column(CppEntity::entityHash)
            std::size_t entityHash;

            #pragma db column(CppMemberType::typeHash)
            std::size_t typeHash;

            #pragma db column(CppEntity::qualifiedName)
            std::string qualifiedName;

            #pragma db column(CppEntity::astNodeId)
            CppAstNodeId astNodeId;

            #pragma db column(File::path)
            std::string filePath;
        };

} //model
} //cc

#endif //CC_MODEL_AFFERENTMETRICS_H
