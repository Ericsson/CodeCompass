using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace CSharpParser.model
{
    enum ClassTypeEnum
    {
        Class,
        Interface,
        Record
    }
    class CsharpClass : CsharpEntity
    {
        public ClassTypeEnum ClassType { get; set; }
        public CsharpNamespace CsharpNamespace { get; set; }
    }
    //[Table("csharp_structs")]
    class CsharpStruct : CsharpEntity
    {
        public CsharpNamespace CsharpNamespace { get; set; }
    }
}
