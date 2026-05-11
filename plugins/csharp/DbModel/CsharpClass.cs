using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace DbModel
{
    public enum ClassTypeEnum
    {
        Class,
        Interface,
        Record
    }
    public class CsharpClass : CsharpEntity
    {
        public ClassTypeEnum ClassType { get; set; }
        public CsharpNamespace? CsharpNamespace { get; set; }
    }
    //[Table("csharp_structs")]
    public class CsharpStruct : CsharpEntity
    {
        public CsharpNamespace? CsharpNamespace { get; set; }
    }
}
