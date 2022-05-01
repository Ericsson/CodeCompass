using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace CSharpParser.model
{
    //[Table("csharp_classes")]
    class CsharpClass : CsharpEntity
    {
        public bool IsInterface { get; set; }
        public bool IsRecord { get; set; }
        public CsharpNamespace CsharpNamespace { get; set; }
        public HashSet<CsharpVariable> CsharpClassLocals { get; set; } = new HashSet<CsharpVariable>();
        public HashSet<CsharpMethod> CsharpClassMethods { get; set; } = new HashSet<CsharpMethod>();
        public HashSet<CsharpMethod> CsharpClassConstructors { get; set; } = new HashSet<CsharpMethod>();
        public HashSet<CsharpMethod> CsharpClassDestructors { get; set; } = new HashSet<CsharpMethod>();
        public HashSet<CsharpEtcEntity> CsharpClassEvents { get; set; } = new HashSet<CsharpEtcEntity>();
        public void AddEvent(CsharpEtcEntity ev)
        {
            CsharpClassEvents.Add(ev);
        } 
        public void AddVariable(CsharpVariable var)
        {
            CsharpClassLocals.Add(var);
        }
        public void AddVariables(HashSet<CsharpVariable> vars)
        {
            CsharpClassLocals.UnionWith(vars);
        }
        public void AddMethod(CsharpMethod method)
        {
            CsharpClassMethods.Add(method);
        }
        public void AddMethods(HashSet<CsharpMethod> methods)
        {
            CsharpClassMethods.UnionWith(methods);
        }
        public void AddConstructor(CsharpMethod method)
        {
            CsharpClassConstructors.Add(method);
        }
        public void AddDestructor(CsharpMethod method)
        {
            CsharpClassDestructors.Add(method);
        }
    }
    //[Table("csharp_structs")]
    class CsharpStruct : CsharpEntity
    {
        public CsharpNamespace CsharpNamespace { get; set; }
        public HashSet<CsharpVariable> CsharpStructLocals { get; set; } = new HashSet<CsharpVariable>();
        public HashSet<CsharpMethod> CsharpStructMethods { get; set; } = new HashSet<CsharpMethod>();
        public HashSet<CsharpEtcEntity> CsharpStructEvents { get; set; } = new HashSet<CsharpEtcEntity>();
        public void AddEvent(CsharpEtcEntity ev)
        {
            CsharpStructEvents.Add(ev);
        }
        public void AddVariable(CsharpVariable var)
        {
            CsharpStructLocals.Add(var);
        }
        public void AddVariables(HashSet<CsharpVariable> vars)
        {
            CsharpStructLocals.UnionWith(vars);
        }
        public void AddMethod(CsharpMethod method)
        {
            CsharpStructMethods.Add(method);
        }    
        public void AddMethods(HashSet<CsharpMethod> methods)
        {
            CsharpStructMethods.UnionWith(methods);
        }        
        public void AddConstructor(CsharpMethod method)
        {
            CsharpStructMethods.Add(method);
        }
        public void AddDestructor(CsharpMethod method)
        {
            CsharpStructMethods.Add(method);
        }
    }
}
