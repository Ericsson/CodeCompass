using System.ComponentModel.DataAnnotations.Schema;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace DbModel
{
    public enum VariableTypeEnum
    {
        Property,
        LINQ,
        Parameter,
        Variable
    }
    public class CsharpVariable : CsharpTypedEntity
    {
        public VariableTypeEnum VariableType { get; set; }
    }
}
