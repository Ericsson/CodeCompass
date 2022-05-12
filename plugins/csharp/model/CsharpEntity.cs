using System;

namespace CSharpParser.model
{
    class CsharpEntity
    {
        public long Id { get; set; }
        public CsharpAstNode AstNode { get; set; }
        public CsharpAstNode ParentNode{  get; set; }
        public long EntityHash { get; set; }
        public String Name { get; set; } = " ";
        public String QualifiedName { get; set; } = " ";
        public string DocumentationCommentXML { get; set; } = " ";
    }

    class CsharpTypedEntity : CsharpEntity
    {
        public long TypeHash { get; set; }
        public String QualifiedType { get; set; } = " ";
    }

    enum EtcEntityTypeEnum
    {
        Event,
        Invocation
    }
    class CsharpEtcEntity : CsharpTypedEntity
    {
        public EtcEntityTypeEnum EtcEntityType { get; set; }
        public ulong DeclaratorNodeId { get; set; }
    }
}
