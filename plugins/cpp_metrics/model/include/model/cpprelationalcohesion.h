#ifndef CC_MODEL_CPPRELATIONALCOHESION_H
#define CC_MODEL_CPPRELATIONALCOHESION_H

#include <model/cppentity.h>
#include <model/cpprecord.h>
#include <model/cppastnode.h>
#include <model/cppfunction.h>
#include <model/cppvariable.h>

namespace cc
{
    namespace model
    {

        #pragma db view \
        object(File)
        struct RelationalCohesionFileView
        {
            #pragma db column(File::path)
            std::string filePath;

            #pragma db column(File::type)
            std::string fileType;

        };

        #pragma db view \
        object(CppRecord) \
        object(CppAstNode : CppRecord::astNodeId == CppAstNode::id) \
        object(File : CppAstNode::location.file) \
        object(CppMemberType : CppMemberType::memberAstNode) 
        struct RelationalCohesionRecordView
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

        #pragma db view \
        object(CppFunction) \
        object(CppAstNode : CppFunction::astNodeId == CppAstNode::id) \
        object(File : CppAstNode::location.file)
        struct RelationalCohesionFunctionView
        {
            #pragma db column(CppEntity::entityHash)
            std::size_t entityHash;
            
            #pragma db column(CppTypedEntity::typeHash)
            std::size_t returnType;

            #pragma db column(File::path)
            std::string filePath;
        };

        #pragma db view \
        object(CppEntity) \
        object(CppVariable = Parameters : CppFunction::parameters) \
        object(CppAstNode : CppAstNode.id == CppEntity.astNodeId) \
        object(File : CppAstNode::location.file)
        struct RelationalCohessionFunctionParameterView
        {
            #pragma db column(Parameters::typeHash)
            std::size_t typeHash;

            #pragma db column(File::path)
            std::string filePath;
        };


        #pragma db view \
        object(CppVariable = Locals : CppFunction::locals) \
        object(CppAstNode : CppAstNode.id == CppEntity.astNodeId) \
        object(File : CppAstNode::location.file)
        struct RelationalCohessionFunctionLocalView
        {
            #pragma db column(Locals::typeHash)
            std::size_t typeHash;

            #pragma db column(File::path)
            std::string filePath;
        };

        #pragma db view \
        object(CppVariable) \
        object(CppAstNode : CppAstNode.id == CppEntity.astNodeId) \
        object(File : CppAstNode::location.file)
        struct RelationalCohesionVariableView
        {
            #pragma db column(CppVariable::typeHash)
            std::size_t typeHash;

            #pragma db column(File::path)
            std::string filePath;

        };
        
    }
}

#endif