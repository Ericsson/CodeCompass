using System.ComponentModel.DataAnnotations.Schema;

namespace DbModel
{
    //[Table("csharp_enum_members")]
    public class CsharpEnumMember : CsharpEntity
    {
        public int EqualsValue { get; set; }
        public long? CsharpEnumId { get; set; }
    }
}
