using System.Collections.Generic;

namespace CSharpParser.model
{
    enum MethodTypeEnum
    {
        Delegate,
        Accessor,
        Constructor,
        Destuctor,
        Method,
        Operator
    }

    class CsharpMethod : CsharpTypedEntity
    {
        public MethodTypeEnum MethodType { get; set; }
    }
    
}
