using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace StandAloneCSharpParser.model
{
    //[Table("csharp_variables")]
    class CsharpVariable : CsharpTypedEntity
    {
        public bool IsProperty { get; set; } = false;
    }
}
