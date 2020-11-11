#include <regex>

#include <service/pythonservice.h>

#include <util/util.h>

#include <model/pythonclass-odb.hxx>
#include <model/pythondocumentation.h>
#include <model/pythondocumentation-odb.hxx>
#include <model/pythonentity-odb.hxx>
#include <model/pythonfunction.h>
#include <model/pythonfunction-odb.hxx>
#include <model/pythonimport.h>
#include <model/pythonimport-odb.hxx>
#include <model/pythoninheritance.h>
#include <model/pythoninheritance-odb.hxx>
#include <model/pythontype.h>
#include <model/pythontype-odb.hxx>
#include <model/pythonvariable.h>
#include <model/pythonvariable-odb.hxx>
#include <model/pythonentity-odb.hxx>

namespace
{
    typedef odb::query<cc::model::PythonAstNode> AstQuery;
    typedef odb::result<cc::model::PythonAstNode> AstResult;
    typedef odb::query<cc::model::PythonEntity> EntityQuery;
    typedef odb::result<cc::model::PythonEntity> EntityResult;
    typedef odb::query<cc::model::PythonVariable> VarQuery;
    typedef odb::result<cc::model::PythonVariable> VarResult;
    typedef odb::query<cc::model::PythonFunction> FuncQuery;
    typedef odb::result<cc::model::PythonFunction> FuncResult;
    typedef odb::query<cc::model::PythonClass> ClassQuery;
    typedef odb::result<cc::model::PythonClass> ClassResult;
    typedef odb::query<cc::model::PythonClassMember> ClassMemQuery;
    typedef odb::result<cc::model::PythonClassMember> ClassMemResult;
    typedef odb::query<cc::model::PythonImport> ModImpQuery;
    typedef odb::result<cc::model::PythonImport> ModImpResult;
    typedef odb::query<cc::model::PythonInheritance> InhQuery;
    typedef odb::result<cc::model::PythonInheritance> InhResult;
    typedef odb::query<cc::model::PythonType> TypeQuery;
    typedef odb::result<cc::model::PythonType> TypeResult;
    typedef odb::query<cc::model::PythonDocumentation> DocQuery;
    typedef odb::result<cc::model::PythonDocumentation> DocResult;
    typedef odb::query<cc::model::File> FileQuery;
    typedef odb::result<cc::model::File> FileResult;

    struct CreateAstNodeInfo
    {
        typedef std::map<cc::model::PythonAstNodeId, std::string> VisibilityMap;

        CreateAstNodeInfo(const VisibilityMap& visibilities_ = {}) : _visibilities(visibilities_)
        {
        }

        cc::service::language::AstNodeInfo operator()(const cc::model::PythonAstNode& astNode_)
        {
            cc::service::language::AstNodeInfo ret;

            ret.__set_id(std::to_string(astNode_.id));
            ret.__set_entityHash(cc::model::createAstNodeInfoEntityHash(astNode_));
            ret.__set_astNodeType(cc::model::astTypeToString(astNode_.astType));
            ret.__set_symbolType(cc::model::symbolTypeToString(astNode_.symbolType));
            ret.__set_astNodeValue(astNode_.astValue);

            VisibilityMap::const_iterator it = _visibilities.find(astNode_.id);
            if (it != _visibilities.end()){
                ret.__set_tags({it->second});
            }

            ret.range.range.startpos.line = astNode_.location.range.start.line;
            ret.range.range.startpos.column = astNode_.location.range.start.column;
            ret.range.range.endpos.line = astNode_.location.range.end.line;
            ret.range.range.endpos.column = astNode_.location.range.end.column;

            if (astNode_.location.file){
                ret.range.file = std::to_string(astNode_.location.file.object_id());
            }

            return ret;
        }

        const VisibilityMap& _visibilities;
    };
}

namespace cc {
namespace service {
namespace language {

PythonServiceHandler::PythonServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_)
        :   _db(db_),
            _transaction(db_),
            _datadir(datadir_),
            _context(context_)
{
}

void PythonServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
    return_.push_back("PY");
    return_.push_back("Dir");
}

void PythonServiceHandler::getAstNodeInfo(
    AstNodeInfo& return_,
    const core::AstNodeId& astNodeId_)
{
    return_ = _transaction([this, &astNodeId_](){
        return CreateAstNodeInfo()(queryPythonAstNode(astNodeId_));
    });
}

void PythonServiceHandler::getAstNodeInfoByPosition(
    AstNodeInfo& return_,
    const core::FilePosition& fpos_)
{
    _transaction([&, this](){
        //--- Query nodes at the given position ---//

        AstResult nodes = _db->query<model::PythonAstNode>(
            AstQuery::location.file == std::stoull(fpos_.file) &&
            // StartPos <= Pos
            ((AstQuery::location.range.start.line == fpos_.pos.line &&
              AstQuery::location.range.start.column <= fpos_.pos.column) ||
             AstQuery::location.range.start.line < fpos_.pos.line) &&
            // Pos < EndPos
            ((AstQuery::location.range.end.line == fpos_.pos.line &&
              AstQuery::location.range.end.column > fpos_.pos.column) ||
             AstQuery::location.range.end.line > fpos_.pos.line));

        //--- Select innermost clickable node ---//

        model::Range minRange(model::Position(0, 0), model::Position());
        model::PythonAstNode min;

        for (const model::PythonAstNode& node : nodes)
        {
            if (node.location.range < minRange)
            {
                min = node;
                minRange = node.location.range;
            }
        }

        return_ = _transaction([this, &min](){
            return CreateAstNodeInfo(getVisibilities({min}))(min);
        });
    });
}

void PythonServiceHandler::getSourceText(
    std::string& return_,
    const core::AstNodeId& astNodeId_)
{
    return_ = _transaction([this, &astNodeId_](){
        model::PythonAstNode astNode = queryPythonAstNode(astNodeId_);

        if (astNode.location.file){
            return cc::util::textRange(
                astNode.location.file.load()->content.load()->content,
                astNode.location.range.start.line,
                astNode.location.range.start.column,
                astNode.location.range.end.line,
                astNode.location.range.end.column);
        }

        return std::string();
    });
}

void PythonServiceHandler::getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_)
{
    PythonEntity entity = queryPythonEntityByAstNode(astNodeId_);

    DocResult doc = _db->query<model::PythonDocumentation>(
            DocQuery::documented == entity.id);

    if (doc.empty()){
        return std::string();
    }

    return_ = doc.begin()->documentation;
}

void PythonServiceHandler::getProperties(
    std::map<std::string, std::string>& return_,
    const core::AstNodeId& astNodeId_)
{
    _transaction([&, this](){
        model::PythonAstNode node = queryPythonAstNode(astNodeId_);

        switch (node.symbolType)
        {
            case model::PythonAstNode::SymbolType::Variable:
            {
                VarResult variables = _db->query<model::PythonVariable>(
                        VarQuery::astNodeId == node.id);
                model::PythonVariable variable = *variables.begin();

                return_["Name"] = variable.name;
                return_["Qualified name"] = variable.qualifiedName;
                break;
            }

            case model::PythonAstNode::SymbolType::Function:
            {
                FuncResult functions = _db->query<model::PythonFunction>(
                        FuncQuery::astNodeId == node.id);
                model::PythonFunction function = *functions.begin();

                return_["Name"] = function.qualifiedName.substr(
                        function.qualifiedName.find_last_of('.') + 1);
                return_["Qualified name"] = function.qualifiedName;
                return_["Signature"] = function.name;       // TODO

                break;
            }

            case model::PythonAstNode::SymbolType::Class:
            {
                ClassResult classes = _db->query<model::PythonClass>(
                        ClassQuery::astNodeId == node.id);
                model::PythonClass cl = *classes.begin();

                return_["Name"] = cl.name;
                return_["Qualified name"] = cl.qualifiedName;

                break;
            }

            case model::PythonAstNode::SymbolType::Module:
            {
                ModImpResult modules = _db->query<model::PythonImport>(
                        ModImpQuery::astNodeId == node.id);
                model::PythonImport module = *modules.begin();

                return_["From"] = module.imported->filename;
                //return_["To"] = module.importer->filename;
                //return_["Symbol"] = imported symbol

                break;
            }
        }
    });
}

void PythonServiceHandler::getDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_)
{

}

void PythonServiceHandler::getDiagram(
        std::string& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t diagramId_)
{

}

void PythonServiceHandler::getDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{

}

void PythonServiceHandler::getFileDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_)
{

}

void PythonServiceHandler::getFileDiagram(
        std::string& return_,
        const core::FileId& fileId_,
        const int32_t diagramId_)
{

}

void PythonServiceHandler::getFileDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{

}

void PythonServiceHandler::getReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId)
{
    model::PythonAstNode node = queryPythonAstNode(astNodeId);

    return_["Declaration"]               = DECLARATION;
    return_["Usage"]                     = USAGE;

    switch (node.symbolType){
        case model::PythonAstNode::SymbolType::Variable:
            return_["Type"]                  = TYPE;
            break;

        case model::PythonAstNode::SymbolType::Function:
            return_["This calls"]            = THIS_CALLS;
            return_["Callee"]                = CALLEE;
            return_["Caller"]                = CALLER;
            return_["Parameters"]            = PARAMETER;
            return_["Local variables"]       = LOCAL_VAR;
            return_["Return type"]           = RETURN_TYPE;
            break;

        case model::PythonAstNode::SymbolType::Class:
            return_["Inherits from"]         = INHERIT_FROM;
            return_["Inherited by"]          = INHERIT_BY;
            return_["Data member"]           = DATA_MEMBER;
            return_["Method"]                = METHOD;
            return_["Nested class"]          = NESTED_CLASS;
            break;

        case model::PythonAstNode::SymbolType::Module:
            break;
    }
}

void PythonServiceHandler::getReferences(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::vector<std::string>& /*tags_*/)
{
    std::vector<model::PythonAstNode> nodes;
    model::PythonAstNode node;

    return _transaction([&, this](){
        switch (referenceId_)
        {
            case DECLARATION:
                nodes = queryPythonAstNodes(
                        astNodeId_,
                        AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                break;

            case USAGE:
                nodes = queryPythonAstNodes(astNodeId_);
                break;

            case THIS_CALLS:
                nodes = queryCalls(astNodeId_);
                break;

            case CALLS_OF_THIS:
                nodes = queryPythonAstNodes(
                        astNodeId_,
                        AstQuery::astType == model::PythonAstNode::AstType::Usage);
                break;

            case CALLEE:
                for (const model::PythonAstNode& call : queryCalls(astNodeId_))
                {
                    core::AstNodeId astNodeId = std::to_string(call.id);
                    std::vector<model::PythonAstNode> defs = queryDeclarations(astNodeId);
                    nodes.insert(nodes.end(), defs.begin(), defs.end());
                }

                std::sort(nodes.begin(), nodes.end());
                nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

                break;

            case CALLER:
                for (const model::PythonAstNode& astNode : queryPythonAstNodes(
                        astNodeId_,
                        AstQuery::astType == model::PythonAstNode::AstType::Usage))
                {
                    const model::Position& start = astNode.location.range.start;
                    const model::Position& end   = astNode.location.range.end;

                    AstResult result = _db->query<model::PythonAstNode>(
                            AstQuery::astType    == model::PythonAstNode::AstType::Declaration &&
                            AstQuery::symbolType == model::PythonAstNode::SymbolType::Function &&
                            // Same file
                            AstQuery::location.file == astNode.location.file.object_id() &&
                            // StartPos >= Pos
                            ((AstQuery::location.range.start.line == start.line &&
                              AstQuery::location.range.start.column <= start.column) ||
                             AstQuery::location.range.start.line < start.line) &&
                            // Pos > EndPos
                            ((AstQuery::location.range.end.line == end.line &&
                              AstQuery::location.range.end.column > end.column) ||
                             AstQuery::location.range.end.line > end.line));

                    nodes.insert(nodes.end(), result.begin(), result.end());
                }

                std::sort(nodes.begin(), nodes.end());
                nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

                break;

            case PARAMETER:
            {
                node = queryPythonAstNode(astNodeId_);

                FuncResult functions = _db->query<model::PythonFunction>(
                        FuncQuery::astNodeId == node.id);
                model::PythonFunction function = *functions.begin();

                for (auto var : function.parameters){
                    nodes.push_back(queryPythonAstNode(
                            std::to_string(var.load()->astNodeId)));
                }

                break;
            }

            case LOCAL_VAR:
            {
                node = queryPythonAstNode(astNodeId_);

                FuncResult functions = _db->query<model::PythonFunction>(
                        FuncQuery::astNodeId == node.id);
                model::PythonFunction function = *functions.begin();

                for (auto var : function.locals){
                    nodes.push_back(queryPythonAstNode(
                            std::to_string(var.load()->astNodeId)));
                }

                break;
            }

            case RETURN_TYPE:
            {
                node = queryPythonAstNode(astNodeId_);

                FuncResult functions = _db->query<model::PythonFunction>(
                        FuncQuery::astNodeId == node.id);
                model::PythonFunction function = *functions.begin();

                std::vector<model::PythonClass> types = queryTypes(function);

                for (const model::PythonClass& cl : types)
                {
                    std::vector<model::PythonAstNode> defs =
                            queryDeclarations(std::to_string(cl.astNodeId));
                    nodes.insert(nodes.end(), defs.begin(), defs.end());
                }

                break;
            }

            case TYPE:
            {
                node = queryPythonAstNode(astNodeId_);

                VarResult varNodes = _db->query<cc::model::PythonVariable>(
                        VarQuery::astNodeId == node.id);

                const model::PythonVariable& variable = *varNodes.begin();

                std::vector<model::PythonClass> types = queryTypes(variable);

                for (const model::PythonClass& cl : types)
                {
                    std::vector<model::PythonAstNode> defs =
                            queryDeclarations(std::to_string(cl.astNodeId));
                    nodes.insert(nodes.end(), defs.begin(), defs.end());
                }

                break;
            }

            case INHERIT_FROM:
                node = queryPythonAstNode(astNodeId_);

                for (const model::PythonInheritance& inh :
                        _db->query<model::PythonInheritance>(
                                InhQuery::derived == node.id))
                {
                    model::PythonEntity cl = _db->query_value<model::PythonEntity>(
                            EntityQuery::id == inh.base);
                    AstResult result = _db->query<model::PythonAstNode>(
                            AstQuery::id == cl.astNodeId &&
                            AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                    nodes.insert(nodes.end(), result.begin(), result.end());
                }

                break;

            case INHERIT_BY:
                node = queryPythonAstNode(astNodeId_);

                for (const model::PythonInheritance& inh :
                        _db->query<model::PythonInheritance>(
                                InhQuery::base == node.id ))
                {
                    model::PythonEntity cl = _db->query_value<model::PythonEntity>(
                            EntityQuery::id == inh.base);
                    AstResult result = _db->query<model::PythonAstNode>(
                            AstQuery::id == cl.astNodeId &&
                            AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                    nodes.insert(nodes.end(), result.begin(), result.end());
                }

                break;

            case DATA_MEMBER:
            {
                node = queryPythonAstNode(astNodeId_);

                for (const model::PythonClassMember& mem : _db->query<model::PythonClassMember>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Attribute))
                {
                    for (const model::PythonVariable& var : _db->query<model::PythonVariable>(
                            VarQuery::id == mem.memberId))
                    {
                        model::PythonAstNode astNode = queryPythonAstNode(std::to_string(var.astNodeId));
                        if (astNode.location.range.end.line != model::Position::npos){
                            nodes.push_back(astNode);
                        }
                    }
                }

                break;
            }

            case METHOD:
            {
                node = queryPythonAstNode(astNodeId_);

                for (const model::PythonClassMember& mem : _db->query<model::PythonClassMember>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Method))
                {
                    for (const model::PythonFunction& func : _db->query<model::PythonFunction>(
                            FuncQuery::id == mem.memberId))
                    {
                        nodes.push_back(queryPythonAstNode(std::to_string(func.astNodeId)));
                    }
                }

                break;
            }

            case NESTED_CLASS:
            {
                node = queryPythonAstNode(astNodeId_);

                for (const model::PythonClassMember& mem : _db->query<model::PythonClassMember>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Class))
                {
                    for (const model::PythonClass& cl : _db->query<model::PythonClass>(
                            ClassQuery::id == mem.memberId))
                    {
                        nodes.push_back(queryPythonAstNode(std::to_string(cl.astNodeId)));
                    }
                }

                break;
            }
        }

        std::sort(nodes.begin(), nodes.end(), compareByValue);

        return_.reserve(nodes.size());
        _transaction([this, &return_, &nodes](){
            std::transform(
                    nodes.begin(), nodes.end(),
                    std::back_inserter(return_),
                    CreateAstNodeInfo(getVisibilities(nodes)));
        });
    });
}

std::int32_t PythonServiceHandler::getReferenceCount(
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_)
{
    model::PythonAstNode node = queryPythonAstNode(astNodeId_);

    return _transaction([&, this]() -> std::int32_t {
        switch (referenceId_)
        {
            case DECLARATION:
                return queryPythonAstNodeCount(astNodeId_,
                    AstQuery::astType == model::PythonAstNode::AstType::Declaration);

            case USAGE:
                return queryPythonAstNodeCount(astNodeId_);

            case THIS_CALLS:
                return queryCallsCount(astNodeId_);

            case CALLS_OF_THIS:
                return queryPythonAstNodeCount(astNodeId_,
                    AstQuery::astType == model::PythonAstNode::AstType::Usage);

            case CALLEE:
            {
                std::int32_t count = 0;

                std::set<std::uint64_t> astNodeIds;
                for (const model::PythonAstNode& call : queryCalls(astNodeId_))
                {
                    model::PythonAstNode node = queryPythonAstNode(std::to_string(call.id));
                    astNodeIds.insert(node.id);
                }

                if (!astNodeIds.empty()){
                    count = _db->query_value<model::PythonAstCount>(
                            AstQuery::id.in_range(
                                    astNodeIds.begin(), astNodeIds.end()) &&
                            AstQuery::astType == model::PythonAstNode::AstType::Declaration &&
                            AstQuery::location.range.end.line != model::Position::npos).count;
                }

                return count;
            }

            case CALLER:
            {
                std::vector<AstNodeInfo> references;
                getReferences(references, astNodeId_, CALLER, {});
                return references.size();
            }

            case PARAMETER:
                return _db->query_value<model::PythonFunctionParamCount>(
                        FuncQuery::astNodeId == node.id).count;

            case LOCAL_VAR:
                return _db->query_value<model::PythonFunctionLocalCount>(
                        FuncQuery::astNodeId == node.id).count;

            case RETURN_TYPE:
            {
                node = queryPythonAstNode(astNodeId_);

                FuncResult functions = _db->query<cc::model::PythonFunction>(
                        FuncQuery::astNodeId == node.id);

                const model::PythonFunction& function = *functions.begin();

                std::vector<model::PythonClass> types = queryTypes(function);

                std::int32_t result = 0;

                for(const model::PythonClass& cl : types){
                    result += _db->query_value<model::PythonClassCount>(
                            ClassQuery::id == function.id).count;
                }

                return result;
            }

            case TYPE:
            {
                node = queryPythonAstNode(astNodeId_);

                VarResult varNodes = _db->query<cc::model::PythonVariable>(
                        VarQuery::astNodeId == node.id);

                const model::PythonVariable& variable = *varNodes.begin();

                std::vector<model::PythonClass> types = queryTypes(variable);

                std::int32_t result = 0;

                for(const model::PythonClass& cl : types){
                    result += _db->query_value<model::PythonClassCount>(
                            ClassQuery::id == cl.id).count;
                }

                return result;
            }

            case INHERIT_FROM:
                return _db->query_value<model::PythonInheritanceCount>(
                        InhQuery::derived == node.id).count;

            case INHERIT_BY:
                return _db->query_value<model::PythonInheritanceCount>(
                        InhQuery::base == node.id).count;

            case DATA_MEMBER:
                return _db->query_value<model::PythonClassMemberCount>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Attribute).count;

            case METHOD:
                return _db->query_value<model::PythonClassMemberCount>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Method).count;

            case NESTED_CLASS:
                return _db->query_value<model::PythonClassMemberCount>(
                        ClassMemQuery::astNodeId == node.id &&
                        ClassMemQuery::kind == model::PythonClassMember::Kind::Class).count;

            default:
                return 0;
        }
    });
}

void PythonServiceHandler::getReferencesInFile(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const core::FileId& fileId_,
        const std::vector<std::string>& tags_)
{
    // TODO
}

void PythonServiceHandler::getReferencesPage(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::int32_t pageSize_,
        const std::int32_t pageNo_)
{
    // TODO
}

void PythonServiceHandler::getFileReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_)
{
    return_["Imports"]      = IMPORTS;
    return_["Classes"]      = CLASSES;
    return_["Functions"]    = FUNCTIONS;
    return_["Variables"]    = VARIABLES;
}

void PythonServiceHandler::getFileReferences(
    std::vector<AstNodeInfo>& return_,
    const core::FileId& fileId_,
    const std::int32_t referenceId_)
{
    std::vector<model::PythonAstNode> nodes;

    _transaction([&, this](){
        switch (referenceId_)
        {
            case CLASSES:
                nodes = queryPythonAstNodesInFile(fileId_,
                   AstQuery::symbolType == model::PythonAstNode::SymbolType::Class);
                break;

            case VARIABLES:
                nodes = queryPythonAstNodesInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Variable &&
                    AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                break;

            case FUNCTIONS:
                nodes = queryPythonAstNodesInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Function &&
                    (AstQuery::astType == model::PythonAstNode::AstType::Declaration));
                break;

            case IMPORTS:
                nodes = queryPythonAstNodesInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Module);
                break;
        }

        std::sort(nodes.begin(), nodes.end(), compareByValue);

        return_.reserve(nodes.size());
        _transaction([this, &return_, &nodes](){
            std::transform(
                    nodes.begin(), nodes.end(),
                    std::back_inserter(return_),
                    CreateAstNodeInfo(getVisibilities(nodes)));
        });
    });
}

std::int32_t PythonServiceHandler::getFileReferenceCount(
    const core::FileId& fileId_,
    const std::int32_t referenceId_)
{
    return _transaction([&, this]() -> std::int32_t {
        switch (referenceId_)
        {
            case CLASSES:
                return queryPythonAstNodeCountInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Class);
                break;

            case VARIABLES:
                return queryPythonAstNodeCountInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Variable &&
                    AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                break;

            case FUNCTIONS:
                return queryPythonAstNodeCountInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Function &&
                    AstQuery::astType == model::PythonAstNode::AstType::Declaration);
                break;

            case IMPORTS:
                return queryPythonAstNodeCountInFile(fileId_,
                    AstQuery::symbolType == model::PythonAstNode::SymbolType::Module);
                break;

            default:
                return 0;
        }
    });
}

void PythonServiceHandler::getSyntaxHighlight(
    std::vector<SyntaxHighlight>& return_,
    const core::FileRange& range_)
{
    std::vector<std::string> content;

    _transaction([&, this]() {

        //--- Load the file content and break it into lines ---//

        model::FilePtr file = _db->query_one<model::File>(
                FileQuery::id == std::stoull(range_.file));

        if (!file->content.load()){
            return;
        }

        std::istringstream s(file->content->content);
        std::string line;
        while (std::getline(s, line)){
            content.push_back(line);
        }

        //--- Iterate over AST node elements ---//

        for (const model::PythonAstNode& node : _db->query<model::PythonAstNode>(
                AstQuery::location.file == std::stoull(range_.file) &&
                AstQuery::location.range.start.line >= range_.range.startpos.line &&
                AstQuery::location.range.end.line < range_.range.endpos.line &&
                AstQuery::location.range.end.line != model::Position::npos))
        {
            if (node.astValue.empty()){
                continue;
            }

            // Regular expression to find element position
            std::string reg = "\\b" + node.astValue + "\\b";

            for (std::size_t i = node.location.range.start.line - 1;
                 i < node.location.range.end.line && i < content.size();
                 ++i)
            {
                std::regex words_regex(reg);
                auto words_begin = std::sregex_iterator(
                        content[i].begin(), content[i].end(),
                        words_regex);
                auto words_end = std::sregex_iterator();

                for (std::sregex_iterator ri = words_begin; ri != words_end; ++ri)
                {
                    SyntaxHighlight syntax;
                    syntax.range.startpos.line = i + 1;
                    syntax.range.startpos.column = ri->position() + 1;
                    syntax.range.endpos.line = i + 1;
                    syntax.range.endpos.column =
                            syntax.range.startpos.column + node.astValue.length();

                    std::string symbolClass =
                            "cm-" + model::symbolTypeToString(node.symbolType);
                    syntax.className = symbolClass + " " +
                                       symbolClass + "-" + model::astTypeToString(node.astType);

                    return_.push_back(std::move(syntax));
                }
            }
        }
    });
}

bool PythonServiceHandler::compareByPosition(
    const model::PythonAstNode& lhs,
    const model::PythonAstNode& rhs)
{
    return lhs.location.range.start < rhs.location.range.start;
}

bool PythonServiceHandler::compareByValue(
    const model::PythonAstNode& lhs,
    const model::PythonAstNode& rhs)
{
    return lhs.astValue < rhs.astValue;
}

model::PythonAstNode PythonServiceHandler::queryPythonAstNode(const core::AstNodeId& astNodeId_)
{
    return _transaction([&, this](){
        model::PythonAstNode node;

        if (!_db->find(std::stoull(astNodeId_), node))
        {
            core::InvalidId ex;
            ex.__set_msg("Invalid PythonAstNode ID");
            ex.__set_nodeid(astNodeId_);
            throw ex;
        }

        return node;
    });
}

std::vector<model::PythonAstNode> PythonServiceHandler::queryPythonAstNodes(
    const core::AstNodeId& astNodeId_,
    const odb::query<model::PythonAstNode>& query_)
{
    model::PythonAstNode node = queryPythonAstNode(astNodeId_);

    AstResult result = _db->query<model::PythonAstNode>(
            AstQuery::id == node.id &&
            AstQuery::location.range.end.line != model::Position::npos &&
            query_);

    return std::vector<model::PythonAstNode>(result.begin(), result.end());
}

std::vector<model::PythonAstNode> PythonServiceHandler::queryPythonAstNodesInFile(
    const core::FileId& fileId_,
    const odb::query<model::PythonAstNode>& query_)
{
    AstResult result = _db->query<model::PythonAstNode>(
            AstQuery::location.file == std::stoull(fileId_) && query_);

    return std::vector<model::PythonAstNode>(result.begin(), result.end());
}

std::uint32_t PythonServiceHandler::queryPythonAstNodeCountInFile(
    const core::FileId& fileId_,
    const odb::query<model::PythonAstNode>& query_)
{
    return _db->query_value<model::PythonAstCount>(
            AstQuery::location.file == std::stoull(fileId_) && query_).count;
}

std::vector<model::PythonAstNode> PythonServiceHandler::queryDeclarations(const core::AstNodeId& astNodeId_)
{
    return queryPythonAstNodes(
            astNodeId_,
            AstQuery::astType == model::PythonAstNode::AstType::Declaration);
}

odb::query<model::PythonAstNode> PythonServiceHandler::astCallsQuery(const model::PythonAstNode& astNode_)
{
    const model::Position& start = astNode_.location.range.start;
    const model::Position& end = astNode_.location.range.end;

    return (AstQuery::location.file == astNode_.location.file.object_id() &&
            (AstQuery::astType == model::PythonAstNode::AstType::Usage ||
            AstQuery::symbolType == model::PythonAstNode::SymbolType::Function &&
            // StartPos >= Pos
            ((AstQuery::location.range.start.line == start.line &&
              AstQuery::location.range.start.column >= start.column) ||
             AstQuery::location.range.start.line > start.line) &&
            // Pos > EndPos
            ((AstQuery::location.range.end.line == end.line &&
              AstQuery::location.range.end.column < end.column) ||
             AstQuery::location.range.end.line < end.line)));
}

std::vector<model::PythonAstNode> PythonServiceHandler::queryCalls(const core::AstNodeId& astNodeId_)
{
    std::vector<model::PythonAstNode> nodes = queryDeclarations(astNodeId_);

    if (nodes.empty()){
        return nodes;
    }

    model::PythonAstNode node = nodes.front();
    AstResult result = _db->query<model::PythonAstNode>(astCallsQuery(node));

    return std::vector<model::PythonAstNode>(result.begin(), result.end());
}

std::size_t PythonServiceHandler::queryPythonAstNodeCount(
        const core::AstNodeId& astNodeId_,
        const odb::query<model::PythonAstNode>& query_)
{
    model::PythonAstNode node = queryPythonAstNode(astNodeId_);

    model::PythonAstCount q = _db->query_value<model::PythonAstCount>(
            AstQuery::id == node.id &&
            AstQuery::location.range.end.line != model::Position::npos &&
            query_);

    return q.count;
}

std::size_t PythonServiceHandler::queryCallsCount(const core::AstNodeId& astNodeId_)
{
    std::vector<model::PythonAstNode> nodes = queryDeclarations(astNodeId_);

    if (nodes.empty()){
        return std::size_t(0);
    }

    model::PythonAstNode node = nodes.front();

    return _db->query_value<model::PythonAstCount>(astCallsQuery(node)).count;
}

std::vector<model::PythonClass> PythonServiceHandler::queryTypes(const model::PythonEntity& entity)
{
    std::vector<model::PythonClass> result;

    for(const model::PythonType& type : _db->query<model::PythonType>(TypeQuery::symbol == entity.id)){
        ClassResult cl = _db->query<model::PythonClass>(ClassQuery::id == type.type);
        result.push_back(*cl.begin());
    }

    return result;
}

model::PythonEntity PythonServiceHandler::queryPythonEntity(const model::PythonEntityId& id)
{
    EntityResult entities = _db->query<model::PythonEntity>(EntityQuery::id == id);
    return *entities.begin();
}

model::PythonEntity PythonServiceHandler::queryPythonEntityByAstNode(const model::AstNodeId& id)
{
    EntityResult entities = _db->query<model::PythonEntity>(EntityQuery::astNodeId == id);
    return *entities.begin();
}

std::map<model::PythonAstNodeId, std::string> PythonServiceHandler::getVisibilities(
        const std::vector<model::PythonAstNode>& nodes_)
{
    std::map<model::PythonAstNodeId, std::string> visibilities;

    for (const model::PythonAstNode& node : nodes_)
    {
        switch(node.symbolType){
            case mode::PythonAstNode::SymbolType::Variable:
            case mode::PythonAstNode::SymbolType::Function:
            case mode::PythonAstNode::SymbolType::Class:
                model::PythonEntity entity = queryPythonEntity(node);
                visibilities[node.id] = entity.visibility;
                break;
        }
    }

    return visibilities;
}

}
}
}