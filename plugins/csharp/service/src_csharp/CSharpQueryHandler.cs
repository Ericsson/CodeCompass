using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Security;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.EntityFrameworkCore;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using language;
using cc.service.csharp;
using DbModel;

public class CSharpQueryHandler : CsharpService.IAsync
{        
    private CsharpDbContext dbContext;
    public CSharpQueryHandler(string connectionString)
    {
        string dbSystem = connectionString.Substring(0, connectionString.IndexOf(':')).ToLower();
        //Converting the connectionstring into entiy framwork style connectionstring
        string csharpConnectionString = TransformConnectionString(dbSystem, connectionString);

        if(csharpConnectionString == null)
        {
            throw new NotSupportedException($"Unsupported database system:{dbSystem}");
        }

        dbContext = new CsharpDbContext(dbSystem, csharpConnectionString);
    }

    private language.AstNodeInfo CreateAstNodeInfo(CsharpAstNode node)
    {
        language.AstNodeInfo ret = new language.AstNodeInfo();
        ret.Id = node.Id.ToString();
        ret.AstNodeValue = node.AstValue;
        ret.AstNodeType = node.RawKind.ToString();
        ret.SymbolType = node.AstSymbolType.ToString();
        ret.Range = getFileRange(node);

        List<string> tags = new List<string>();
        tags.Add(node.Accessibility.ToString());

        ret.Tags = tags;

        return ret;
    }

    private List<language.AstNodeInfo> CreateAstNodeInfoList(List<CsharpAstNode> nodeList)
    {
        var ret = new List<language.AstNodeInfo>();
        foreach (var node in nodeList)
        {
            var astNodeInfo = CreateAstNodeInfo(node);
            ret.Add(astNodeInfo);
        }

        return ret;
    }

    private FileRange getFileRange(CsharpAstNode node)
    {
        FileRange fileRange = new FileRange();
        Position startPosition = new Position
        {
            Line = (int)node.Location_range_start_line,
            Column = (int)node.Location_range_start_column
        };

        Position endPosition = new Position
        {
            Line = (int)node.Location_range_end_line,
            Column = (int)node.Location_range_end_column
        };

        Range range = new Range
        {
            Startpos = startPosition,
            Endpos = endPosition
        };

        fileRange.File = node.Path;
        fileRange.Range = range;

        return fileRange;
    }

    private CsharpAstNode QueryCsharpAstNode(string astNodeId)
    {
        CsharpAstNode ret;
        try
        {
            ulong nodeId = ulong.Parse(astNodeId);
            ret = dbContext.CsharpAstNodes
                .Where(a => a.Id==nodeId)
                .First();
        }
        catch (InvalidOperationException e)
        {
            System.Console.WriteLine($"[CSharpService error] There are no AstNode with this ID:{astNodeId}");
            ret = new CsharpAstNode();
            ret.Id = 0;
        }
        catch (FormatException e)
        {
            System.Console.WriteLine($"[CSharpService error] Invalid AstNode ID format:{astNodeId}");
            ret = new CsharpAstNode();
            ret.Id = 0;
        }
        return ret;
    }

    private List<CsharpAstNode> QueryInvocations(CsharpAstNode astNode)
    {
        var ret = dbContext.CsharpEtcEntitys
            .Where(e => e.DeclaratorNodeId == astNode.Id)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryDeclarators(CsharpAstNode astNode)
    {
        var ids = dbContext.CsharpEtcEntitys
            .Where(e => e.AstNode.Id == astNode.Id)
            .Select(e => e.DeclaratorNodeId.ToString())
            .ToList();

        if (ids.Count == 0)
        {
            return new List<CsharpAstNode>();
        }
        else
        {
            return ids.Select(id => QueryCsharpAstNode(id)).ToList();
        }
    }

    private List<CsharpAstNode> QueryEvals(CsharpAstNode astNode)
    {
        var ret = 
            from invoc in dbContext.CsharpEtcEntitys
            join variable in dbContext.CsharpVariables 
                on invoc.DeclaratorNodeId equals variable.AstNode.Id
            where invoc.DeclaratorNodeId == astNode.Id
                && variable.VariableType == VariableTypeEnum.LINQ
                && (invoc.QualifiedType != "IEnumerable"
                    || invoc.EtcEntityType == EtcEntityTypeEnum.ForeachExpr)
            select invoc.AstNode;
        return ret.ToList();
    }

    private List<CsharpAstNode> QueryParams(CsharpAstNode astNode)
    {
        var ret = dbContext.CsharpVariables
            .Where(e => e.ParentNode.Id == astNode.Id 
                && e.VariableType == VariableTypeEnum.Parameter)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryLocals(CsharpAstNode astNode){        
        var ret = dbContext.CsharpVariables
            .Where(e => e.ParentNode.Id == astNode.Id 
                && e.VariableType == VariableTypeEnum.Variable)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryProperties(CsharpAstNode astNode)
    {
        var ret = dbContext.CsharpVariables
            .Where(e => e.ParentNode.Id == astNode.Id 
                && e.VariableType == VariableTypeEnum.Property)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryCalls(CsharpAstNode astNode)
    {
        var ret = 
            from invoc in dbContext.CsharpEtcEntitys
            join node in dbContext.CsharpAstNodes
                on invoc.DeclaratorNodeId equals node.Id 
            where node.AstSymbolType == AstSymbolTypeEnum.Method
                && invoc.AstNode.Path == astNode.Path  
                && invoc.AstNode.Location_range_start_line >= astNode.Location_range_start_line  
                && invoc.AstNode.Location_range_end_line <= astNode.Location_range_end_line                     
            select invoc.AstNode;
        return ret.ToList();
    }

    private List<CsharpAstNode> QueryCallees(CsharpAstNode astNode)
    {
        var ret = 
            from invoc in dbContext.CsharpEtcEntitys
            join node in dbContext.CsharpAstNodes
                on invoc.DeclaratorNodeId equals node.Id 
            where node.AstSymbolType == AstSymbolTypeEnum.Method
                && invoc.AstNode.Path == astNode.Path  
                && invoc.AstNode.Location_range_start_line >= astNode.Location_range_start_line  
                && invoc.AstNode.Location_range_end_line <= astNode.Location_range_end_line       
            select node;
        return ret.Distinct().ToList();
    }

    private List<CsharpAstNode> QueryCallers(CsharpAstNode astNode)
    {
        var invocations = dbContext.CsharpEtcEntitys
            .Where(e => e.DeclaratorNodeId == astNode.Id)
            .Select(e => e.AstNode);
        var ret = 
            from invoc in invocations
            join node in dbContext.CsharpAstNodes
                on invoc.Path equals node.Path 
            where node.AstSymbolType == AstSymbolTypeEnum.Method     
                && invoc.Location_range_start_line >= node.Location_range_start_line  
                && invoc.Location_range_end_line <= node.Location_range_end_line 
            select node;
        return ret.Distinct().ToList();
    }

    private List<CsharpAstNode> QueryEnumConsts(CsharpAstNode astNode)
    {
        var ret = new List<CsharpAstNode>();
        if (astNode.AstSymbolType == AstSymbolTypeEnum.Enum)
        {
            ret = dbContext.CsharpEnumMembers
            .Where(e => e.ParentNode.Id == astNode.Id)
            .Select(e => e.AstNode)
            .ToList();
        } 
        else if (astNode.AstSymbolType == AstSymbolTypeEnum.EnumMember)
        {
            var parent = dbContext.CsharpEnumMembers
            .Where(e => e.AstNode.Id == astNode.Id)
            .Select(e => e.ParentNode).FirstOrDefault();

            ret = dbContext.CsharpEnumMembers
            .Where(e => e.ParentNode.Id == parent.Id)
            .Select(e => e.AstNode)
            .ToList();
        }
        
        return ret;
    }
    
    private List<CsharpAstNode> QueryMethods(CsharpAstNode astNode)
    {
        var ret = dbContext.CsharpMethods
            .Where(e => e.ParentNode.Id == astNode.Id)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryMethodType(CsharpAstNode astNode, MethodTypeEnum type)
    {
        var ret = dbContext.CsharpMethods
            .Where(e => e.ParentNode.Id == astNode.Id
                && e.MethodType == type)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> QueryEvents(CsharpAstNode astNode)
    {
        var ret = dbContext.CsharpEtcEntitys
            .Where(e => e.ParentNode.Id == astNode.Id
                && e.EtcEntityType == EtcEntityTypeEnum.Event)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }
    

    public async Task<language.AstNodeInfo> getAstNodeInfo(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getAstNodeInfoAsync");
        return await Task.FromResult(new language.AstNodeInfo());
    }

    public async Task<language.AstNodeInfo> getAstNodeInfoByPosition(string path_, 
        Position pos_,
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var nodes = dbContext.CsharpAstNodes
            .Where(a => 
                a.Path == path_ &&
                ((a.Location_range_start_line == pos_.Line &&
                  a.Location_range_start_column <= pos_.Column) ||
                  a.Location_range_start_line < pos_.Line) &&
                ((a.Location_range_end_line == pos_.Line &&
                  a.Location_range_end_column > pos_.Column) ||
                  a.Location_range_end_line > pos_.Line));
        if (nodes.Count() == 0)
        {
            System.Console.WriteLine("[CSharpService error] There are no AstNode at this position!");
            return await Task.FromResult(new language.AstNodeInfo());
        } 

        var minNode = nodes.FirstOrDefault();
        foreach (var node in nodes.ToList())
        {
            if (node.isRangeSmaller(minNode))
                minNode = node;            
        }

        return await Task.FromResult(CreateAstNodeInfo(minNode));
    }

    public async Task<Dictionary<string, string>> getProperties(string astNodeIds, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        Dictionary<string, string> ret = new Dictionary<string, string>();
        CsharpAstNode node = QueryCsharpAstNode(astNodeIds);    
        ret.Add("AstNode Type", node.RawKind.ToString());    
        ret.Add("Accessibility", node.Accessibility.ToString());    
        switch(node.AstSymbolType){
            case AstSymbolTypeEnum.Variable:
                var variable = dbContext.CsharpVariables
                    .Where(v => v.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", variable.Name+" ");
                ret.Add("Qualified Name", variable.QualifiedName+" ");
                ret.Add("Documentation Comment", variable.DocumentationCommentXML+" ");
                ret.Add("Qualified Type", variable.QualifiedType+" ");
                ret.Add("Variable Type", variable.VariableType.ToString());
                break;
            case AstSymbolTypeEnum.Method:
                var method = dbContext.CsharpMethods
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", method.Name+" ");
                ret.Add("Qualified Name", method.QualifiedName+" ");
                ret.Add("Documentation Comment", method.DocumentationCommentXML+" ");
                ret.Add("Qualified Type", method.QualifiedType+" ");
                ret.Add("Method Type", method.MethodType.ToString());
                break;
            case AstSymbolTypeEnum.Class:
                var Class = dbContext.CsharpClasses
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Class.Name+" ");
                ret.Add("Qualified Name", Class.QualifiedName+" ");
                ret.Add("Documentation Comment", Class.DocumentationCommentXML+" ");
                if (Class.CsharpNamespace != null) ret.Add("Namespace", Class.CsharpNamespace.Name+" ");
                ret.Add("Class Type", Class.ClassType.ToString());
                break;
            case AstSymbolTypeEnum.Struct:
                var Struct = dbContext.CsharpClasses
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Struct.Name+" ");
                ret.Add("Qualified Name", Struct.QualifiedName+" ");
                ret.Add("Documentation Comment", Struct.DocumentationCommentXML+" ");
                if (Struct.CsharpNamespace != null) ret.Add("Namespace", Struct.CsharpNamespace.Name+" ");
                break;
            case AstSymbolTypeEnum.Namespace:
                var Namespace = dbContext.CsharpNamespaces
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Namespace.Name+" ");
                ret.Add("Qualified Name", Namespace.QualifiedName+" ");
                ret.Add("Documentation Comment", Namespace.DocumentationCommentXML+" ");
                break;
            case AstSymbolTypeEnum.Enum:
                var Enum = dbContext.CsharpEnums
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Enum.Name+" ");
                ret.Add("Qualified Name", Enum.QualifiedName+" ");
                ret.Add("Documentation Comment", Enum.DocumentationCommentXML+" ");
                ret.Add("Namespace", Enum.CsharpNamespace.Name+" ");
                break;
            case AstSymbolTypeEnum.EnumMember:
                var EnumMember = dbContext.CsharpEnumMembers
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", EnumMember.Name+" ");
                ret.Add("Qualified Name", EnumMember.QualifiedName+" ");
                ret.Add("Documentation Comment", EnumMember.DocumentationCommentXML+" ");                
                ret.Add("Value", EnumMember.EqualsValue.ToString());
                break;
            case AstSymbolTypeEnum.EtcEntity:
                var EtcEntity = dbContext.CsharpEtcEntitys
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", EtcEntity.Name+" ");
                ret.Add("Qualified Name", EtcEntity.QualifiedName+" ");
                ret.Add("Qualified Type", EtcEntity.QualifiedType+" ");
                ret.Add("Documentation Comment", EtcEntity.DocumentationCommentXML+" ");                
                ret.Add("Etc Entity Type", EtcEntity.EtcEntityType.ToString());                
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {node.AstSymbolType} kind is unhandled");
                break;
        }        
        return await Task.FromResult(ret);
    }

    public async Task<string> getDocumentation(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getDocumentationAsync");
        CsharpAstNode node = QueryCsharpAstNode(astNodeId);
        return await Task.FromResult("Documentation");
    }

    public async Task<FileRange> getFileRange(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {        
        return await Task.FromResult(getFileRange(QueryCsharpAstNode(astNodeId)));
    }

    public async Task<Dictionary<string, int>> getReferenceTypes(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = QueryCsharpAstNode(astNodeId);
        Dictionary<string, int> ret = new Dictionary<string, int>();
        ret.Add("Definition", (int)ReferenceType.DEFINITION);
        ret.Add("Declaration", (int)ReferenceType.DECLARATION);
        ret.Add("Usage", (int)ReferenceType.USAGE);
        switch (node.AstSymbolType)
        {
            case AstSymbolTypeEnum.Variable:    
                var variable = dbContext.CsharpVariables
                    .Where(v => v.AstNode == node)
                    .FirstOrDefault();            
                ret.Add("Reads", (int)ReferenceType.READ);
                ret.Add("Writes", (int)ReferenceType.WRITE);
                ret.Add("Type", (int)ReferenceType.TYPE);
                if (variable.VariableType == VariableTypeEnum.LINQ)
                {
                    ret.Add("LINQ evaluation", (int)ReferenceType.EVALUATION);
                    ret.Add("LINQ data modification", (int)ReferenceType.DATA_MODIFICATION);
                }
                break;
            case AstSymbolTypeEnum.Method:
                ret.Add("This calls", (int)ReferenceType.THIS_CALLS);
                ret.Add("Callee", (int)ReferenceType.CALLEE);
                ret.Add("Caller", (int)ReferenceType.CALLER);
                ret.Add("Parameters", (int)ReferenceType.PARAMETER);
                ret.Add("Local variables", (int)ReferenceType.LOCAL_VAR);
                break;
            case AstSymbolTypeEnum.Class:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                ret.Add("Inherits from", (int)ReferenceType.INHERIT_FROM);
                ret.Add("Inherited by", (int)ReferenceType.INHERIT_BY);
                ret.Add("Data members", (int)ReferenceType.DATA_MEMBER);
                ret.Add("Methods", (int)ReferenceType.METHOD);
                ret.Add("Accesors", (int)ReferenceType.ACCESSOR);
                ret.Add("Operators", (int)ReferenceType.OPERATOR);
                ret.Add("Constructors", (int)ReferenceType.CONSTRUCTOR);
                ret.Add("Delegates", (int)ReferenceType.DELEGATE);
                ret.Add("Destructors", (int)ReferenceType.DESTRUCTOR);
                ret.Add("Global variables", (int)ReferenceType.LOCAL_VAR);
                break;
            case AstSymbolTypeEnum.Struct:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                ret.Add("Inherits from", (int)ReferenceType.INHERIT_FROM);
                ret.Add("Inherited by", (int)ReferenceType.INHERIT_BY);
                ret.Add("Data member", (int)ReferenceType.DATA_MEMBER);
                ret.Add("Methods", (int)ReferenceType.METHOD);
                ret.Add("Accesors", (int)ReferenceType.ACCESSOR);
                ret.Add("Operators", (int)ReferenceType.OPERATOR);
                ret.Add("Constructors", (int)ReferenceType.CONSTRUCTOR);
                ret.Add("Delegates", (int)ReferenceType.DELEGATE);
                ret.Add("Destructors", (int)ReferenceType.DESTRUCTOR);
                ret.Add("Global variables", (int)ReferenceType.LOCAL_VAR);
                break;
            case AstSymbolTypeEnum.Namespace:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                break;
            case AstSymbolTypeEnum.Enum:
                ret.Add("Enum constants", (int)ReferenceType.ENUM_CONSTANTS);
                break;
            case AstSymbolTypeEnum.EnumMember:                
                ret.Add("Enum constants", (int)ReferenceType.ENUM_CONSTANTS);
                break;
            case AstSymbolTypeEnum.EtcEntity:
                ret.Add("Aliases", (int)ReferenceType.ALIAS); 
                ret.Add("Callee", (int)ReferenceType.CALLEE);
                ret.Add("Caller", (int)ReferenceType.CALLER);
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {node.AstSymbolType} kind is unhandled");
                break;
        }

        return await Task.FromResult(ret);
    }

    public async Task<int> getReferenceCount(string astNodeId, int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = QueryCsharpAstNode(astNodeId);        
        int ret = 0;
        switch ((ReferenceType)referenceId)
        {
            case ReferenceType.USAGE:
                ret = QueryInvocations(node).Count();
                break;
            case ReferenceType.DEFINITION:
            case ReferenceType.DECLARATION:
                ret = QueryDeclarators(node).Count();
                break;
            case ReferenceType.EVALUATION:
                ret = QueryEvals(node).Count();
                break;
            case ReferenceType.PARAMETER:
                ret = QueryParams(node).Count();
                break;
            case ReferenceType.LOCAL_VAR:
                ret = QueryLocals(node).Count();
                break;
            case ReferenceType.DATA_MEMBER:
                ret = QueryProperties(node).Count();
                break;
            case ReferenceType.THIS_CALLS:
                ret = QueryCalls(node).Count();
                break;
            case ReferenceType.CALLEE:
                ret = QueryCallees(node).Count();
                break;
            case ReferenceType.CALLER:
                ret = QueryCallers(node).Count();
                break;
            case ReferenceType.ENUM_CONSTANTS:
                ret = QueryEnumConsts(node).Count();
                break;
            case ReferenceType.METHOD:
                ret = QueryMethods(node).Count();
                break;
            case ReferenceType.CONSTRUCTOR:
                ret = QueryMethodType(node, MethodTypeEnum.Constructor).Count();
                break;
            case ReferenceType.DESTRUCTOR:
                ret = QueryMethodType(node, MethodTypeEnum.Destuctor).Count();
                break;
            case ReferenceType.OPERATOR:
                ret = QueryMethodType(node, MethodTypeEnum.Operator).Count();
                break;
            case ReferenceType.ACCESSOR:
                ret = QueryMethodType(node, MethodTypeEnum.Accessor).Count();
                break;
            case ReferenceType.DELEGATE:
                ret = QueryMethodType(node, MethodTypeEnum.Delegate).Count();
                break;
            case ReferenceType.EVENT:
                ret = QueryEvents(node).Count();
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {(ReferenceType)referenceId}"+ 
                    " ReferenceType is unhandled");
                break;
        }
        return await Task.FromResult(ret);
    }

    public async Task<List<language.AstNodeInfo>> getReferences(string astNodeId, 
        int referenceId, List<string> tags, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = QueryCsharpAstNode(astNodeId);        
        var ret = new List<language.AstNodeInfo>();
        switch ((ReferenceType)referenceId)
        {
            case ReferenceType.USAGE:
                ret = CreateAstNodeInfoList(QueryInvocations(node));
                break;
            case ReferenceType.DEFINITION:
            case ReferenceType.DECLARATION:
                ret = CreateAstNodeInfoList(QueryDeclarators(node));
                break;
            case ReferenceType.EVALUATION:
                ret = CreateAstNodeInfoList(QueryEvals(node));
                break;
            case ReferenceType.PARAMETER:
                ret = CreateAstNodeInfoList(QueryParams(node));
                break;
            case ReferenceType.LOCAL_VAR:
                ret = CreateAstNodeInfoList(QueryLocals(node));
                break;
            case ReferenceType.DATA_MEMBER:
                ret = CreateAstNodeInfoList(QueryProperties(node));
                break;
            case ReferenceType.THIS_CALLS:
                ret = CreateAstNodeInfoList(QueryCalls(node));
                break;
            case ReferenceType.CALLEE:
                ret = CreateAstNodeInfoList(QueryCallees(node));
                break;
            case ReferenceType.CALLER:
                ret = CreateAstNodeInfoList(QueryCallers(node));
                break;
            case ReferenceType.ENUM_CONSTANTS:
                ret = CreateAstNodeInfoList(QueryEnumConsts(node));
                break;
            case ReferenceType.METHOD:
                ret = CreateAstNodeInfoList(QueryMethods(node));
                break;
            case ReferenceType.CONSTRUCTOR:
                ret = CreateAstNodeInfoList(QueryMethodType(node, MethodTypeEnum.Constructor));
                break;
            case ReferenceType.DESTRUCTOR:
                ret = CreateAstNodeInfoList(QueryMethodType(node, MethodTypeEnum.Destuctor));
                break;
            case ReferenceType.OPERATOR:
                ret = CreateAstNodeInfoList(QueryMethodType(node, MethodTypeEnum.Operator));
                break;
            case ReferenceType.ACCESSOR:
                ret = CreateAstNodeInfoList(QueryMethodType(node, MethodTypeEnum.Accessor));
                break;
            case ReferenceType.DELEGATE:
                ret = CreateAstNodeInfoList(QueryMethodType(node, MethodTypeEnum.Delegate));
                break;
            case ReferenceType.EVENT:
                ret = CreateAstNodeInfoList(QueryEvents(node));
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {(ReferenceType)referenceId}"+ 
                    " ReferenceType is unhandled");
                break;
        }
        return await Task.FromResult(ret);        
    }

    public async Task<Dictionary<string, int>> getFileReferenceTypes(
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var ret = new Dictionary<string, int>();
        ret.Add("Types", (int)FileReferenceType.TYPES);
        ret.Add("Functions", (int)FileReferenceType.FUNCTIONS);
        ret.Add("Includes", (int)FileReferenceType.INCLUDES);
        return await Task.FromResult(ret);
    }

    public async Task<int> getFileReferenceCount(string path, int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(0);
    }

    public async Task<List<language.AstNodeInfo>> getFileReferences(string path, 
        int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new List<language.AstNodeInfo>());
    }

    public async Task<Dictionary<string, int>> getDiagramTypes(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new Dictionary<string, int>());
    }

    public async Task<string> getDiagram(string astNodeId, int diagramId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult("Diagram");
    }

    public async Task<List<language.SyntaxHighlight>> getSyntaxHighlight(FileRange range, 
        List<string> content, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new List<language.SyntaxHighlight>());
    }

    private static string TransformConnectionString(string dbSystem, string connectionString)
    {
      string csharpConnectionString = "";
      if (dbSystem == "pgsql")
      {
        connectionString = connectionString.Substring(connectionString.IndexOf(':') + 1);
        connectionString = connectionString.Replace("user", "username");
        string[] properties = connectionString.Split(';');

        for (int i = 0; i < properties.Length; ++i)
        {
          csharpConnectionString += properties[i].Substring(0, 1).ToUpper()
              + properties[i].Substring(1);
          if (i < properties.Length - 1)
          {
            csharpConnectionString += ";";
          }
        }
      }
      else if(dbSystem == "sqlite")
      {
        // "sqlite:database=" needs to be removed from the connection string.
        connectionString = connectionString.Substring(connectionString.IndexOf(':') + 10);
        csharpConnectionString = "Data Source=" + connectionString;
      }
      else
      {
        csharpConnectionString = null;
      }

      return csharpConnectionString;
    }

}