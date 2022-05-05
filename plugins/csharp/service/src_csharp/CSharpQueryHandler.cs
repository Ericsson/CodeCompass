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
        ret.Range = getFileRange(node);

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
        }
        return ret;
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
        switch(node.AstType){
            case AstTypeEnum.Variable:
                var variable = dbContext.CsharpVariables
                    .Where(v => v.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", variable.Name+" ");
                ret.Add("Qualified Name", variable.QualifiedName+" ");
                ret.Add("Documentation Comment", variable.DocumentationCommentXML+" ");
                ret.Add("Qualified Type", variable.QualifiedType+" ");
                ret.Add("Variable Type", variable.VariableType.ToString());
                break;
            case AstTypeEnum.Method:
                var method = dbContext.CsharpMethods
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", method.Name+" ");
                ret.Add("Qualified Name", method.QualifiedName+" ");
                ret.Add("Documentation Comment", method.DocumentationCommentXML+" ");
                ret.Add("Qualified Type", method.QualifiedType+" ");
                ret.Add("Method Type", method.MethodType.ToString());
                break;
            case AstTypeEnum.Class:
                var Class = dbContext.CsharpClasses
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Class.Name+" ");
                ret.Add("Qualified Name", Class.QualifiedName+" ");
                ret.Add("Documentation Comment", Class.DocumentationCommentXML+" ");
                ret.Add("Namespace", Class.CsharpNamespace.Name+" ");
                ret.Add("Class Type", Class.ClassType.ToString());
                break;
            case AstTypeEnum.Struct:
                var Struct = dbContext.CsharpClasses
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Struct.Name+" ");
                ret.Add("Qualified Name", Struct.QualifiedName+" ");
                ret.Add("Documentation Comment", Struct.DocumentationCommentXML+" ");
                ret.Add("Namespace", Struct.CsharpNamespace.Name+" ");
                break;
            case AstTypeEnum.Namespace:
                var Namespace = dbContext.CsharpNamespaces
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Namespace.Name+" ");
                ret.Add("Qualified Name", Namespace.QualifiedName+" ");
                ret.Add("Documentation Comment", Namespace.DocumentationCommentXML+" ");
                break;
            case AstTypeEnum.Enum:
                var Enum = dbContext.CsharpEnums
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", Enum.Name+" ");
                ret.Add("Qualified Name", Enum.QualifiedName+" ");
                ret.Add("Documentation Comment", Enum.DocumentationCommentXML+" ");
                ret.Add("Namespace", Enum.CsharpNamespace.Name+" ");
                break;
            case AstTypeEnum.EnumMember:
                var EnumMember = dbContext.CsharpEnumMembers
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", EnumMember.Name+" ");
                ret.Add("Qualified Name", EnumMember.QualifiedName+" ");
                ret.Add("Documentation Comment", EnumMember.DocumentationCommentXML+" ");                
                ret.Add("Value", EnumMember.EqualsValue.ToString());
                break;
            case AstTypeEnum.EtcEntity:
                var EtcEntity = dbContext.CsharpEtcEntitys
                    .Where(m => m.AstNode == node)
                    .FirstOrDefault();
                ret.Add("Name", EtcEntity.Name+" ");
                ret.Add("Qualified Name", EtcEntity.QualifiedName+" ");
                ret.Add("Documentation Comment", EtcEntity.DocumentationCommentXML+" ");                
                ret.Add("Etc Entity Type", EtcEntity.EtcEntityType.ToString());                
                break;
            default:
                System.Console.WriteLine($"[CSharpService] {node.AstType} kind is unhandled");
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

}