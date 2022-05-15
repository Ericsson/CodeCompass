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
using csharp;
using CSharpParser.model;

public class CSharpQueryHandler : CsharpService.IAsync
{        
    private CsharpDbContext dbContext;
    public CSharpQueryHandler(string connenctionString) {
        //Converting the connectionstring into entiy framwork style connectionstring
        connenctionString = connenctionString.Substring(connenctionString.IndexOf(':')+1);
        connenctionString = connenctionString.Replace("user", "username");
        string[] properties = connenctionString.Split(';');
        string csharpConnenctionString = "";
        for(int i = 0; i<properties.Length; ++i) {
            if (properties[i].Contains("database=")) {
                csharpConnenctionString += "Database=codecompass_csharp_db";
            } else {
                csharpConnenctionString += properties[i].Substring(0,1).ToUpper() 
                    + properties[i].Substring(1);
            }
            if (i<properties.Length-1){
                csharpConnenctionString += ";";
            }
        }

        //WriteLine($"[CSharpService] Converted connectionstring:\n{csharpConnenctionString}");

        dbContext = new CsharpDbContext(csharpConnenctionString);
        dbContext.Database.EnsureCreated();
    }

    private language.AstNodeInfo createAstNodeInfo(CsharpAstNode node){
        language.AstNodeInfo ret = new language.AstNodeInfo();
        ret.Id = node.Id.ToString();
        ret.AstNodeValue = node.AstValue;
        ret.AstNodeType = node.RawKind.ToString();
        ret.SymbolType = node.AstSymbolType.ToString();
        ret.Range = getFileRange(node);

        return ret;
    }

    private List<language.AstNodeInfo> createAstNodeInfoList(List<CsharpAstNode> nodeList){
        var ret = new List<language.AstNodeInfo>();
        foreach(var node in nodeList){
            var astNodeInfo = createAstNodeInfo(node);
            ret.Add(astNodeInfo);
            //System.Console.WriteLine($"[CSharpService] createAstNodeInfoList got this: {astNodeInfo.AstNodeValue}");
        }

        return ret;
    }

    private FileRange getFileRange(CsharpAstNode node) {
        FileRange fileRange = new FileRange();
        Position startPosition = new Position{
            Line = (int)node.Location_range_start_line,
            Column = (int)node.Location_range_start_column};

        Position endPosition = new Position{
            Line = (int)node.Location_range_end_line,
            Column = (int)node.Location_range_end_column};

        Range range = new Range{
            Startpos = startPosition,
            Endpos = endPosition
        };

        fileRange.File = node.Path;
        fileRange.Range = range;

        return fileRange;
    }

    private CsharpAstNode queryCsharpAstNode(string astNodeId){
        CsharpAstNode ret;
        try{
            ret = dbContext.CsharpAstNodes
                .Where(a => a.Id.ToString()==astNodeId)
                .First();
        } catch(InvalidOperationException e){
            System.Console.WriteLine($"[CSharpService error] There are no AstNode with this ID:{astNodeId}");
            ret = new CsharpAstNode();
            ret.Id = 0;
        }
        return ret;
    }

    private List<CsharpAstNode> queryInvocations(CsharpAstNode astNode){
        var ret = dbContext.CsharpEtcEntitys
            .Where(e => e.DeclaratorNodeId == astNode.Id)
            .Select(e => e.AstNode)
            .ToList();
        return ret;
    }

    private List<CsharpAstNode> queryDeclarators(CsharpAstNode astNode){
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
            return ids.Select(id => queryCsharpAstNode(id)).ToList();
        }
    }

    private List<CsharpAstNode> queryEvals(CsharpAstNode astNode){
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

    public async Task<language.AstNodeInfo> getAstNodeInfoAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getAstNodeInfoAsync");
        return await Task.FromResult(new language.AstNodeInfo());
    }

    public async Task<language.AstNodeInfo> getAstNodeInfoByPositionAsync(string path_, 
        Position pos_,
        CancellationToken cancellationToken = default(CancellationToken))
    {
        //System.Console.WriteLine("[CSharpService] getAstNodeInfoByPositionAsync"+
        //$" pos = {pos_.Line}:{pos_.Column}");
        
        var nodes = dbContext.CsharpAstNodes
            .Where(a => 
                a.Path == path_ &&
                ((a.Location_range_start_line == pos_.Line &&
                  a.Location_range_start_column <= pos_.Column) ||
                  a.Location_range_start_line < pos_.Line) &&
                ((a.Location_range_end_line == pos_.Line &&
                  a.Location_range_end_column > pos_.Column) ||
                  a.Location_range_end_line > pos_.Line));
        if (nodes.Count() == 0){
            System.Console.WriteLine("[CSharpService error] There are no AstNode at this position!");
            return await Task.FromResult(new language.AstNodeInfo());
        } 

        var minNode = nodes.FirstOrDefault();
        foreach (var node in nodes.ToList())
        {
            //System.Console.WriteLine($"\tnode range: {node.Location_range_start_line}:{node.Location_range_start_column} - "+
            //$"{node.Location_range_end_line}:{node.Location_range_end_column}");
            if (node.isRangeSmaller(minNode))
                minNode = node;            
        }

        return await Task.FromResult(createAstNodeInfo(minNode));
    }

    public async Task<Dictionary<string, string>> getPropertiesAsync(string astNodeIds, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        //System.Console.WriteLine("[CSharpService] getPropertiesAsync");
        Dictionary<string, string> ret = new Dictionary<string, string>();
        CsharpAstNode node = queryCsharpAstNode(astNodeIds);    
        ret.Add("AstNode Type", node.RawKind.ToString());    
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

    public async Task<string> getDocumentationAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getDocumentationAsync");
        return await Task.FromResult("Documentation");
    }

    public async Task<FileRange> getFileRangeAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {        
        return await Task.FromResult(getFileRange(queryCsharpAstNode(astNodeId)));
    }

    public async Task<Dictionary<string, int>> getReferenceTypesAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = queryCsharpAstNode(astNodeId);
        Dictionary<string, int> ret = new Dictionary<string, int>();
        ret.Add("Definition", (int)ReferenceType.DEFINITION);
        ret.Add("Declaration", (int)ReferenceType.DECLARATION);
        ret.Add("Usage", (int)ReferenceType.USAGE);
        switch(node.AstSymbolType){
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
                ret.Add("Virtual call", (int)ReferenceType.VIRTUAL_CALL);
                ret.Add("Function pointer call", (int)ReferenceType.FUNC_PTR_CALL);
                ret.Add("Parameters", (int)ReferenceType.PARAMETER);
                ret.Add("Local variables", (int)ReferenceType.LOCAL_VAR);
                ret.Add("Overrides", (int)ReferenceType.OVERRIDE);
                ret.Add("Overridden by", (int)ReferenceType.OVERRIDDEN_BY);
                ret.Add("Return type", (int)ReferenceType.RETURN_TYPE);
                break;
            case AstSymbolTypeEnum.Class:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                ret.Add("Inherits from", (int)ReferenceType.INHERIT_FROM);
                ret.Add("Inherited by", (int)ReferenceType.INHERIT_BY);
                ret.Add("Data member", (int)ReferenceType.DATA_MEMBER);
                ret.Add("Method", (int)ReferenceType.METHOD);
                break;
            case AstSymbolTypeEnum.Struct:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                ret.Add("Inherits from", (int)ReferenceType.INHERIT_FROM);
                ret.Add("Inherited by", (int)ReferenceType.INHERIT_BY);
                ret.Add("Data member", (int)ReferenceType.DATA_MEMBER);
                ret.Add("Method", (int)ReferenceType.METHOD);
                break;
            case AstSymbolTypeEnum.Namespace:
                ret.Add("Aliases", (int)ReferenceType.ALIAS);
                break;
            case AstSymbolTypeEnum.Enum:
                ret.Add("Enum constants", (int)ReferenceType.ENUM_CONSTANTS);
                break;
            case AstSymbolTypeEnum.EnumMember:                
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

    public async Task<int> getReferenceCountAsync(string astNodeId, int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = queryCsharpAstNode(astNodeId);        
        int ret = 0;
        switch ((ReferenceType)referenceId)
        {
            case ReferenceType.USAGE:
                ret = queryInvocations(node).Count();
                break;
            case ReferenceType.DECLARATION:
                ret = queryDeclarators(node).Count();
                break;
            case ReferenceType.EVALUATION:
                ret = queryEvals(node).Count();
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {(ReferenceType)referenceId}"+ 
                    " ReferenceType is unhandled");
                break;
        }
        return await Task.FromResult(ret);
    }

    public async Task<List<language.AstNodeInfo>> getReferencesAsync(string astNodeId, 
        int referenceId, List<string> tags, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var node = queryCsharpAstNode(astNodeId);        
        var ret = new List<language.AstNodeInfo>();
        switch ((ReferenceType)referenceId)
        {
            case ReferenceType.USAGE:
                ret = createAstNodeInfoList(queryInvocations(node));
                break;
            case ReferenceType.DECLARATION:
                ret = createAstNodeInfoList(queryDeclarators(node));
                break;
            case ReferenceType.EVALUATION:
                ret = createAstNodeInfoList(queryEvals(node));
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {(ReferenceType)referenceId}"+ 
                    " ReferenceType is unhandled");
                break;
        }
        return await Task.FromResult(ret);        
    }

    public async Task<Dictionary<string, int>> getFileReferenceTypesAsync(
        CancellationToken cancellationToken = default(CancellationToken))
    {
        var ret = new Dictionary<string, int>();
        ret.Add("Types", (int)FileReferenceType.TYPES);
        ret.Add("Functions", (int)FileReferenceType.FUNCTIONS);
        ret.Add("Includes", (int)FileReferenceType.INCLUDES);
        return await Task.FromResult(ret);
    }

    public async Task<int> getFileReferenceCountAsync(string path, int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(0);
    }

    public async Task<List<language.AstNodeInfo>> getFileReferencesAsync(string path, 
        int referenceId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new List<language.AstNodeInfo>());
    }

    public async Task<Dictionary<string, int>> getDiagramTypesAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new Dictionary<string, int>());
    }

    public async Task<string> getDiagramAsync(string astNodeId, int diagramId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult("Diagram");
    }

    public async Task<List<language.SyntaxHighlight>> getSyntaxHighlightAsync(FileRange range, 
        List<string> content, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        return await Task.FromResult(new List<language.SyntaxHighlight>());
    }


}