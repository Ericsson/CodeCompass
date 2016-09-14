/*
 * symbolhandler.cpp
 *
 *  Created on: Sep 4, 2013
 *      Author: ezoltbo
 */

#include <boost/algorithm/string_regex.hpp>

#include "cppservicehelper/symbolhandler.h"
#include "util/streamlog.h"

#include "model/cxx/cpptype.h"
#include "model/cxx/cpptype-odb.hxx"
#include "model/cxx/cpptypedef.h"
#include "model/cxx/cpptypedef-odb.hxx"
#include "model/cxx/cppfunction.h"
#include "model/cxx/cppfunction-odb.hxx"
#include "model/cxx/cppfunctionpointer.h"
#include "model/cxx/cppfunctionpointer-odb.hxx"
#include "model/cxx/cppvariable.h"
#include "model/cxx/cppvariable-odb.hxx"
#include "model/cxx/cppmacroexpansion.h"
#include "model/cxx/cppmacroexpansion-odb.hxx"
#include "model/cxx/cppinheritance.h"
#include "model/cxx/cppinheritance-odb.hxx"
#include "model/cxx/cppimplicit.h"
#include "model/cxx/cppimplicit-odb.hxx"
#include "model/cxx/cppfriendship.h"
#include "model/cxx/cppfriendship-odb.hxx"
#include "model/cxx/cppenum.h"
#include "model/cxx/cppenum-odb.hxx"
#include "model/cxx/cpprelation.h"
#include "model/cxx/cpprelation-odb.hxx"
#include <model/cxx/cpppointeranalysis.h>
#include <model/cxx/cpppointeranalysis-odb.hxx>
#include "diagram/diagram.h"
#include "utils.h"
#include <util/util.h>
#include <iostream>
namespace cc
{ 
namespace service
{  
namespace language
{

namespace
{ // unnamed namespace

class NodeFilterByPos
{
public:
  bool shouldSkip(const cc::model::CppAstNode& node_)
  {
    bool uniqueRange = true;
    auto ranges = _nodeRanges.equal_range(node_.astValue);
    for (auto it = ranges.first; it != ranges.second; ++it)
    {
      if (it->second == node_.location.range)
      {
        uniqueRange = false;
        break;
      }
    }
  
    if (!uniqueRange)
    {
      return true;
    }
    else
    {
      _nodeRanges.emplace(node_.astValue, node_.location.range);
      return false;
    }
  }

private:
  std::multimap<std::string, cc::model::Range> _nodeRanges;
};

auto sortByPosition = [](const model::CppAstNode& lhs,
                         const model::CppAstNode& rhs)
{
  return lhs.location.range.start < rhs.location.range.start;
};

} // unnamed namespace

InfoNode makeInfoNode(
  std::shared_ptr<odb::database> db,
  std::vector<std::string> category,
  const model::CppVariable& variable)
{
  CppOdbQuery query(db);

  auto node = db->load<model::CppAstNode>(variable.astNodeId.get());
  auto varInfo = createAstNodeInfo(*node);

  auto typeName = variable.qualifiedType;

  return makeInfoNode(std::move(category), variable.name, typeName, varInfo);
}

InfoNode makeInfoNode(
  std::vector<std::string> category_,
  std::string label_,
  std::string value_,
  AstNodeInfo astValue_)
{
  InfoNode infoNode;
  infoNode.category = std::move(category_);
  infoNode.label = label_;
  infoNode.value = value_;
  infoNode.astValue = astValue_;
  return infoNode;
}

InfoNode makeInfoNode(
  std::vector<std::string> category,
  const model::CppAstNode& call,
  const model::CppFunction& function)
{
  auto callInfo = createAstNodeInfo(call);
  const auto& range = call.location.range;
  const auto& file = call.location.file;

  std::string signature = getSignature(function);

  const auto& line = std::to_string(range.start.line);
  const auto& column = std::to_string(range.start.column);

  file.load();
  auto callStr = textRange(file->content.load()->content, range);

  category.push_back(signature);

  return makeInfoNode(std::move(category), line + ':' + column, callStr, callInfo);
}

std::vector<std::string> namespacesAsCategory(const std::string& qualifiedName)
{
  std::vector<std::string> namespaces;

  boost::split_regex(namespaces, qualifiedName, boost::regex("::"));

  namespaces.pop_back(); // drop name of the function/variable/type

  return namespaces;
}

InfoNode makeInfoNodeOfFunction(
  const model::CppFunction& function,
  odb::database& db_)
{
  auto node = db_.load<model::CppAstNode>(function.astNodeId.get());
  auto astNode = createAstNodeInfo(*node);
  auto qualifiedName = function.qualifiedName;
  auto namespaces = namespacesAsCategory(qualifiedName);

  return makeInfoNode(namespaces, "", function.name, astNode);
}

InfoNode makeInfoQueryNode(
  std::vector<std::string> category,
  int queryId,
  std::vector<std::string> filters)
{
  InfoQuery query;
  query.queryId = queryId;
  query.filters = std::move(filters);

  InfoNode ret;
  ret.category = std::move(category);
  ret.query = query;

  return ret;
}

std::vector<InfoNode> makeHitCountQueryNodes(
  std::shared_ptr<odb::database> db,
  const std::vector<model::AstCountGroupByFiles>& hitcounts,
  int queryId,
  std::vector<std::string> filters)
{
  typedef std::pair<model::File, unsigned> HitCount;
  std::vector<HitCount> hitcountsVec;
  hitcountsVec.reserve(128);

  for (const auto& hitcount : hitcounts)
  {
    if (hitcount.file != 0)
    {
      model::File file;
      db->load<model::File>(hitcount.file, file);

      hitcountsVec.push_back( { std::move(file), hitcount.count });
    }
  }

  std::sort(hitcountsVec.begin(), hitcountsVec.end(),
    [](const HitCount& lhs, const HitCount& rhs){
      return lhs.first.filename < rhs.first.filename;
  });

  std::vector<InfoNode> ret;
  ret.reserve(hitcountsVec.size());

  for (unsigned i = 0; i < hitcountsVec.size(); ++i)
  {
    const auto& hitcount = hitcountsVec[i];

    std::string cat = hitcount.first.filename + " ("
      + std::to_string(hitcount.second) + ')';

    if ((i > 0 && hitcount.first.filename == hitcountsVec[i-1].first.filename) ||
        (i < (hitcountsVec.size() - 1) &&
         hitcount.first.filename == hitcountsVec[i+1].first.filename))
    {
      cat += " in " + hitcount.first.path;
    }

    filters.push_back(std::to_string(hitcount.first.id));

    ret.push_back(makeInfoQueryNode( { cat }, queryId, filters));

    filters.pop_back();
  }

  return ret;
}

std::vector<InfoNode> makeHitCountQueryNodes(
  std::shared_ptr<odb::database> db,
  odb::result<model::AstCountGroupByFiles> hitcounts,
  int queryId,
  std::vector<std::string> filters)
{
  return makeHitCountQueryNodes(db, {hitcounts.begin(), hitcounts.end()},
    queryId, filters);
}

class FunctionHandler : public SymbolHandler
{
  enum class SubQuery
  {
    declarations = 1,
    parameters,
    locals,
    callees,
    callers,
    calledByInFile,
    assigned,
    assignedInFile,
    overrides,
    overriders
  };

public:
  FunctionHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog()<< "Trying to find CPPfunction with astnodeptr id:" << astNode.id;

    auto cppFunction = query.queryEntityByHash<model::CppFunction>(
      astNode.mangledNameHash);

    // name
    std::string shortName = baseName(cppFunction.qualifiedName, ':');
    ret.push_back(makeInfoNode({}, "Name", shortName));

    // qualified name
    ret.push_back(
      makeInfoNode({}, "Qualified name", cppFunction.qualifiedName));

    

    // return type:
    //   for built-in types: string
    //   for others: AstNodeInfo
    if (cppFunction.qualifiedType != "__*tor__none__")
    {
      // short name with signature
      ret.push_back(makeInfoNode({}, "Signature", cppFunction.name));
    
      if (cppFunction.typeHash != 0)
      {
        auto result = query.queryEntityByHash(cppFunction.typeHash);

        auto& type = *result.begin();

        auto typeAst = db->load<model::CppAstNode>(type.astNodeId.get());
        auto returnInfo = createAstNodeInfo(*typeAst);

        ret.push_back(
          makeInfoNode({}, "Return Type", cppFunction.qualifiedType, returnInfo));
      }
      else
      {
        ret.push_back(
          makeInfoNode({}, "Return Type", cppFunction.qualifiedType));
      }
    }
    else
    {
      // short name with signature
      cppFunction.name.erase(cppFunction.name.begin(), cppFunction.name.begin() + 5);
      ret.push_back(makeInfoNode({}, "Signature", cppFunction.name));
    }

    // definition
    SLog(util::DEBUG)<< "getting definition for " << astNode.id;
    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Defined", getFileloc(astNode), defAst));


    ret.push_back(makeInfoQueryNode({"Declarations"}, (int)SubQuery::declarations));
    ret.push_back(makeInfoQueryNode({"Parameters"}, (int)SubQuery::parameters));
    ret.push_back(makeInfoQueryNode({"Local Variables"}, (int)SubQuery::locals));
    ret.push_back(makeInfoQueryNode({"Callees"}, (int)SubQuery::callees));
    ret.push_back(makeInfoQueryNode({"Callers"}, (int)SubQuery::callers,
      {std::to_string(cppFunction.mangledNameHash)}));

    if (cppFunction.isVirtual)
    {
      ret.push_back(
        makeInfoQueryNode( { "Overrides" }, (int)SubQuery::overrides));
      ret.push_back(
        makeInfoQueryNode( { "Overriden by" }, (int)SubQuery::overriders));
    }

    ret.push_back(makeInfoQueryNode({"Assigned To Function Pointer"},
      (int)SubQuery::assigned));

    SLog(util::DEBUG)<< "Returning from getInfoTreeForFunction";

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    using namespace model;

    auto cppFunction =
      query.queryEntityByHash<CppFunction>(astNode.mangledNameHash);

    auto declarations = [this, &astNode, &cppFunction]()
    {
      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(model::CppAstNode::AstType::Declaration) && 
        CppOdbQuery::AstQuery::visibleInSourceCode == true,
        [](const model::CppAstNode& lhs,
          const model::CppAstNode& rhs)
        {
          const auto& lhsName = lhs.location.file.load()->filename;
          const auto& rhsName = rhs.location.file.load()->filename;
          return lhsName < rhsName;
        })
      )
      {
        auto declAst = createAstNodeInfo(ref);
        ret.push_back(
          makeInfoNode({}, "", getFileloc(ref), declAst));
      }

      return ret;
    };

    auto parameters = [this, &astNode, &cppFunction]()
    {
      std::vector<InfoNode> ret;
      for (auto param : cppFunction.parameters) // loop for all parameters
      {
        ret.push_back(makeInfoNode(db, {}, *param.load()));
      }
      return ret;
    };

    auto locals = [this, &astNode, &cppFunction]()
    {
      std::vector<InfoNode> ret;
      for (auto local : cppFunction.locals) // loop for all parameters
      {
        ret.push_back(makeInfoNode(db, {}, *local.load()));
      }

      std::sort(ret.begin(), ret.end(), [](const InfoNode& lhs, const InfoNode& rhs)
        {
          return lhs.label < rhs.label;
        });

      return ret;
    };

    auto callees = [this, &astNode, &cppFunction]()
    {
      std::vector<InfoNode> ret;
      for (auto call : query.sortQuery<CppAstNode>
        (CppOdbQuery::queryCallsInAstNode(astNode), sortByPosition)) // loop for function calls
      {
        try
        {
          auto callFunc = query.queryEntityByHash<model::CppFunction>(
            query.loadDefinitionOrDeclaration(call)[0].mangledNameHash);

          ret.push_back(makeInfoNode( { }, call, callFunc));
        } catch (const std::exception& ex)
        {
          SLog(util::ERROR)<< "Exception caught while callee was: " << call.astValue
          << " Message: " << ex.what();
        }
      }
      return ret;
    };

    auto callers = [&, this]()
    {
      if (infoQuery.filters.empty())
      {
        throw std::runtime_error("Empty InfoQuery::filters in FunctionHandler::callers");
      }

      model::HashType mangledNameHash = stoull(infoQuery.filters.front());

      if (mangledNameHash != cppFunction.mangledNameHash)
      {
        cppFunction = query.queryEntityByHash<CppFunction>(mangledNameHash);
      }

      typedef odb::query<model::AstCountGroupByFiles>  ACQuery;
      typedef odb::result<model::AstCountGroupByFiles> ACResult;

      typedef std::map<model::FileId, int> FileIdCount;

      FileIdCount fidCount;

      auto collectResult = [&](ACResult result)
      {
        for (auto row : result)
        {
          fidCount[row.file] += row.count;
        }
      };

      auto GroupByResult = [this](model::HashType hash, model::CppAstNode::AstType astType)
      {
        return db->query<model::AstCountGroupByFiles>(
          ACQuery::CppAstNode::mangledNameHash == hash &&
          ACQuery::CppAstNode::astType == astType
        );
      };

      auto CollectRelationResult =
      [&](const std::set<model::HashType>& hashes, model::CppAstNode::AstType astType)
      {
        for (auto elem : hashes)
        {
          auto callers = GroupByResult(elem, astType);
          collectResult(callers);
        }
      };

      // direct callers
      auto directCallers = GroupByResult(mangledNameHash, model::CppAstNode::AstType::Usage);
      collectResult(directCallers);

      // virtual callers
      if (cppFunction.isVirtual)
      {
        auto result = query.reverseTransitiveClosureOfRel(
                CppRelation::Kind::Override, mangledNameHash);

        result.insert(mangledNameHash);

        CollectRelationResult(result, model::CppAstNode::AstType::VirtualCall);
      }

      // func ptr callers
      {
        auto result = query.reverseTransitiveClosureOfRel(
                CppRelation::Kind::Assign, mangledNameHash);

        CollectRelationResult(result, model::CppAstNode::AstType::Usage);
      }

      std::vector<model::AstCountGroupByFiles> accumulatedResult(fidCount.size());

      std::transform(fidCount.begin(), fidCount.end(), accumulatedResult.begin(),
        [](const std::pair<model::FileId, int>& elem)
        {
          model::AstCountGroupByFiles ret;

          ret.file = elem.first;
          ret.count = elem.second;

          return ret;
        });

      return makeHitCountQueryNodes(db, accumulatedResult, (int)SubQuery::calledByInFile, infoQuery.filters);
    };

    auto overrides = [this, &cppFunction]()
    {
      std::vector<InfoNode> ret;

      auto overrideds = query.reverseTransitiveClosureOfRel(
        CppRelation::Kind::Override, cppFunction.mangledNameHash);

      for (auto overrided : overrideds)
      {
        auto overFunc = query.queryEntityByHash<CppFunction>(overrided);

        ret.push_back(makeInfoNodeOfFunction(overFunc, *db));
      }

      return ret;
    };

    auto overriders = [this, &cppFunction]()
    {
      std::vector<InfoNode> ret;

      auto overriders = query.getTransitiveClosureOfRel(
        CppRelation::Kind::Override, cppFunction.mangledNameHash);

      for (auto overrider : overriders)
      {
        auto overFunc = query.queryEntityByHash<CppFunction>(overrider);

        ret.push_back(makeInfoNodeOfFunction(overFunc, *db));
      }

      return ret;
    };

    auto assigned = [this, &astNode, &cppFunction]()
    {
      typedef odb::query<model::AstCountGroupByFiles> ACQuery;

      auto resultSet = db->query<model::AstCountGroupByFiles>(
        ACQuery::CppAstNode::mangledNameHash == astNode.mangledNameHash &&
        ACQuery::CppAstNode::astType == model::CppAstNode::AstType::Read
      );

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::assignedInFile);
    };

    auto calledByInFile = [this, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      if(infoQuery.filters.empty())
        throw std::runtime_error("No file ID in InfoQuery.filters");

      auto mangledNameHash   = std::stoull(infoQuery.filters[0]);
      auto fileId = std::stoull(infoQuery.filters[1]);

      auto callers = getCallers(mangledNameHash, fileId);
      
      std::unordered_set<model::HashType> functionHashes;

      // create the result
      for (auto caller : callers)
      {
        try
        {
          auto outerHash = query.loadOuterFunction(caller).mangledNameHash;
          auto outerFunc = query.queryEntityByHash<model::CppFunction>(
            outerHash);

          if (functionHashes.find(outerHash) == functionHashes.end())
          {
            functionHashes.insert(outerHash);

            ret.push_back(makeInfoQueryNode({getSignature(outerFunc), {"Callers"}}, (int)SubQuery::callers,
              {std::to_string(outerHash)}));
          }

          ret.push_back(
            makeInfoNode( {}, caller, outerFunc));
        } catch (const std::exception& ex)
        {
          SLog(util::ERROR)<< "Exception caught while caller was: " << caller.astValue
          << " Message: " << ex.what();
        }
      }
      return ret;
    };

    auto assignedInFile = [this, &astNode, &cppFunction, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      if(infoQuery.filters.empty())
        throw std::runtime_error("No file ID in InfoQuery.filters");

      auto fileId = std::stoull(infoQuery.filters[0]);

      SLog(util::DEBUG)<< "getting readers for " << astNode.id;
      for (auto caller : query.sortQuery<model::CppAstNode>(CppOdbQuery::queryReads(astNode)
        && CppOdbQuery::astFileId(fileId), sortByPosition)) // loop for calls on this function
      {
        auto callerInfo = createAstNodeInfo(caller);

        auto filename = baseName(caller.location.file->path);
        const auto& line = std::to_string(caller.location.range.start.line);
        const auto& column = std::to_string(caller.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            callerInfo.astNodeSrcText, callerInfo));
      }
      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);
    switch (queryId)
    {
      case SubQuery::declarations:
        return declarations();
      case SubQuery::parameters:
        return parameters();
      case SubQuery::locals:
        return locals();
      case SubQuery::callees:
        return callees();
      case SubQuery::callers:
        return callers();
      case SubQuery::calledByInFile:
        return calledByInFile();
      case SubQuery::assigned:
        return assigned();
      case SubQuery::overrides:
        return overrides();
      case SubQuery::overriders:
        return overriders();
      case SubQuery::assignedInFile:
        return assignedInFile();
      default:
        break;
    }

    return {};
  }
};

class VariableHandler : public SymbolHandler
{
public:
  enum class SubQuery
  {
    usage = 1,
    usageInFile,
    aliases,
  };
  
  VariableHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }
  
  class PointerAnalysisHandler
  {
  public : 
    PointerAnalysisHandler(std::shared_ptr<odb::database> db, const model::CppAstNode& astNode)
      :db(db), astNode(astNode)
    {      
    }
    
    bool isNullPtr(model::HashType h)
    {
      return h == util::fnvHash("nullptr");
    }
    /**
     * Collect assignment statements for algorithm
     */
    bool collectStatements()
    {      
      #if defined(DATABASE_PGSQL2)
          db->execute("\
            CREATE OR REPLACE FUNCTION get_all_statements(use_parent bigint, recursivelyFindAll integer) RETURNS setof \"CppPointerAnalysis\" AS $$ \
            DECLARE\
                process_node bigint[] := ARRAY[ use_parent ];\
                statements bigint[] := '{}';\
                parent bigint[] := '{}';\
                new_statement bigint[];\
                new_parent bigint[];\
            BEGIN\
              WHILE ( array_upper( process_node, 1 ) IS NOT NULL ) LOOP	\
                new_statement := ARRAY( \
                  SELECT rhs FROM \"CppPointerAnalysis\" WHERE lhs = ANY( process_node ) AND rhs <> ALL( statements )\
                  UNION\
                  SELECT lhs FROM \"CppPointerAnalysis\" WHERE rhs = ANY( process_node ) AND lhs <> ALL( statements )\
                );\
                statements := statements || new_statement;\
                IF recursivelyFindAll <> 0 THEN\
                  process_node := new_statement;\
                ELSE\
                  process_node := '{}';\
                END IF;\
              END LOOP;\
              RETURN QUERY Select * from \"CppPointerAnalysis\" where  statements @> array[lhs] OR statements @> array[rhs]; \
            END;\
            $$ LANGUAGE plpgsql;"
          );      
          auto states = db->query<cc::model::GetAllStatements> ("select * from get_all_statements(" + std::to_string((signed long long)astNode.mangledNameHash) + "," + std::to_string(recursivelyFindAll) +" );");

        for(auto state : states)
        {
          variables.push_back(state.lhs);
          variables.push_back(state.rhs);

          cc::model::CppPointerAnalysis cp;
          cp.id = state.id;
          cp.lhs = state.lhs;
          cp.rhs = state.rhs;
          cp.lhsOperators = state.lhsOperators;
          cp.rhsOperators = state.rhsOperators;
          statements.push_back(std::move(cp));
        }
          std::cout<< "STATES "<<states.size()<<std::endl;
      #else
        typedef typename odb::query<model::CppPointerAnalysis> Query;

        auto start = std::chrono::system_clock::now();
       
        while(!q.empty())
        {
          auto end = std::chrono::system_clock::now();
          if( std::chrono::duration_cast<std::chrono::seconds>(end - start) > std::chrono::seconds(10) )
          {
            return false;
          }
          auto hash = q.front();            
          auto result = db->query<model::CppPointerAnalysis>(
            Query::lhs == hash ||
            Query::rhs == hash
          );
          
          for(auto pa : result)
          {
            // process left side of an assign 
            if(std::find(variables.begin(), variables.end(), pa.lhs) == variables.end())
            {
              if(!isNullPtr(pa.lhs))
              {
                variables.push_back(pa.lhs);
                q.push_back(pa.lhs);
              }else
                nullPointerVariables.push_back(pa.rhs);
            }

            // process right side of an assign 
            if(std::find(variables.begin(), variables.end(), pa.rhs) == variables.end())
            {
              if(!isNullPtr(pa.rhs))
              {
                variables.push_back(pa.rhs);
                q.push_back(pa.rhs);
              }else
                nullPointerVariables.push_back(pa.lhs);
            }

            if(!isNullPtr(pa.lhs) && !isNullPtr(pa.rhs) && 
              std::find(statements.begin(), statements.end(), pa) == statements.end())
            {
              if(pa.rhs == hash ||
                 pa.lhsOperators.find("*") != std::string::npos || 
                 pa.rhsOperators.find("*") != std::string::npos)
                statements.push_back(pa);
              else
                statements.insert(statements.begin(),pa);
            }
          }
          
          if(result.size() > dbMaxResultSize)
            return false;
          
          q.pop_front();
        }
        return true;
      #endif         
    }
  
    /**
     * Recursively evaluate the right side of an assign
     * @param h - astNode hash, represent variable
     * @param op - variable operators (&, *)
     */
    std::set<model::HashType> eval_rhs(model::HashType h, std::string op) 
    {
      std::set<model::HashType> _ret;

      if(op.size() == 0)
        _ret.insert(h);

      if(op.size() != 0 && op[0] == '&')
        for(auto i : eval_rhs(h, op.erase(0,1)))
          _ret.insert(i);

      if(op.size() != 0 && op[0] == '*')
      {
        std::string o = op.erase(0,1);        
        for(auto e : PT[h])
          for(auto i : eval_rhs(std::get<0>(e), o))
            _ret.insert(i);
      }

      return _ret;
    };

    /**
     * Evaluate the left side of an assign
     * @param h - astNode hash, represent variable
     * @param op - variable operators (&, *)
     */
    std::set<model::HashType> eval_lhs(model::HashType h, std::string op)
    {
      return eval_rhs(h,op);
    }; 
    
    
    /**
     * TODO : use getEntityByHash, new operator can be weard
     * Get astNode by hash from database
     * @param v  - astNode hash, represent variable
     */
    model::CppAstNode getAstNode(const model::HashType& v)
    {
      if(astNodes.count(v) == 0)
      {
        model::CppAstNode actualAstNode;

        auto result = db->query<model::CppAstNode>(
            CppOdbQuery::astMangledNameHash(v) && 
            (CppOdbQuery::astAstType(model::CppAstNode::AstType::Definition) || 
            CppOdbQuery::astAstType(model::CppAstNode::AstType::Declaration)) && 
            CppOdbQuery::AstQuery::visibleInSourceCode == true);
        if (!result.empty())
        {
          actualAstNode = *result.begin();
        }else{
          auto entities = db->query<model::CppAstNode>(
            CppOdbQuery::astMangledNameHash(v) && 
            CppOdbQuery::AstQuery::visibleInSourceCode == true);
          if (!entities.empty())
          {
            actualAstNode = *entities.begin();
          }
          else
          {
            auto eResult = db->query<model::CppAstNode>(
            CppOdbQuery::astMangledNameHash(v));
            if (!eResult.empty())
              actualAstNode = *eResult.begin();
            else 
            {
              auto resultById = db->query<model::CppAstNode>(
                CppOdbQuery::AstQuery::id == v);
    
              if (!resultById.empty())
              {
                actualAstNode = *resultById.begin();
              }
            }
          }
        }
        
        astNodes[v] = std::move(actualAstNode); 
      }
      
      return astNodes[v];
    }
    
    /**
     * Create graph node from v
     * @param v - variable mangledNameHash
     * @param unaryOperator
     * @return graph node
     */
    util::Graph::Node createNode(const model::HashType& v, const std::string& unaryOperator)
    {      
      if (graphNodes.count(v) == 0)
      {
        util::Graph::Node node;
        model::CppAstNode actualAstNode = getAstNode(v);
        if(actualAstNode.id != 0 && astNodes.count(v) != 0)
        {
          std::string label = astNodes[v].astValue + unaryOperator;
          if(label != "")
          {
            node = graph.addNode(mainSGraph);
            graph.setAttribute(node, "id", std::to_string(astNodes[v].id));
            graph.setAttribute(node, "shape", "circle");
            graph.setAttribute(node, "label", label);
          }          
        }
        graphNodes[v] = std::move(node);
      }
      return graphNodes[v];
    }

    /**
     * Adding a new graph node to an existing subgraph or create a new one
     * @param lhs - parent node
     * @param rhs - child node
     */
    bool createAndSetSubgraph(const model::HashType& lhs, const model::HashType& rhs)
    {
      bool isNewEdge = false;
      util::Graph::Node node;
      util::Graph::Subgraph sgraph;

      model::CppAstNode actualAstNodeLhs = getAstNode(lhs);
      model::CppAstNode actualAstNodeRhs = getAstNode(rhs);  
              
      if(actualAstNodeLhs.id == 0 || actualAstNodeRhs.id == 0)
        return false;
            
      if(graphNodes.count(rhs) == 0)
        createNode(rhs, "");
      
      if(subgraphs.count(rhs) == 0)
      {
        sgraph = graph.addSubgraph("");
        graph.setAttribute(sgraph,"rankdir","RL");
        graph.setAttribute(sgraph,"rank","same");

        // remove node from group
        auto newNode = graph.addNode(sgraph);
        
        if(graphNodes.count(rhs) != 0)
        {
          for(auto c : graph.getChildren(graphNodes[rhs]))
          {
            auto oldEdge = edges[graphNodes[rhs]][c];
            auto edge = graph.addEdge(newNode,c);
            if(oldEdge.id != "")
            {
              graph.setAttribute(edge, "style", graph.getAttribute(oldEdge, "style"));
              graph.setAttribute(edge, "color", graph.getAttribute(oldEdge, "color")); 
            }
            edges[graphNodes[rhs]][c] = edge;

          }
          for(auto p : graph.getParents(graphNodes[rhs]))
          {
            auto oldEdge = edges[p][graphNodes[rhs]];
            auto edge = graph.addEdge(p, newNode);
            if(oldEdge.id != "")
            {
              graph.setAttribute(edge, "style", graph.getAttribute(oldEdge, "style")); 
              graph.setAttribute(edge, "color", graph.getAttribute(oldEdge, "color")); 
            }
            edges[p][graphNodes[rhs]] = edge;
          }
          graph.delNode(graphNodes[rhs]);
        }
        
        if(actualAstNodeRhs.id != 0 && astNodes.count(rhs) != 0)
        {
          graphNodes[rhs] = std::move(newNode);
          
          graph.setAttribute(graphNodes[rhs], "id", std::to_string(astNodes[rhs].id));
          graph.setAttribute(graphNodes[rhs], "shape", "circle");

          graph.setAttribute(graphNodes[rhs], "label", astNodes[rhs].astValue);
          if(astNodes[rhs].mangledNameHash == astNode.mangledNameHash) // current
          {
            graph.setAttribute(graphNodes[rhs], "style", "filled");
            graph.setAttribute(graphNodes[rhs], "fillcolor", "gold");
          }          
        }
        subgraphs[rhs] = sgraph;
        subgraphs[lhs] = sgraph;    
      }
      else
      {
        sgraph = subgraphs[rhs];
      }
      if(graphNodes.count(lhs) != 0)
      {
        node = graphNodes[lhs];
      }
      else
      {
        node = graph.addNode(sgraph);    
      }
      if(astNodes[lhs].id != 0)
      {
        graph.setAttribute(node, "id", std::to_string(astNodes[lhs].id));
        graph.setAttribute(node, "shape", "circle");
        graph.setAttribute(node, "label", astNodes[lhs].astValue);      
        if(astNodes[lhs].mangledNameHash == astNode.mangledNameHash) // current
        {
          graph.setAttribute(node, "style", "filled");
          graph.setAttribute(node, "fillcolor", "gold");
        }

        if(graphNodes[lhs].id != node.id)
          graphNodes[lhs] = std::move(node);
        
        if(!graph.hasEdge(graphNodes[lhs], graphNodes[rhs]))
        {
          isNewEdge = true;
          auto edge = graph.addEdge(graphNodes[lhs], graphNodes[rhs]);        
          graph.setAttribute(edge, "style", "dashed"); 
          graph.setAttribute(edge, "color", "grey"); 

          edges[graphNodes[lhs]][graphNodes[rhs]] = edge;
        }
      }
      return isNewEdge;
    }
        
   void timeoutExceed(bool generateGraph)
   {
    if(!isTimeoutExceed)
    {
      if(generateGraph)
      {
        util::Graph::Node node = graph.addNode();
        graph.setAttribute(node, "id", std::to_string(astNode.id));
        graph.setAttribute(node, "shape", "note");
        graph.setAttribute(node, "label", "Timeout exceeded : can't get all aliases and edges!");
        graph.setAttribute(node, "fontsize", "24");
        graph.setAttribute(node, "style", "filled");
        graph.setAttribute(node, "fillcolor", "red");
 // 
 //       util::Graph::Edge edge = graph.addEdge(rootNode, node);
 //       graph.setAttribute(edge, "arrowhead", "empty");      
 //       graph.setAttribute(edge, "color", "red");      

      }
      else
      {
        auto refInfo = createAstNodeInfo(astNode);
        if(PT[astNode.mangledNameHash].empty())
        {
          ret.push_back(
              makeInfoNode( {}, 
                "Timeout exceeded :",
                "can't get aliases!", refInfo));
        }
        else
        {
          ret.push_back(
                makeInfoNode( {}, 
                  "Timeout exceeded :",
                  "can't get all aliases!", refInfo));
        }
      }
      isTimeoutExceed = true;
    }
   }
   
   /**
    * 
    * @param fromNode
    * @param toNode
    * @return 
    */
   bool addEdge(util::Graph::Node& fromNode, util::Graph::Node& toNode)
   {     
     util::Graph::Edge edge;
     
     std::string fromId = fromNode.id;
     std::string toId = toNode.id;
     
     if(fromId.find_first_not_of(' ') != std::string::npos && 
        toId.find_first_not_of(' ') != std::string::npos   && 
        !graph.hasEdge(fromNode, toNode))
     {
      edge = graph.addEdge(fromNode, toNode);
      edges[fromNode][toNode] = edge;
      return true;
     }
     
     return false;
   }
    /**
     * Pointer Analysis Algorithm
     * @param generateGraph - generate graph if true
     */
    void pointsToAlgorithm(bool generateGraph = true)
    {
      bool isAllStatement = collectStatements(); // Collect statements
      
      auto start = std::chrono::system_clock::now();
      for(auto state : statements)
      {           
        auto end = std::chrono::system_clock::now();
        if( std::chrono::duration_cast<std::chrono::seconds>(end - start) > std::chrono::seconds(10) )
        {          
          timeoutExceed(generateGraph);
          return;
        }
        auto lhsOp = state.lhsOperators;
        auto rhsOp = state.rhsOperators;

        if((rhsOp.size() == 0 || rhsOp.find("&") == std::string::npos)  &&
            lhsOp.find("&") == std::string::npos && state.rhs != util::fnvHash("nullptr")){
          rhsOp += "*";
        }

        auto L = eval_lhs(state.lhs, state.lhsOperators);
        auto R = eval_rhs(state.rhs, rhsOp);
        
        for(auto lhs : L)
        {
          for(auto rhs : R)
          {  
            bool isAliases = lhsOp.find("&") != std::string::npos;

            PT[lhs].insert(std::make_tuple(rhs,isAliases));              
          } 
        }
      }
      
      if(!isAllStatement)
        timeoutExceed(generateGraph);
    }
    
    /**
     * Get aliases for info tree
     */
    std::vector<InfoNode> getInfoTree()
    {      
      q.push_back(astNode.mangledNameHash);
      variables.push_back(astNode.mangledNameHash); 
      
      pointsToAlgorithm(false);
      
      
      std::set<model::HashType> processed;      
      std::function<void (const model::HashType)> collectAliasesOfAlias = [&](const model::HashType v)
      {
        processed.insert(v);
        for(const auto& rhs : PT[v])
        {
          if(processed.find(std::get<0>(rhs)) == processed.end())
          {
            PT[astNode.mangledNameHash].insert(rhs);            
            collectAliasesOfAlias(std::get<0>(rhs));
          }
        }
        
        for(auto from : PT)
        {
          auto to = std::find_if(from.second.begin(), from.second.end(), 
            [&v](const auto& item) 
            { 
              return std::get<0>(item) == v; 
            }
          );
          if(to != from.second.end())
          {
            if(processed.find(from.first) == processed.end())
            {
              PT[astNode.mangledNameHash].insert(std::make_tuple(from.first, false));              
              collectAliasesOfAlias(from.first);
            }
          }
        }
        
      };
      
      collectAliasesOfAlias(astNode.mangledNameHash);
      
      std::map<std::string, std::vector<model::CppAstNode>> groupByFile;

      // Group astNode by filename
      for(auto variable : PT[astNode.mangledNameHash]) 
      {
        if(std::get<0>(variable) != astNode.mangledNameHash && astNode.location.file)
        {
          model::CppAstNode astNode = getAstNode(std::get<0>(variable));

          if(astNode.location.file)
          {
            std::string fileName = astNode.location.file.load()->filename;

            groupByFile[fileName].push_back(std::move(astNode));
          }

        }
      }   

      for(auto group : groupByFile) 
      {
        for(const auto& astNode : group.second)
        {
          if(astNode.location.file)
          {
            auto refInfo = createAstNodeInfo(astNode);

            const auto& line = std::to_string(astNode.location.range.start.line);
            const auto& column = std::to_string(astNode.location.range.start.column);

            std::string dir = astNode.location.file->filename + " (" + 
                std::to_string(groupByFile[group.first].size()) + ")";
            
            ret.push_back(
                makeInfoNode( {dir}, 
                  line + ':' + column,
                  refInfo.astNodeSrcText, refInfo));
                
              /*ret.push_back(makeInfoQueryNode({dir, refInfo.astNodeSrcText, "Aliases of this"}, (int)VariableHandler::SubQuery::aliases,
                  {"ar", std::to_string(astNode.mangledNameHash)}));

              ret.push_back(
                makeInfoNode( {dir, refInfo.astNodeSrcText}, 
                  line + ':' + column,
                  refInfo.astNodeSrcText, refInfo));*/
          }
        }
      }      
      
      return ret;
    }
    
    /**
     * Get pointer analysis diagram
     * @return diagram
     */
    std::string getDiagram()
    {
      if(astNode.symbolType == model::CppAstNode::SymbolType::Function)
        astNode.mangledNameHash = astNode.id;
      
      q.push_back(astNode.mangledNameHash);
      variables.push_back(astNode.mangledNameHash); 
      
      graph.setAttribute("rankdir", "BT");
      graph.setAttribute("nodesep", "0.4");
      graph.setAttribute("ranksep", "0.7");
      
      mainSGraph = graph.addSubgraph(""); // every non aliases in this group
    
      auto node = createNode(astNode.mangledNameHash, "");
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", "gold");
        
      pointsToAlgorithm();
      
      auto drawEdge = [&](const model::HashType from, const model::HashType to, bool isAliases)
      {                  
        if(isAliases){          
          if(createAndSetSubgraph(from, to))
            return true;        
        }else
        {
          model::CppAstNode lhsAstNode = getAstNode(from);
          model::CppAstNode rhsAstNode = getAstNode(to);
          if(lhsAstNode.id != 0  && rhsAstNode.id != 0)
          {          
            util::Graph::Node fromNode = createNode(from, "");    
            util::Graph::Node toNode = createNode(to, "");

            if(addEdge(fromNode, toNode))
              return true;
          }
        }
        return false;
      };
      
      std::queue<model::HashType> recQ;
      recQ.push(astNode.mangledNameHash);
      bool firstLevel = true;
      
      auto start = std::chrono::system_clock::now();
      while(!recQ.empty())
      {                
        auto end = std::chrono::system_clock::now();
        if( std::chrono::duration_cast<std::chrono::seconds>(end - start) > std::chrono::seconds(10) )
        {          
          timeoutExceed(true);
          break;
        }
        
        auto v = recQ.front();
        if(firstLevel)
        {
          for(const auto& rhs : PT[v])
          {
            if(drawEdge(v, std::get<0>(rhs), std::get<1>(rhs)))
              recQ.push(std::get<0>(rhs));
          }
          firstLevel = false;
        }

        for(auto from : PT)
        {
          auto to = std::find_if(from.second.begin(), from.second.end(), 
            [&v](const auto& item) 
            { 
              return std::get<0>(item) == v; 
            }
          );
          if(to != from.second.end())
          {
            if(drawEdge(from.first, v, std::get<1>(*to)))
              recQ.push(from.first);
          }
        }
        recQ.pop();
      }
      
      for(auto nullPointerVar : nullPointerVariables)
      {
        if(graphNodes.count(nullPointerVar) != 0)
          graph.setAttribute(graphNodes[nullPointerVar], "shape", "Mcircle");
      }
      
      return graph.output(util::Graph::SVG);
    }
    
  private:        
    std::shared_ptr<odb::database> db;
    bool isTimeoutExceed;
    std::vector<model::CppPointerAnalysis> statements; // statements for algorithm
    std::vector<model::HashType> variables; // all variables in statements    
    std::vector<model::HashType> nullPointerVariables;
    std::map<model::HashType, std::set<std::tuple<model::HashType, bool>>> PT; // point to set for aliases

    std::set<std::tuple<model::HashType, bool>> aliasesAlias; // alias of an alias

    std::deque<model::HashType> q; // queue for collecting statements

    std::map<model::HashType, util::Graph::Node> graphNodes; // Graph nodes
    std::map<model::HashType, cc::model::CppAstNode> astNodes; // Aliases astNode

    std::map<util::Graph::Node, std::map<util::Graph::Node, util::Graph::Edge>> edges;
    // Pointer analyis graph
    util::Graph graph;
    std::map<model::HashType, util::Graph::Subgraph> subgraphs; // subgraphs of aliases
    util::Graph::Subgraph mainSGraph;
    
    model::CppAstNode astNode;
    
    std::vector<InfoNode> ret;
    
    static const int dbMaxResultSize = 100;
  };
  
  std::vector<InfoNode> getPointerAnalysisInfoTree(const model::CppAstNode& astNode) override
  {
    try
    {
      // TODO: Better solution for checking table existence or emptyness?
      db->query_value<model::CppPointerAnalysisCount>();
    }
    catch (const odb::exception& ex)
    {
      std::vector<InfoNode> ret;

      ret.push_back(makeInfoNode({}, "Error", "Pointer Analysis is not available! You should parse your project again."));
      return ret;
    }
    PointerAnalysisHandler pah(db, astNode);
    return pah.getInfoTree();
  }
  
  std::string getPointerAnalysisDiagram(const model::CppAstNode& astNode) override
  {
    try
    {
      // TODO: Better solution for checking table existence or emptyness?
      db->query_value<model::CppPointerAnalysisCount>();
    }
    catch (const odb::exception& ex)
    {
      util::Graph graph;
      auto node = graph.addNode();
      graph.setAttribute(node, "id", "dberror");
      graph.setAttribute(node, "label", "Pointer Analysis is not available! You should parse your project again.");
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", "red");
          
      return graph.output(util::Graph::SVG);      
    }
    PointerAnalysisHandler pah(db, astNode);
    return pah.getDiagram();
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    using namespace model;
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)<< "Trying to find CPPvariable with astnodeptr id:"<< astNode.id;

    auto cppVariable = query.queryEntityByHash<CppVariable>(
      astNode.mangledNameHash);

    // short name with signature
    ret.push_back(makeInfoNode({}, "Name", cppVariable.name));

    // qualified name
    ret.push_back(
      makeInfoNode({}, "Qualified Name", cppVariable.qualifiedName));

    //type
    if (cppVariable.typeHash != 0)
    {
      AstNodeInfo info;

      typedef odb::query<model::CppEntity> EQ;

      auto r = db->query<model::CppEntity>(
        EQ::mangledNameHash == cppVariable.typeHash);
      if (!r.empty())
      {
        model::CppEntity& entity = *r.begin();

        auto node = db->load<model::CppAstNode>(entity.astNodeId.get());
        info = createAstNodeInfo(*node);
      }

      ret.push_back(makeInfoNode({}, "Type", cppVariable.qualifiedType, info));
    }
    else
    {
      ret.push_back(makeInfoNode({}, "Type", cppVariable.qualifiedType));
    }

    // declared
    SLog(util::DEBUG)<< "getting declaration for " << astNode.id;
    AstNodeInfo vDecl = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Declaration", getFileloc(astNode), vDecl));

    ret.push_back(makeInfoQueryNode({"Reads"}, (int)SubQuery::usage, {"r"}));
    ret.push_back(makeInfoQueryNode({"Writes"}, (int)SubQuery::usage, {"w"}));
    ret.push_back(makeInfoQueryNode({"Aliases"}, (int)SubQuery::aliases, {"a"}));
    
    SLog(util::DEBUG)<< "Returning from getInfoTreeForVariable";

    return ret;
  }

  
  
  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    auto cppVariable = query.queryEntityByHash<model::CppVariable>(
      astNode.mangledNameHash);

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usage = [this, &astNode](model::CppAstNode::AstType astType, const std::string& use)
    {
      auto resultSet = db->query<model::AstCountGroupByFiles>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType));

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::usageInFile, {use});
    };

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usageInFile = [this, &astNode](model::CppAstNode::AstType astType, model::FileId fileId)
    {
      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType) &&
        CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        auto refInfo = createAstNodeInfo(ref);

        auto filename = baseName(ref.location.file->path);
        const auto& line = std::to_string(ref.location.range.start.line);
        const auto& column = std::to_string(ref.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            refInfo.astNodeSrcText, refInfo));
      }

      return ret;
    };

    if(infoQuery.filters.empty())
      throw std::runtime_error("No use-type (rear or write) in InfoQuery.filters");

    auto use = infoQuery.filters.front();
    model::CppAstNode::AstType astType;

    if (use == "r")
      astType = model::CppAstNode::AstType::Read;
    else
      astType = model::CppAstNode::AstType::Write;

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      
      case SubQuery::usage:
        return usage(astType, use);
      case SubQuery::usageInFile:
      {
        if(infoQuery.filters.size() != 2)
          throw std::runtime_error("Invalid InfoQuery.filters");

        auto fileId = stoull(infoQuery.filters[1]);
        return usageInFile(astType, fileId);
      }
      case SubQuery::aliases:
      {  
        return getPointerAnalysisInfoTree(astNode);
      }
      default:
        break;
    }

    return {};
  }
};

class FunctionPointerHandler : public SymbolHandler
{
  enum class SubQuery
  {
    calls = 1,
    reads,
    writes,
    usageInFile
  };

public:
  FunctionPointerHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)
      << "Trying to find CppFunctionPointer with astnodeptr id:" << astNode.id;

    auto cppFuncPointer = query.queryEntityByHash<model::CppFunctionPointer>(
      astNode.mangledNameHash);

    // short name with signature
    ret.push_back(makeInfoNode( { }, "Name", cppFuncPointer.name));

    // qualified name
    ret.push_back(makeInfoNode( { }, "Qualified Name", cppFuncPointer.qualifiedName));

    // definition
    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(makeInfoNode( { }, "Defined", getFileloc(astNode), defAst));

    ret.push_back(makeInfoNode({}, "Type", cppFuncPointer.qualifiedType));

    ret.push_back(
      makeInfoQueryNode( { "Calls" }, (int)SubQuery::calls));
    ret.push_back(
      makeInfoQueryNode( { "Reads" }, (int)SubQuery::reads));
    ret.push_back(
      makeInfoQueryNode( { "Writes" }, (int)SubQuery::writes));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    auto cppFuncPointer = query.queryEntityByHash<model::CppFunctionPointer>(
      astNode.mangledNameHash);

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usage = [&, this](model::CppAstNode::AstType astType)
    {
      auto resultSet = db->query<model::AstCountGroupByFiles>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType));

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::usageInFile,
        {std::to_string((int)astType)});
    };

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usageInFile = [&, this]()
    {
      if (infoQuery.filters.size() != 2)
      {
        throw std::runtime_error("Not enough filter in InfoQuery object!");
      }

      auto astType = (model::CppAstNode::AstType)(stoul(infoQuery.filters[0]));
      auto fileId  = stoull(infoQuery.filters[1]);

      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType) &&
        CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        auto refInfo = createAstNodeInfo(ref);

        auto filename = baseName(ref.location.file->path);
        const auto& line = std::to_string(ref.location.range.start.line);
        const auto& column = std::to_string(ref.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            refInfo.astNodeSrcText, refInfo));
      }

      return ret;

    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::calls:       return usage(model::CppAstNode::AstType::Usage); break;
      case SubQuery::reads:       return usage(model::CppAstNode::AstType::Read); break;
      case SubQuery::writes:      return usage(model::CppAstNode::AstType::Write); break;
      case SubQuery::usageInFile: return usageInFile(); break;
      default:                    break;
    }

    return {};
  }
};

class TypeHandler : public SymbolHandler
{
  enum class SubQuery
  {
    aliases = 1,
    inheritance,
    friends,
    methods,
    members,
    inhMethods,
    inhMembers,
    usageParameter,
    usageReturnType,
    usageField,
    usageGlobal,
    usageLocal,
    usageInFile
  };
public:
  TypeHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)<< "Trying to find CppType with astnodeptr id:"<<astNode.id;

    auto cppType = query.queryEntityByHash<model::CppType>(
      astNode.mangledNameHash);

    if (cppType.isAbstract)
    {
      ret.push_back(makeInfoNode({}, "", "Abstract type"));
    }

    if (cppType.isPOD)
    {
      ret.push_back(makeInfoNode({}, "", "POD type"));
    }

    // short name with signature
    ret.push_back(makeInfoNode({}, "Name", cppType.name));

    // qualified name
    ret.push_back(
      makeInfoNode({}, "Qualified Name", cppType.qualifiedName));

    // definition
    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Defined", getFileloc(astNode), defAst));


    ret.push_back(
      makeInfoQueryNode({"Aliases"}, (int)SubQuery::aliases));
    ret.push_back(
      makeInfoQueryNode({"Inherits From"}, (int)SubQuery::inheritance, {"from"}));
    ret.push_back(
      makeInfoQueryNode({"Inherited By"}, (int)SubQuery::inheritance, {"by"}));
    ret.push_back(
      makeInfoQueryNode({"Friends"}, (int)SubQuery::friends));
    ret.push_back(
      makeInfoQueryNode({"Methods"}, (int)SubQuery::methods));
    ret.push_back(
      makeInfoQueryNode({"Members"}, (int)SubQuery::members));

    ret.push_back(
      makeInfoQueryNode({"Usage", "Global"}, (int)SubQuery::usageGlobal));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Local"}, (int)SubQuery::usageLocal));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Field"}, (int)SubQuery::usageField));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Parameter"}, (int)SubQuery::usageParameter));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Return Type"}, (int)SubQuery::usageReturnType));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    auto cppType = query.queryEntityByHash<model::CppType>(
      astNode.mangledNameHash);

    auto sortByLabel = [](const InfoNode& lhs, const InfoNode& rhs)
      {
        return lhs.label < rhs.label;
      };

    // Sorting members
    auto sortMember = [this](const model::CppMemberType& lhs, const model::CppMemberType& rhs)
      {
        std::string lhsAstValue = lhs.memberAstNode.load()->astValue;
        std::string rhsAstValue = rhs.memberAstNode.load()->astValue;
        
        std::transform(lhsAstValue.begin(), lhsAstValue.end(), lhsAstValue.begin(), ::tolower);
        std::transform(rhsAstValue.begin(), rhsAstValue.end(), rhsAstValue.begin(), ::tolower);
        
        auto left  = std::make_tuple(lhs.isStatic, lhsAstValue, lhs.visibility);
        auto right = std::make_tuple(rhs.isStatic, rhsAstValue, rhs.visibility);
        if (left < right)
          return true;
        else if (left == right)
        {
          auto lhsEntityResult = query.queryEntityByHash(lhs.typeHash);
          auto rhsEntityResult = query.queryEntityByHash(rhs.typeHash);

          auto& lhsEntity = *lhsEntityResult.begin();
          auto& rhsEntity = *rhsEntityResult.begin();

          return lhsEntity.name < rhsEntity.name;
        }
        return false;
      };

    auto aliases = [&, this]()
    {
      std::vector<InfoNode> ret;

      auto aliasSet = query.getTransitiveClosureOfRel(
        model::CppRelation::Kind::Alias, cppType.mangledNameHash);

      for (auto& alias : aliasSet)
      {
        auto entityResult = query.queryEntityByHash(alias);

        auto& entity = *entityResult.begin();
        auto node = db->load<model::CppAstNode>(entity.astNodeId.get());
        auto astInfo = createAstNodeInfo(*node);

        ret.push_back(makeInfoNode(
          {}, "", entity.qualifiedName, astInfo));
      }

      std::sort(ret.begin(), ret.end(), sortByLabel);

      return ret;
    };

    auto inheritance = [this, &astNode, &cppType, &infoQuery]()
    {
      if(infoQuery.filters.empty())
        throw std::runtime_error("Invalid InfoQuery.filters");

      bool from = false;
      if (infoQuery.filters.front() == "from")
        from = true;

      std::vector<InfoNode> ret;
      {
        typedef odb::query<model::CppInheritance> QInheritance;

        for (model::CppInheritance inheritance : db->query<
          model::CppInheritance>(
            (from ?
              QInheritance::derived :
              QInheritance::base) == cppType.mangledNameHash))
        {
          auto hash = from ? inheritance.base : inheritance.derived;

          auto t = query.queryEntityByHash(hash);
          auto& e = *t.begin();
          auto node = db->load<model::CppAstNode>(e.astNodeId.get());
          auto baseInfo = createAstNodeInfo(*node);

          auto visibility = visibilityToString(inheritance.visibility);

          ret.push_back(makeInfoNode(
            { visibility},
            "", e.qualifiedName, baseInfo));
        }
      }

      return ret;
    };

    auto friends = [this, &astNode, &cppType]()
    {
      std::vector<InfoNode> ret;
      typedef odb::query<model::CppFriendship> QFriendship;

      for (model::CppFriendship friendship : db->query<model::CppFriendship>(
        QFriendship::target == cppType.mangledNameHash))
      {
        try
        {
          auto friendRes = query.queryEntityByHash(
            friendship.theFriend);
          auto& friendType = *friendRes.begin();
          auto node = db->load<model::CppAstNode>(friendType.astNodeId.get());
          auto friendInfo = createAstNodeInfo(*node);

          ret.push_back(
            makeInfoNode( { }, "", friendType.qualifiedName,
              friendInfo));
        } catch (std::runtime_error&)
        {
          // Suppress runtime errors. It the friend does not exists then it's not
          // an error.
        }
      }
      return ret;
    };

    auto getMethods = [&, this](const model::CppType& cppType)
    {
      std::vector<InfoNode> ret;

      auto funtions_result = query.queryMembersByType(cppType,
        model::CppMemberType::Kind::Method);

      std::vector<model::CppMemberType> functions = { funtions_result.begin(),
        funtions_result.end() };

      ret.reserve(functions.size());

      std::sort(functions.begin(), functions.end(), sortMember);

      for (const auto& memFun : functions)
      {
        try
       {

          auto memFunInfo = createAstNodeInfo(*memFun.memberAstNode.load());
          SLog()<< "MemFun: " << memFun.memberAstNode->astValue;

          auto cppFunc = query.queryEntityByHash<model::CppFunction>(
            memFun.memberAstNode->mangledNameHash);
          std::string funcName = memFun.memberAstNode->astValue;

          std::string returnType = cppFunc.qualifiedType;
          if (returnType == "__*tor__none__") // don't show "void" return type for ctors
            // HACK: hide the "void" or new "__*tor_none__" return value.
            // But if the string is empty, the node does not get visible, cross-reference with InfoTree.js:42
            returnType = "[ignored]";


          std::vector<std::string> category;

          if (memFun.isStatic)
          {
            category.push_back("Static");
          } else {
            bool isImplicit = false;
            model::CppImplicit implicit;
            try
            {
              implicit = query.queryEntityByHash<model::CppImplicit>(
                memFun.memberAstNode->mangledNameHash);
              isImplicit = true; // If an implicit function is not found for the mangledNameHash, an exception is thrown
            } catch (const std::exception& ex) {
              // Consume, if no result is found, the function is not implicit...
            }

            if (isImplicit)
            {
              category.push_back("Compiler-generated");
              
              memFunInfo.astNodeSrcText = implicit.code;
              
              // Keep this synchronised with InfoTree.js!
              std::ostringstream os;
              os << "[Implicit]" << "|" << static_cast<int>(model::File::Type::CxxSource)
                << "|" << "compiler-generated-function.cpp" << "|" << core::FileParseStatus::FullyParsed;
              memFunInfo.documentation = os.str();
            }
          }

          category.push_back(visibilityToString(memFun.visibility));

          ret.push_back(
            makeInfoNode(std::move(category),
                         getSignature(cppFunc),
                         returnType,
                         memFunInfo));

        } catch (const std::exception& ex)
        {
          SLog(util::ERROR)<< "Exception caught: " << ex.what();
        }
      }

      return ret;
    };

    auto getMembers = [&, this](const model::CppType& cppType)
    {
      std::vector<InfoNode> ret;

      auto fields_result = query.queryMembersByType(cppType,
        model::CppMemberType::Kind::Field);

      std::vector<model::CppMemberType> fields = { fields_result.begin(),
        fields_result.end() };

      ret.reserve(fields.size());

      std::sort(fields.begin(), fields.end(), sortMember);

      for (const auto& member : fields)
      {
        try
        {
          auto memberInfo = createAstNodeInfo(*member.memberAstNode.load());

          auto var = query.queryEntityByHash<model::CppVariable>(
            member.memberAstNode->mangledNameHash);

          std::string typeStr = var.qualifiedType;

          std::vector<std::string> category;

          if (member.isStatic)
          {
            category.push_back("Static");
          }

          category.push_back(visibilityToString(member.visibility));

          ret.push_back(
            makeInfoNode(std::move(category), var.name, typeStr, memberInfo));

        } catch (const std::exception& ex)
        {
          SLog(util::ERROR)<< "Exception caught: " << ex.what();
        }
      }

      return ret;
    };

    auto methods = [this, &cppType, &getMethods](){
      auto ret = getMethods(cppType);
      ret.push_back(makeInfoQueryNode({"Inherited"}, (int)SubQuery::inhMethods));
      return ret;
    };

    auto members = [this, &cppType, &getMembers](){
      auto ret = getMembers(cppType);
      ret.push_back(makeInfoQueryNode({"Inherited"}, (int)SubQuery::inhMembers));
      return ret;
    };

    auto inhMethods = [this, &astNode, &cppType, &getMethods]()
    {
      std::vector<InfoNode> ret;
      typedef odb::query<model::CppInheritance> QInheritance;
      for (model::CppInheritance inheritance : db->query<
        model::CppInheritance>(
        QInheritance::derived == cppType.mangledNameHash))
      {
        auto t = query.queryEntityByHash<model::CppType>(inheritance.base);
        auto vec = getMethods(t);

        std::move(vec.begin(), vec.end(), std::back_inserter(ret));
      }
      return ret;
    };

    auto inhMembers = [this, &astNode, &cppType, &getMembers]()
    {
      std::vector<InfoNode> ret;
      typedef odb::query<model::CppInheritance> QInheritance;
      for (model::CppInheritance inheritance : db->query<
        model::CppInheritance>(
        QInheritance::derived == cppType.mangledNameHash))
      {
        auto t = query.queryEntityByHash<model::CppType>(inheritance.base);
        auto vec = getMembers(t);

        std::move(vec.begin(), vec.end(), std::back_inserter(ret));
      }
      return ret;
    };

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usage = [&, this](model::CppAstNode::AstType astType)
    {
      auto resultSet = db->query<model::AstCountGroupByFiles>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType));

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::usageInFile,
        {std::to_string((int)astType)});
    };

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usageInFile = [&, this]()
    {
      if (infoQuery.filters.size() != 2)
      {
        throw std::runtime_error("Not enough filter in InfoQuery object!");
      }

      auto astType = (model::CppAstNode::AstType)(stoul(infoQuery.filters[0]));
      auto fileId  = stoull(infoQuery.filters[1]);

      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType) &&
        CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        auto refInfo = createAstNodeInfo(ref);

        auto filename = baseName(ref.location.file->path);
        const auto& line = std::to_string(ref.location.range.start.line);
        const auto& column = std::to_string(ref.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            refInfo.astNodeSrcText, refInfo));
      }

      return ret;

    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::aliases:         return aliases(); break;
      case SubQuery::inheritance:     return inheritance(); break;
      case SubQuery::friends:         return friends(); break;
      case SubQuery::methods:         return methods(); break;
      case SubQuery::members:         return members(); break;
      case SubQuery::inhMethods:      return inhMethods(); break;
      case SubQuery::inhMembers:      return inhMembers(); break;
      case SubQuery::usageGlobal:     return usage(model::CppAstNode::AstType::GlobalTypeLoc); break;
      case SubQuery::usageLocal:      return usage(model::CppAstNode::AstType::LocalTypeLoc); break;
      case SubQuery::usageField:      return usage(model::CppAstNode::AstType::FieldTypeLoc); break;
      case SubQuery::usageParameter:  return usage(model::CppAstNode::AstType::ParameterTypeLoc); break;
      case SubQuery::usageReturnType: return usage(model::CppAstNode::AstType::ReturnTypeLoc); break;
      case SubQuery::usageInFile:     return usageInFile(); break;
      default: break;
    }

    return {};
  }

};

class MacroHandler : public SymbolHandler
{
  enum SubQuery
  {
    expansions = 1,
    expansionsInFile,
    undefinitions
  };
public:
  MacroHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    // definition
    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Defined", getFileloc(astNode), defAst));

    ret.push_back(makeInfoQueryNode({"Expansions"}, (int)SubQuery::expansions));
    ret.push_back(makeInfoQueryNode({"Undefinitions"}, (int)SubQuery::undefinitions));

    SLog(util::DEBUG)<< "Returning from getInfoTreeForMacro";

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    auto expansions =
      [this, &astNode]()
      {
        std::vector<InfoNode> ret;

        auto resultSet = db->query<model::AstCountGroupByFiles>(
          CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
          CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage));

        return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::expansionsInFile);
      };

    auto expansionsInFile = [this, &astNode, &infoQuery]()
    {
      if(infoQuery.filters.empty())
        throw std::runtime_error("No file ID in InfoQuery.filters");

      std::vector<InfoNode> ret;

      auto fileId = std::stoull(infoQuery.filters.front());
      for (auto usage :  query.sortQuery<model::CppAstNode>(
          CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
          CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage) &&
          CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        usage.location.file.load();
        auto line = std::to_string(usage.location.range.start.line);
        auto column = std::to_string(usage.location.range.start.column);
        auto loc = line + ':' + column;

        auto usageInfo = createAstNodeInfo(usage);

        typedef odb::query<model::CppMacroExpansion> MQuery;
        auto expansion = db->query<model::CppMacroExpansion>(
          MQuery::astNodePtr == usage.id);

        std::string expStr = (*expansion.begin()).expansion;

        ret.push_back(makeInfoNode(
            {}, loc, expStr, usageInfo));
      }

      return ret;
    };


    auto undefinitions = [this, &astNode, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      for (auto undef : query.sortQuery<model::CppAstNode>(
          CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
          CppOdbQuery::astAstType(model::CppAstNode::AstType::UnDefinition), sortByPosition))
      {
        if (!undef.location.file)
          continue;

        undef.location.file.load();
        auto filename = undef.location.file->filename;
        auto line = std::to_string(undef.location.range.start.line);
        auto column = std::to_string(undef.location.range.start.column);
        auto loc = line + ':' + column;

        auto undefInfo = createAstNodeInfo(undef);

        ret.push_back(makeInfoNode({filename}, loc, undef.astValue, undefInfo));
      }

      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::expansions: return expansions();
      case SubQuery::expansionsInFile: return expansionsInFile();
      case SubQuery::undefinitions: return undefinitions();
      default: break;
    }

    return {};
  }

  std::string getInfoBoxText(const model::CppAstNode& astNode) override
  {
    typedef odb::query<model::CppMacroExpansion> MQuery;

    auto result = db->query<model::CppMacroExpansion>(
      MQuery::astNodePtr == astNode.id);

    if (result.empty())
    {
      return {};
    }

    auto mExp = *result.begin();

    return mExp.expansion;
  }
};

class TypedefHandler : public SymbolHandler
{
  enum SubQuery
  {
    usageParameter = 1,
    usageReturnType,
    usageField,
    usageGlobal,
    usageLocal,
    usageInFile
  };
public:
  TypedefHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)<< "Trying to find Typedef with astnodeptr id:"<< astNode.id;

    auto cppTypedef = query.queryEntityByHash<model::CppTypedef>(
      astNode.mangledNameHash);

    // short name with signature
    ret.push_back(makeInfoNode({}, "Name", cppTypedef.name));

    // qualified name
    ret.push_back(
      makeInfoNode({}, "Qualified Name", cppTypedef.qualifiedName));

    SLog(util::DEBUG)<< "getting definition for " << astNode.id;
    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Defined", getFileloc(astNode), defAst));

    if (cppTypedef.typeHash != 0)
    {
      auto result = query.queryEntityByHash(cppTypedef.typeHash);

      auto& type = *result.begin();

      auto typeAst = db->load<model::CppAstNode>(type.astNodeId.get());
      auto returnInfo = createAstNodeInfo(*typeAst);

      ret.push_back(
        makeInfoNode({}, "Underlying type", cppTypedef.qualifiedType, returnInfo));
    }
    else
    {
      ret.push_back(
        makeInfoNode({}, "Underlying Type", cppTypedef.qualifiedType));
    }

    ret.push_back(
      makeInfoQueryNode({"Usage", "Global"}, (int)SubQuery::usageGlobal));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Local"}, (int)SubQuery::usageLocal));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Field"}, (int)SubQuery::usageField));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Parameter"}, (int)SubQuery::usageParameter));
    ret.push_back(
      makeInfoQueryNode({"Usage", "Return Type"}, (int)SubQuery::usageReturnType));


    SLog(util::DEBUG)<< "Returning from getInfoTreeForTypedef";

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usage = [&, this](model::CppAstNode::AstType astType)
    {
      auto resultSet = db->query<model::AstCountGroupByFiles>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType));

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::usageInFile,
        {std::to_string((int)astType)});
    };

    //TODO: refactor (it is copy/pasted in TypeHandler and VariableHandler and TypedefHandler)
    auto usageInFile = [&, this]()
    {
      if (infoQuery.filters.size() != 2)
      {
        throw std::runtime_error("Not enough filter in InfoQuery object!");
      }

      auto astType = (model::CppAstNode::AstType)(stoul(infoQuery.filters[0]));
      auto fileId  = stoull(infoQuery.filters[1]);

      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(astType) &&
        CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        auto refInfo = createAstNodeInfo(ref);

        auto filename = baseName(ref.location.file->path);
        const auto& line = std::to_string(ref.location.range.start.line);
        const auto& column = std::to_string(ref.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            refInfo.astNodeSrcText, refInfo));
      }

      return ret;

    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::usageGlobal:     return usage(model::CppAstNode::AstType::GlobalTypeLoc); break;
      case SubQuery::usageLocal:      return usage(model::CppAstNode::AstType::LocalTypeLoc); break;
      case SubQuery::usageField:      return usage(model::CppAstNode::AstType::FieldTypeLoc); break;
      case SubQuery::usageParameter:  return usage(model::CppAstNode::AstType::ParameterTypeLoc); break;
      case SubQuery::usageReturnType: return usage(model::CppAstNode::AstType::ReturnTypeLoc); break;
      case SubQuery::usageInFile:     return usageInFile(); break;
      default: break;
    }

    return {};
  }
};

class EnumHandler : public SymbolHandler
{
  enum class SubQuery
  {
    constants = 1,
  };

public:
  EnumHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)<< "Trying to find CppEnum with astnodeptr id:"<<astNode.id;

    auto cppEnum = query.queryEntityByHash<model::CppEnum>(
      astNode.mangledNameHash);

    ret.push_back(makeInfoNode({}, "Name", cppEnum.name));

    ret.push_back(
      makeInfoNode({}, "Qualified Name", cppEnum.qualifiedName));

    AstNodeInfo defAst = createAstNodeInfo(astNode);
    ret.push_back(
      makeInfoNode({}, "Defined", getFileloc(astNode), defAst));

    ret.push_back(
      makeInfoQueryNode({"Enum Constants"}, (int)SubQuery::constants));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    model::CppEnum cppEnum = query.queryEntityByHash<model::CppEnum>(
          astNode.mangledNameHash);

    auto constants = [this, &astNode, &cppEnum]()
    {
      std::vector<InfoNode> ret;
      ret.reserve(cppEnum.enumConstants.size());
      {
        for (const auto& constant : cppEnum.enumConstants)
        {
          constant.load();
          auto node = db->load<model::CppAstNode>(constant->astNodeId.get());
          auto astInfo = createAstNodeInfo(*node);

          ret.push_back(
            makeInfoNode({},
                         constant->name,
                         std::to_string(constant->value),
                         astInfo)
            );
        }
      }
      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::constants: return constants();
      default: break;
    }

    return {};

  }
};

class EnumConstantHandler : public SymbolHandler
{
  enum SubQuery
  {
    usage = 1,
    usageInFile
  };
public:
  EnumConstantHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
    {
      std::vector<InfoNode> ret;

      SLog(util::DEBUG)<< "Trying to find CppEnum with astnodeptr id:"<<astNode.id;

      auto cppEnumC = query.queryEntityByHash<model::CppEnumConstant>(
        astNode.mangledNameHash);

      ret.push_back(makeInfoNode({}, "Name", cppEnumC.name));

      ret.push_back(
        makeInfoNode({}, "Qualified Name", cppEnumC.qualifiedName));

      AstNodeInfo defAst = createAstNodeInfo(astNode);
      ret.push_back(
        makeInfoNode({}, "Defined", getFileloc(astNode), defAst));

      ret.push_back(
        makeInfoNode({}, "Value", std::to_string(cppEnumC.value)));

      ret.push_back(
        makeInfoQueryNode({"Usage"}, (int)SubQuery::usage));

      return ret;
    }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    auto usage = [this, &astNode]()
    {
      auto resultSet = db->query<model::AstCountGroupByFiles>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage));

      return makeHitCountQueryNodes(db, resultSet, (int)SubQuery::usageInFile);
    };

    auto usageInFile = [this, &astNode](model::FileId fileId)
    {
      std::vector<InfoNode> ret;

      for (auto ref : query.sortQuery<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
        CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage) &&
        CppOdbQuery::astFileId(fileId), sortByPosition))
      {
        auto refInfo = createAstNodeInfo(ref);

        auto filename = baseName(ref.location.file->path);
        const auto& line = std::to_string(ref.location.range.start.line);
        const auto& column = std::to_string(ref.location.range.start.column);

        ret.push_back(
          makeInfoNode( {}, line + ':' + column,
            refInfo.astNodeSrcText, refInfo));
      }

      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);

    switch (queryId)
    {
      case SubQuery::usage:
        return usage();
      case SubQuery::usageInFile:
      {
        if(infoQuery.filters.size() != 1)
          throw std::runtime_error("Invalid InfoQuery.filters");

        auto fileId = std::stoull(infoQuery.filters[0]);
        return usageInFile(fileId);
      }
      default:
        break;
    }

    return {};
  }

  std::string getInfoBoxText(const model::CppAstNode& astNode) override
  {
    auto cppEnumC = query.queryEntityByHash<model::CppEnumConstant>(
            astNode.mangledNameHash);

    return cppEnumC.name + " = " + std::to_string(cppEnumC.value);
  }
};

class DefaultHandler : public SymbolHandler
{
public:
  DefaultHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::CppAstNode& astNode) override
  {
    std::vector<InfoNode> ret;
    auto astInfo = createAstNodeInfo(astNode);

    ret.push_back(makeInfoNode({}, "ASTNode", ""));
    ret.push_back(
      makeInfoNode({}, astInfo.astNodeType, astNode.astValue));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::CppAstNode& astNode,
    const InfoQuery& query) override
  {
    return
    {};
  }

  std::string getInfoBoxText(const model::CppAstNode& astNode) override
  {
    return astNode.astValue;
  }
};

std::string SymbolHandler::getInfoBoxText(const model::CppAstNode& astNode)
{
  auto defNode = query.loadDefinitionOrDeclaration(astNode)[0];
  const auto& fileContent =
    defNode.location.file.load()->content.load()->content;
  return textRange(fileContent, defNode.location.range);
}

std::unique_ptr<SymbolHandler> SymbolHandler::getHandler(
  std::shared_ptr<odb::database> db, model::CppAstNode::SymbolType symbolType)
{
  using namespace model;
  switch (symbolType)
  {
    case CppAstNode::SymbolType::Variable:
      return std::unique_ptr<SymbolHandler> { new VariableHandler(db) };
    case CppAstNode::SymbolType::Function:
      return std::unique_ptr<SymbolHandler> { new FunctionHandler(db) };
    case CppAstNode::SymbolType::FunctionPtr:
      return std::unique_ptr<SymbolHandler> { new FunctionPointerHandler(db) };
    case CppAstNode::SymbolType::Type:
      return std::unique_ptr<SymbolHandler> { new TypeHandler(db) };
    case CppAstNode::SymbolType::Typedef:
      return std::unique_ptr<SymbolHandler> { new TypedefHandler(db) };
    case CppAstNode::SymbolType::Enum:
      return std::unique_ptr<SymbolHandler> { new EnumHandler(db) };
    case CppAstNode::SymbolType::EnumConstant:
      return std::unique_ptr<SymbolHandler> { new EnumConstantHandler(db) };
    case CppAstNode::SymbolType::Macro:
      return std::unique_ptr<SymbolHandler> { new MacroHandler(db) };
    default:
      break;
  }
  return std::unique_ptr<SymbolHandler> { new DefaultHandler(db) };
}
  
std::vector<InfoNode> SymbolHandler::getPointerAnalysisInfoTree(const model::CppAstNode& astNode)
{
}
  
std::string SymbolHandler::getPointerAnalysisDiagram(const model::CppAstNode& astNode)
{
}
  
std::vector<model::CppAstNode> SymbolHandler::getCallers(const model::HashType mangledNameHash,
    const model::FileId& fileId){  
    
  using namespace model;

  std::vector<model::CppAstNode> callers;
  callers.reserve(512);

  // direct callers
  for (auto caller : db->query<model::CppAstNode>(
    CppOdbQuery::astMangledNameHash(mangledNameHash) &&
    CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage) &&
    CppOdbQuery::astFileId(fileId)))
  {
    callers.push_back(std::move(caller));
  }

  // virtual callers
  {
    auto overriddens = query.reverseTransitiveClosureOfRel(
      CppRelation::Kind::Override, mangledNameHash);

    overriddens.insert(mangledNameHash);

    for (auto overridden : overriddens)
    {
      for (auto caller : db->query<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(overridden) &&
        CppOdbQuery::astAstType(model::CppAstNode::AstType::VirtualCall) &&
        CppOdbQuery::astFileId(fileId)))
      {
        callers.push_back(std::move(caller));
      }
    }
  }

  // funcptr callers
  {
    auto fptrCallers = query.reverseTransitiveClosureOfRel(
      CppRelation::Kind::Assign, mangledNameHash);

    for (auto fptrCaller : fptrCallers)
    {
      for (auto caller : db->query<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(fptrCaller) &&
        CppOdbQuery::astAstType(model::CppAstNode::AstType::Usage) &&
        CppOdbQuery::astFileId(fileId)))
      {
        callers.push_back(std::move(caller));
      }
    }
  }

  std::sort(callers.begin(), callers.end(), sortByPosition);

  return callers;
}

std::vector<InfoNode> CppFileHandler::getInfoTreeForFile(
  const model::FileId fid)
{
  using namespace model;
  std::vector<InfoNode> ret;

  SLog(util::DEBUG)<< "Trying to find File with file id:" << fid;

  File file;
  db->load<File>(fid, file);

  // filename
  ret.push_back(makeInfoNode({}, "Name", file.filename));

  // path
  ret.push_back(makeInfoNode({}, "Path", file.path));

  // content
  ret.push_back(makeInfoNode({}, "Content", "C/C++ source"));

  ret.push_back(makeInfoQueryNode({"Includes"}, (int)SubQuery::includes));
  ret.push_back(makeInfoQueryNode({"Macros"}, (int)SubQuery::macros));
  ret.push_back(makeInfoQueryNode({"Types"}, (int)SubQuery::types));
  ret.push_back(makeInfoQueryNode({"Functions"}, (int)SubQuery::functions));
  //ret.push_back(makeInfoQueryNode({"Globals"}, (int)SubQuery::globals));

  return ret;
}

std::vector<InfoNode> CppFileHandler::getSubInfoTreeForFile(
  const model::FileId& fid, const InfoQuery& infoQuery)
{
  using namespace model;

  auto includes = [this, fid]()
  {
    std::vector<InfoNode> ret;

    auto includes = query.sortQuery<CppAstNode>(CppOdbQuery::astFileId(fid)
      && CppOdbQuery::astSymbolType(CppAstNode::SymbolType::File), sortByPosition);

    NodeFilterByPos filter;
    for (auto includedFile : includes)
    {
      if (filter.shouldSkip(includedFile))
      {
        continue;
      }
      
      auto includedInfo = createAstNodeInfo(includedFile);

      FileId includedId = stoull(includedFile.astValue);

      File included;
      db->load<File>(includedId, included);

      ret.push_back(makeInfoNode({}, "", included.filename, includedInfo));
    }


    return ret;
  };

  typedef odb::query<CppAstNode> AstQuery;

  auto collectAstNodes = [this, &fid](
    AstQuery astQuery, const std::vector<std::string>& category)
  {
    std::vector<InfoNode> ret;

    auto nodes = db->query<CppAstNode>(
      (CppOdbQuery::astFileId(fid) && astQuery) + "ORDER BY" + AstQuery::astValue);

    NodeFilterByPos filter;
    for (auto node : nodes)
    {
      // Filter the same nodes (it is possible because of templates)
      if (filter.shouldSkip(node))
      {
        continue;
      }

      auto infoNode = createAstNodeInfo(node);
      std::string qualifiedName;
      try
      {
        qualifiedName = query.getQualifiedName(node);
      }catch(...)
      {
        qualifiedName = node.astValue;
      }

      std::vector<std::string> namespaces;
      std::size_t pos = qualifiedName.rfind("::");
      if (pos != std::string::npos)
        namespaces.push_back(qualifiedName.substr(0, pos));

      auto cat = category;
      std::move(namespaces.begin(), namespaces.end(), back_inserter(cat));

      std::string astValue;
      if(node.symbolType == model::CppAstNode::SymbolType::Function)
      {
        auto cppFunction = query.queryEntityByHash<model::CppFunction>(
              node.mangledNameHash);
        astValue = cppFunction.name;
      }else
      {
        astValue = node.astValue;
      }
      ret.push_back(makeInfoNode(cat, "", astValue, infoNode));
    }

    return ret;
  };

  auto macros = [=]()
  {
    auto query = CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Macro) &&
                 CppOdbQuery::astAstType(CppAstNode::AstType::Definition);

    return collectAstNodes(query, {});
  };

  auto types = [=]()
  {
    auto query =
      CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Type) &&
      (CppOdbQuery::astAstType(CppAstNode::AstType::Definition) ||
       CppOdbQuery::astAstType(CppAstNode::AstType::Declaration));

    return collectAstNodes(query, {});
  };

  auto functions = [=]()
  {
    auto query =
      CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Function) &&
      (CppOdbQuery::astAstType(CppAstNode::AstType::Definition) ||
       CppOdbQuery::astAstType(CppAstNode::AstType::Declaration));

    return collectAstNodes(query, {});
  };

//  auto globals = [=]()
//  {
//    std::vector<InfoNode> ret;
//
//
//    return ret;
//  };

  auto queryId = static_cast<SubQuery>(infoQuery.queryId);

  switch (queryId)
  {
    case SubQuery::includes: return includes();
    case SubQuery::macros: return macros();
    case SubQuery::types: return types();
    case SubQuery::functions: return functions();
    //case SubQuery::globals: return globals();
    default:
      break;
  }

  return {};
}

} // language
} // service
} // cc
