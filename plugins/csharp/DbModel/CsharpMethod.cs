using System.Collections.Generic;

namespace DbModel
{
    public enum MethodTypeEnum
    {
        Delegate,
        Accessor,
        Constructor,
        Destuctor,
        Method,
        Operator
    }

    public class CsharpMethod : CsharpTypedEntity
    {
        public MethodTypeEnum MethodType { get; set; }
    }
    
}
