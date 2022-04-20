using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.CodeAnalysis;

namespace StandAloneCSharpParser.model
{
    class CsharpAstNode
    {
        public ulong Id { get; set; }
        public string AstValue { get; set; }
        public long Location_range_start_line { get; set; }
        public long Location_range_start_column { get; set; }
        public long Location_range_end_line { get; set; }
        public long Location_range_end_column { get; set; }
        public string Path { get; set; }
        public long EntityHash { get; set; }
        public int RawKind { get; set; } //SyntaxKind Enum
        public void SetLocation(FileLinePositionSpan f)
        {
            Location_range_start_line = f.StartLinePosition.Line;
            Location_range_start_column = f.StartLinePosition.Character;
            Location_range_end_line = f.EndLinePosition.Line;
            Location_range_end_column = f.EndLinePosition.Character;
            Path = f.Path;
        }
    }
}
