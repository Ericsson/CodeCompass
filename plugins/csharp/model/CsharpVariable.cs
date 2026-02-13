using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace CSharpParser.model
{
    enum VariableTypeEnum
    {
        Property,
        LINQ,
        Parameter,
        Variable
    }
    class CsharpVariable : CsharpTypedEntity
    {
        public VariableTypeEnum VariableType { get; set; }
    }
}
