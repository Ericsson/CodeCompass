using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

namespace CSharpParser.model
{
    enum AstSymbolTypeEnum
    {
        Variable,
        Method, 
        Class,
        Struct,
        Namespace,
        Enum,
        EnumMember,
        EtcEntity

    }

    enum AstTypeEnum
    {
        Declaration,
        Definition,
        Usage,
        Read,
        Write,
        Expression
    }

    class CsharpAstNode
    {
        public ulong Id { get; set; }
        public string AstValue { get; set; }
        public AstSymbolTypeEnum AstSymbolType { get; set; }
        public AstTypeEnum AstType { get; set; }
        public Accessibility Accessibility { get; set; }
        public long Location_range_start_line { get; set; }
        public long Location_range_start_column { get; set; }
        public long Location_range_end_line { get; set; }
        public long Location_range_end_column { get; set; }
        public string Path { get; set; }
        public long EntityHash { get; set; }
        public SyntaxKind RawKind { get; set; } //SyntaxKind Enum
        public void SetLocation(FileLinePositionSpan f)
        {
            Location_range_start_line = f.StartLinePosition.Line+1;
            Location_range_start_column = f.StartLinePosition.Character+1;
            Location_range_end_line = f.EndLinePosition.Line+1;
            Location_range_end_column = f.EndLinePosition.Character+1;
            Path = f.Path;
        }
        public bool isRangeSmaller(CsharpAstNode other){
            if (Location_range_start_line == other.Location_range_start_line){
                if (Location_range_end_line == other.Location_range_end_line){
                    return Location_range_end_column - Location_range_start_column <
                        other.Location_range_end_column - other.Location_range_start_column;
                }
                return Location_range_end_line < other.Location_range_end_line;
            } else if (Location_range_end_line - Location_range_start_line ==
                other.Location_range_end_line - other.Location_range_start_line){
                return Location_range_end_column - Location_range_start_column <
                        other.Location_range_end_column - other.Location_range_start_column;
            }
            return Location_range_end_line - Location_range_start_line <
                other.Location_range_end_line - other.Location_range_start_line;
        }
    }
}
