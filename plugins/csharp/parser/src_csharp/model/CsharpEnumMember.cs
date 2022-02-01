using System.ComponentModel.DataAnnotations.Schema;

namespace StandAloneCSharpParser.model
{
    //[Table("csharp_enum_members")]
    class CsharpEnumMember : CsharpEntity
    {
        public int EqualsValue { get; set; }
    }
}
