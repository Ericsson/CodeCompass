using System;
using System.Collections.Generic;
using System.Linq;
using static System.Console;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using CSharpParser.model;
using Microsoft.CodeAnalysis;

namespace CSharpParser
{
    class AstVisitorHelper
    {
        public ulong createIdentifier(CsharpAstNode astNode){
            string[] properties = 
            {
                astNode.AstValue,":",
                astNode.AstType.ToString(),":",
                astNode.EntityHash.ToString(),":",
                astNode.RawKind.ToString(),":",
                astNode.Path,":",
                astNode.Location_range_start_line.ToString(),":",
                astNode.Location_range_start_column.ToString(),":",
                astNode.Location_range_end_line.ToString(),":",
                astNode.Location_range_end_column.ToString()
            };

            string res = string.Concat(properties);
            
            //WriteLine(res);
            return fnvHash(res);
        }

        private ulong fnvHash(string data_)
        {
            ulong hash = 14695981039346656037;

            int len = data_.Length;
            for (int i = 0; i < len; ++i)
            {
                hash ^= data_[i];
                hash *= 1099511628211;
            }

            return hash;
        }     

        public ulong getAstNodeId(SyntaxNode node){
            CsharpAstNode astNode = new CsharpAstNode
            {
                AstValue = node.ToString(),
                RawKind = node.Kind(),
                EntityHash = node.GetHashCode(),
                AstType = AstTypeEnum.Declaration
            };
            astNode.SetLocation(node.SyntaxTree.GetLineSpan(node.Span));
            var ret = createIdentifier(astNode);
            return ret;
        }
    }
}