using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace StandAloneCSharpParser.model
{
    //[Table("csharp_enums")]
    class CsharpEnum : CsharpEntity
    {
        public CsharpNamespace CsharpNamespace { get; set; }
        public HashSet<CsharpEnumMember> CsharpEnumMembers { get; set; } = new HashSet<CsharpEnumMember>();

        public void AddMember(CsharpEnumMember member)
        {
            CsharpEnumMembers.Add(member);
        }
        public void AddMembers(HashSet<CsharpEnumMember> members)
        {
            CsharpEnumMembers.UnionWith(members);
        }
    }
}
