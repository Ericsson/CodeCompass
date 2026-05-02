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
using CodeCompass.Service.Language;
using cc.service.csharp;
using CSharpParser.model;

class QueryHelper
    {
    public AstNodeInfo createAstNodeInfo(CsharpAstNode node)
    {
        AstNodeInfo ret = new AstNodeInfo();
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

    public List<AstNodeInfo> createAstNodeInfoList(List<CsharpAstNode> nodeList)
    {
        var ret = new List<AstNodeInfo>();
        foreach (var node in nodeList)
        {
            var astNodeInfo = createAstNodeInfo(node);
            ret.Add(astNodeInfo);
        }

        return ret;
    }

    public FileRange getFileRange(CsharpAstNode node)
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
    }