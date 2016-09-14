#ifndef CXXPARSER_METRICSCOLLECTOR_H
#define CXXPARSER_METRICSCOLLECTOR_H

#include <map>
#include <stack>
#include <string>
#include <sstream>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ASTContext.h>

#include <clang/Basic/SourceManager.h>

#include <parser/sourcemanager.h>

#include "model/workspace.h"
#include "model/metrics.h"
#include "model/metrics-odb.hxx"
#include "model/file.h"

#include "cxxparsesession.h"

namespace cc
{

namespace parser
{

class MetricsCollector : public clang::RecursiveASTVisitor<MetricsCollector>
{
public:
    MetricsCollector(std::shared_ptr<model::Workspace> w_,
                     clang::ASTContext& ctx_,
                     CxxParseSession& session_,
                     SourceManager& srcMgr_) 
                     : _w(w_),
                       _clangSourceManager(ctx_.getSourceManager()),
                       _clang2our(session_.clang2our),
                       _srcMgr(srcMgr_)
                       {}
    ~MetricsCollector()
    {
        util::OdbTransaction trans(*_w->getDb());
        trans([this]()
        {
//            for (auto element : _nodeMcCabeMetrics)
//            {
//                model::Metrics temp;
//                temp.nodeid = element.first;
//                temp.isFileId = false;
//                temp.metric = element.second;
//                
//                _w->getDb()->persist(temp);
//            }
            
            for (auto element : _fileMcCabeMetrics)
            {
                model::Metrics temp;
                temp.file = element.first;
                temp.type = model::Metrics::MCCABE;
                temp.metric = element.second;
              
//                model::Metrics temp;
//                std::ostringstream o;
//                o << element.first;
//                temp.nodeid = o.str();//std::to_string(element.first);
//                temp.isFileId = true;
//                temp.metric = element.second;
//                std::cout << temp.id << std::endl;
                
                _w->getDb()->persist(temp);
            }
        });
    }
    
    bool TraverseFunctionDecl(clang::FunctionDecl* decl)
    {
        _functionBodyHelper.push(decl);
        bool b = clang::RecursiveASTVisitor<MetricsCollector>::TraverseFunctionDecl(decl);
        _functionBodyHelper.pop();
        return b;
    }
    
    bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl)
    {
        _functionBodyHelper.push(decl);
        bool b = clang::RecursiveASTVisitor<MetricsCollector>::TraverseFunctionDecl(decl);
        _functionBodyHelper.pop();
        return b;
    }
    
    bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* decl)
    {
        _functionBodyHelper.push(decl);
        bool b = clang::RecursiveASTVisitor<MetricsCollector>::TraverseFunctionDecl(decl);
        _functionBodyHelper.pop();
        return b;
    }
    
    bool TraverseCXXConversionDecl(clang::CXXConversionDecl* decl)
    {
        _functionBodyHelper.push(decl);
        bool b = clang::RecursiveASTVisitor<MetricsCollector>::TraverseFunctionDecl(decl);
        _functionBodyHelper.pop();
        return b;
    }

    bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl* decl)
    {
        _functionBodyHelper.push(decl);
        bool b = clang::RecursiveASTVisitor<MetricsCollector>::TraverseFunctionDecl(decl);
        _functionBodyHelper.pop();
        return b;
    }

    bool VisitForStmt(clang::ForStmt* forStmt)
    {
        model::FileId id = getFileID(forStmt);
        addToMcCabe(id);
        
        return true;
    }

    bool VisitDoStmt(clang::DoStmt* doStmt)
    {
        model::FileId id = getFileID(doStmt);
        addToMcCabe(id);
        
        return true;
    }

    bool VisitIfStmt(clang::IfStmt* ifStmt)
    {
        model::FileId id = getFileID(ifStmt);
        addToMcCabe(id);
        
        //if else exists
        if (ifStmt->getElse() != nullptr)
        {
            addToMcCabe(id);
        }
        
        return true;
    }

    bool VisitSwitchCase(clang::SwitchCase* switchCase)
    {
        model::FileId id = getFileID(switchCase);
        addToMcCabe(id);
        
        return true;
    }

    bool VisitWhileStmt(clang::WhileStmt* whileStmt)
    {
        model::FileId id = getFileID(whileStmt);
        addToMcCabe(id);

        return true;
    }

private:
    void addToMcCabe(const model::FileId& fileId)
    {
        //File metric
        ++_fileMcCabeMetrics[fileId];
        
        //Function metric
        try
        {
            const model::CppAstNodePtr& astNode = _clang2our.at(_functionBodyHelper.top());
            std::string functionName = astNode->mangledName;
            ++_nodeMcCabeMetrics[functionName];
        }
        catch(const std::out_of_range& oor){}
    }

    model::FileId getFileID(const clang::Stmt* stmt) 
    {
        clang::SourceLocation sl = stmt->getLocStart();
        std::string fileName = _clangSourceManager.getFilename(sl).str();
        
        model::FilePtr filePtr;
        bool found = _srcMgr.findFile(fileName, filePtr);
        
        if (found)
        {
            return filePtr->id;
        }
        else
        {
            return 0;
        }
    }

    std::map<model::FileId, unsigned> _fileMcCabeMetrics;
    std::map<std::string, unsigned> _nodeMcCabeMetrics;
    std::stack<clang::FunctionDecl*> _functionBodyHelper;

    std::shared_ptr<model::Workspace> _w;
    const clang::SourceManager& _clangSourceManager;
    std::map<const void*, model::CppAstNodePtr>& _clang2our;
    SourceManager& _srcMgr;
};

} // parser
} // cc
#endif
