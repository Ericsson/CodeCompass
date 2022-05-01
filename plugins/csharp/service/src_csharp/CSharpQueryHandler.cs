using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Security;
using System.Threading;
using System.Threading.Tasks;
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
        System.Console.WriteLine("[CSharpService] getAstNodeInfoByPositionAsync");
        
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
            if (node.isRangeSmaller(minNode))
                minNode = node;            
        }

        return await Task.FromResult(createAstNodeInfo(minNode));
    }

    public async Task<Dictionary<string, string>> getPropertiesAsync(string astNodeIds, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getPropertiesAsync");
        return await Task.FromResult(new Dictionary<string, string>());
    }

    public async Task<string> getDocumentationAsync(string astNodeId, 
        CancellationToken cancellationToken = default(CancellationToken))
    {
        System.Console.WriteLine("[CSharpService] getDocumentationAsync");
        return await Task.FromResult("Documentation");
    }

}