using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace CSharpParser.model
{
    //[Table("csharp_methods")]
    class CsharpMethod : CsharpTypedEntity
    {
        public HashSet<CsharpVariable> CsharpMethodParams { get; set; } = new HashSet<CsharpVariable>();
        public HashSet<CsharpVariable> CsharpMethodLocals { get; set; } = new HashSet<CsharpVariable>();

        public bool IsDelegate { get; set; }

        public bool IsAccessor { get; set; }

        public void AddParam(CsharpVariable var)
        {
            CsharpMethodParams.Add(var);
        }
        public void AddParams(HashSet<CsharpVariable> vars)
        {
            CsharpMethodParams.UnionWith(vars);
        }
        public void AddLocal(CsharpVariable var)
        {
            CsharpMethodLocals.Add(var);
        }
        public void AddLocals(HashSet<CsharpVariable> vars)
        {
            CsharpMethodLocals.UnionWith(vars);
        }
    }
}
