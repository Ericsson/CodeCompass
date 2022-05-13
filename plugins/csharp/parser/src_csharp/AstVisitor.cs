using System;
using System.Collections.Generic;
using System.Linq;
using static System.Console;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using CSharpParser.model;
using Microsoft.CodeAnalysis;

namespace CSharpParser
{
    partial class AstVisitor : CSharpSyntaxWalker
    {
        private readonly CsharpDbContext DbContext;
        private readonly SemanticModel Model;
        private readonly SyntaxTree Tree;

        public bool FullyParsed = true;

        public AstVisitor(CsharpDbContext context, SemanticModel model, SyntaxTree tree)
        {
            this.DbContext = context;
            this.Model = model;
            this.Tree = tree;
        }    

        private ulong createIdentifier(CsharpAstNode astNode){
            string[] properties = 
            {
                astNode.AstValue,":",
                astNode.EntityHash.ToString(),":",
                astNode.RawKind.ToString(),":",
                astNode.Path,":",
                astNode.Location_range_start_line.ToString(),":",
                astNode.Location_range_start_column.ToString(),":",
                astNode.Location_range_end_line.ToString(),":",
                astNode.Location_range_end_column.ToString(),
            };

            string res = string.Concat(properties);
            
            //WriteLine(res);
            return fnvHash(res);
        }

        private ulong fnvHash(string data_)
        {
            ulong hash = 14695981039346656037;

            int len = data_.Length;
            for (int i = 0; i < len; ++i)
            {
                hash ^= data_[i];
                hash *= 1099511628211;
            }

            return hash;
        }     

        private ulong getAstNodeId(SyntaxNode node){
            CsharpAstNode astNode = new CsharpAstNode
            {
                AstValue = node.ToString(),
                RawKind = node.Kind(),
                EntityHash = node.GetHashCode()
            };
            astNode.SetLocation(Tree.GetLineSpan(node.Span));
            return createIdentifier(astNode);
        }  

        private CsharpAstNode AstNode(SyntaxNode node, AstSymbolTypeEnum type, AstTypeEnum astType)
        {
            CsharpAstNode astNode = new CsharpAstNode
            {
                AstValue = node.ToString(),
                RawKind = node.Kind(),
                EntityHash = node.GetHashCode(),
                AstSymbolType = type,
                AstType = astType
            };
            astNode.SetLocation(Tree.GetLineSpan(node.Span));
            astNode.Id = createIdentifier(astNode);          

            if (DbContext.CsharpAstNodes.Find(astNode.Id) == null)
            {
                DbContext.CsharpAstNodes.Add(astNode);
            }
            return astNode;
        }

        private CsharpAstNode AstNode(SyntaxNode node, AstSymbolTypeEnum type)
        {            
            return AstNode(node, type, AstTypeEnum.Declaration);
        }

        public override void VisitUsingDirective(UsingDirectiveSyntax node)
        {
            base.VisitUsingDirective(node);
            //Adatbázisban nem kell feltétlenül tárolni, inkább csak azt kell biztosítani hogy amiket meghívunk vele azok is be legyenek járva
            //WriteLine($" UsingDirective name: {node.Name}");
        }

        public override void VisitNamespaceDeclaration(NamespaceDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Namespace);
            //WriteLine($"\n NamespaceDeclaration visited: {node.Name}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Name}");
            }

            CsharpNamespace csharpNamespace = new CsharpNamespace
            {
                AstNode = astNode,
                Name = node.Name.ToString(),
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => n.Name == csharpNamespace.Name).ToList();
            if (nameSpaces.Count == 0)
            {
                DbContext.CsharpNamespaces.Add(csharpNamespace);
            }

            base.VisitNamespaceDeclaration(node);
        }

        public override void VisitInterfaceDeclaration(InterfaceDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Class);
            base.VisitInterfaceDeclaration(node);
            //WriteLine($"\n InterfaceDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count != 0)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpClass csharpClass = new CsharpClass
            {
                ClassType = ClassTypeEnum.Interface,
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };            

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                VisitPropertyDecl(propertyDeclaration, astNode);
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                VisitMethodDecl(methodDeclaration, astNode);
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                VisitOperatorDecl(operatorDeclaration, astNode);
            }            

            DbContext.CsharpClasses.Add(csharpClass);
        }

        public override void VisitStructDeclaration(StructDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Struct);
            base.VisitStructDeclaration(node);
            //WriteLine($"\n StructDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count != 0)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpStruct csharpStruct = new CsharpStruct
            {
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            foreach (VariableDeclarationSyntax variableDeclaration in node.Members.OfType<VariableDeclarationSyntax>())
            {
                //WriteLine($"Variable name: {variableDeclaration.Variables.First().Identifier}");
                VisitVariableDecl(variableDeclaration, astNode);
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                VisitPropertyDecl(propertyDeclaration, astNode);
                VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text, astNode);
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                VisitMethodDecl(methodDeclaration, astNode);
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                VisitOperatorDecl(operatorDeclaration, astNode);
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                VisitDelegateDecl(delegateDeclaration, astNode);
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                VisitConstructorDecl(constructorDeclaration, astNode);
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                VisitDestructorDecl(destructorDeclaration, astNode);
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration, AstSymbolTypeEnum.EtcEntity);
                string qName2 = "";
                try
                {
                    qName2 = Model.GetDeclaredSymbol(node).ToString();
                }
                catch (Exception)
                {
                    WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
                }  
                CsharpEtcEntity csharpEntity = new CsharpEtcEntity
                {
                    AstNode = astNode2,
                    Name = eventDeclaration.Identifier.Text,
                    QualifiedName = qName,
                    DocumentationCommentXML = Model.GetDeclaredSymbol(eventDeclaration).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash,
                    ParentNode = astNode,
                    EtcEntityType = EtcEntityTypeEnum.Event
                };
                DbContext.CsharpEtcEntitys.Add(csharpEntity);
            }

            DbContext.CsharpStructs.Add(csharpStruct);
        }

        public override void VisitClassDeclaration(ClassDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Class);
            base.VisitClassDeclaration(node);
            //WriteLine($"\n ClassDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count != 0)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpClass csharpClass = new CsharpClass
            {
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            foreach (VariableDeclarationSyntax variableDeclaration in node.Members.OfType<VariableDeclarationSyntax>())
            {
                //WriteLine($"Variable name: {variableDeclaration.Variables.First().Identifier}");
                VisitVariableDecl(variableDeclaration, astNode);
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                VisitPropertyDecl(propertyDeclaration, astNode);
                VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text, astNode);
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                VisitMethodDecl(methodDeclaration, astNode);
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                VisitOperatorDecl(operatorDeclaration, astNode);
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                VisitDelegateDecl(delegateDeclaration, astNode);
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                VisitConstructorDecl(constructorDeclaration, astNode);
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                VisitDestructorDecl(destructorDeclaration,astNode);
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration,AstSymbolTypeEnum.EtcEntity);
                string qName2 = "";
                try
                {
                    qName2 = Model.GetDeclaredSymbol(node).ToString();
                }
                catch (Exception)
                {
                    WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
                }
                CsharpEtcEntity csharpEntity = new CsharpEtcEntity
                {
                    AstNode = astNode2,
                    EtcEntityType = EtcEntityTypeEnum.Event,
                    Name = eventDeclaration.Identifier.Text,
                    QualifiedName = qName,
                    DocumentationCommentXML = Model.GetDeclaredSymbol(eventDeclaration).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash,
                    ParentNode = astNode
                };
                DbContext.CsharpEtcEntitys.Add(csharpEntity);
            }

            DbContext.CsharpClasses.Add(csharpClass);
        }

        public override void VisitRecordDeclaration(RecordDeclarationSyntax node) {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Class);
            //WriteLine($"\n RecordDeclaration visited: {node.Identifier}");
            base.VisitRecordDeclaration(node);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpClass csharpRecord = new CsharpClass
            {
                ClassType = ClassTypeEnum.Record,
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            foreach (VariableDeclarationSyntax variableDeclaration in node.Members.OfType<VariableDeclarationSyntax>())
            {
                //WriteLine($"Variable name: {variableDeclaration.Variables.First().Identifier}");
                VisitVariableDecl(variableDeclaration, astNode);
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                VisitPropertyDecl(propertyDeclaration, astNode);
                VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text, astNode);
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                VisitMethodDecl(methodDeclaration, astNode);
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                VisitOperatorDecl(operatorDeclaration, astNode);
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                VisitDelegateDecl(delegateDeclaration, astNode);
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                VisitConstructorDecl(constructorDeclaration, astNode);
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                VisitDestructorDecl(destructorDeclaration, astNode);
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration, AstSymbolTypeEnum.EtcEntity);
                string qName2 = "";
                try
                {
                    qName2 = Model.GetDeclaredSymbol(node).ToString();
                }
                catch (Exception)
                {
                    WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
                }
                CsharpEtcEntity csharpEntity = new CsharpEtcEntity
                {
                    AstNode = astNode2,
                    EtcEntityType = EtcEntityTypeEnum.Event,
                    Name = eventDeclaration.Identifier.Text,
                    QualifiedName = qName,
                    DocumentationCommentXML = Model.GetDeclaredSymbol(eventDeclaration).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash,
                    ParentNode = astNode
                };
                DbContext.CsharpEtcEntitys.Add(csharpEntity);
            }

            DbContext.CsharpClasses.Add(csharpRecord);
        }

        private void VisitDelegateDecl(DelegateDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node,AstSymbolTypeEnum.Method);
            //WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            CsharpMethod method = new CsharpMethod
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent,
                MethodType = MethodTypeEnum.Delegate
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                VisitMethodParameters(node.ParameterList.Parameters, astNode);
            }           

            DbContext.CsharpMethods.Add(method);
        }

        private void VisitDestructorDecl(DestructorDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Method);
            // WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            CsharpMethod method = new CsharpMethod
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent,
                MethodType = MethodTypeEnum.Destuctor
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                VisitMethodParameters(node.ParameterList.Parameters,astNode);
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                VisitVariableDecl(variableDeclaration, astNode);
            }

            DbContext.CsharpMethods.Add(method);
        }

        private void VisitConstructorDecl(ConstructorDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Method);
            // WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            CsharpMethod method = new CsharpMethod
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent,
                MethodType = MethodTypeEnum.Constructor
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                VisitMethodParameters(node.ParameterList.Parameters,astNode);
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                VisitVariableDecl(variableDeclaration, astNode);
            }

            DbContext.CsharpMethods.Add(method);
        }

        private void VisitMethodDecl(MethodDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Method);
           // WriteLine($"\n MethodDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            string qType = "";
            try
            {
                qType = Model.GetSymbolInfo(node.ReturnType).Symbol.ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedType of this Type: {node.ReturnType}");
            }

            CsharpMethod method = new CsharpMethod
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                QualifiedType = qType,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                TypeHash = qType.GetHashCode(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent,
                MethodType = MethodTypeEnum.Method
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                VisitMethodParameters(node.ParameterList.Parameters,astNode);
            }
            
            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                VisitVariableDecl(variableDeclaration, astNode);
            }         

            DbContext.CsharpMethods.Add(method);
        }

        private void VisitOperatorDecl(OperatorDeclarationSyntax node, CsharpAstNode parent)
        {
            //WriteLine($"\n OperatorDeclaration visited: {node}");
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Method);
            string qName = "";
            string Name = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
                Name = Model.GetDeclaredSymbol(node).Name;
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node}");
            }
            string qType = "";
            try
            {
                qType = Model.GetSymbolInfo(node.ReturnType).Symbol.ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedType of this Type: {node.ReturnType}");
            }

            CsharpMethod csharpOperator = new CsharpMethod
            {
                AstNode = astNode,
                Name = Name,
                QualifiedName = qName,
                QualifiedType = qType,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                TypeHash = qType.GetHashCode(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent,
                MethodType = MethodTypeEnum.Operator
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                VisitMethodParameters(node.ParameterList.Parameters,astNode);
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                VisitVariableDecl(variableDeclaration, astNode);
            }

            DbContext.CsharpMethods.Add(csharpOperator);
        }

        private void VisitMethodParameters(SeparatedSyntaxList<ParameterSyntax> parameters, CsharpAstNode parent)
        {
            foreach (var param in parameters)
            {
                // WriteLine($"\t\t{param.Identifier} : {param.Type}");
                CsharpAstNode astNode = AstNode(param, AstSymbolTypeEnum.Variable);
                string paramQType = "";
                try
                {
                    paramQType = Model.GetSymbolInfo(param.Type).Symbol.ToString();
                }
                catch (Exception)
                {                    
                    FullyParsed = false;
                    WriteLine($"Can not get QualifiedType of this Type: {param.Type}");
                }
                CsharpVariable varibale = new CsharpVariable
                {
                    AstNode = astNode,
                    Name = param.Identifier.Text,
                    QualifiedType = paramQType,
                    TypeHash = paramQType.GetHashCode(),
                    EntityHash = astNode.EntityHash,
                    ParentNode = parent,
                    VariableType = VariableTypeEnum.Parameter
                };
                DbContext.CsharpVariables.Add(varibale);
            }
        }

        private void VisitVariableDecl(VariableDeclarationSyntax node, CsharpAstNode parent)
        {
            foreach (var variable in node.Variables)
            {
                CsharpAstNode astNode = AstNode(variable, AstSymbolTypeEnum.Variable);
                string varQType = "";
                bool isLINQvar = node.DescendantNodes().OfType<QueryExpressionSyntax>().Any();
                try
                {                      
                    if (node.Type.ToString() == "var"){
                        varQType = Model.GetOperation(variable.Initializer.Value).Type.ToString();                        
                    } else {
                        varQType = Model.GetSymbolInfo(node.Type).Symbol.ToString();
                    }                                
                }
                catch (Exception)
                {
                    FullyParsed = false;
                    WriteLine($"Can not get QualifiedType of this Type: {node.Type} at this node: '{node}'");
                }
                
                foreach (var member in node.DescendantNodes().OfType<MemberAccessExpressionSyntax>())
                {
                    isLINQvar = isLINQvar || member.DescendantNodes().OfType<IdentifierNameSyntax>()
                        .Where(memb => new string[]{"Where", "OfType", "Select", "SelectMany"}
                        .Contains(memb.Identifier.ValueText)).Any();              
                }
                
                isLINQvar = isLINQvar && (varQType.Contains("IEnumerable") 
                    || varQType.Contains("IOrderedEnumerable")  
                    || varQType.Contains("IQueryable")); 

                if (isLINQvar) WriteLine($"LINQvar node: '{node}' QualifiedType: '{varQType}'");
                if (varQType == "?") WriteLine($"LINQvar ? node: '{node}' QualifiedType: '{varQType}'");

                CsharpVariable csharpVariable = new CsharpVariable
                {
                    AstNode = astNode,
                    Name = variable.Identifier.Text,
                    QualifiedType = varQType,
                    TypeHash = varQType.GetHashCode(),
                    DocumentationCommentXML = Model.GetDeclaredSymbol(variable).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash,
                    VariableType = isLINQvar ? VariableTypeEnum.LINQ : VariableTypeEnum.Variable,
                    ParentNode = parent
                };
                DbContext.CsharpVariables.Add(csharpVariable);
            }
        }

        private void VisitPropertyDecl(PropertyDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Variable);
            string varQType = "";
            try
            {
                varQType = Model.GetSymbolInfo(node.Type).Symbol.ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedType of this Type: {node.Type}");
            }
            CsharpVariable variable = new CsharpVariable
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedType = varQType,
                TypeHash = varQType.GetHashCode(),
                VariableType = VariableTypeEnum.Property,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash,
                ParentNode = parent
            };
            DbContext.CsharpVariables.Add(variable);
        }

        private void VisitAccessors(AccessorListSyntax node, String propertyName, CsharpAstNode parent)
        {
            HashSet<CsharpMethod> methods = new HashSet<CsharpMethod>();

            if (node == null) return;

            foreach (AccessorDeclarationSyntax accessor in node.Accessors)
            {
                CsharpAstNode astNode = AstNode(accessor, AstSymbolTypeEnum.Method);

                String name = "";
                switch (accessor.Kind())
                {
                    case SyntaxKind.GetAccessorDeclaration:
                        name = ".Get";
                        break;
                    case SyntaxKind.SetAccessorDeclaration:
                        name = ".Set";
                        break;
                    case SyntaxKind.InitAccessorDeclaration:
                        name = ".Init";
                        break;
                    case SyntaxKind.AddAccessorDeclaration:
                        name = ".Add";
                        break;
                    case SyntaxKind.RemoveAccessorDeclaration:
                        name = ".Remove";
                        break;
                    case SyntaxKind.UnknownAccessorDeclaration:
                        name = ".Unknown";
                        break;
                    default:
                        WriteLine($"Can not get Type of this Accesor: {node}");
                        break;
                }

                CsharpMethod method = new CsharpMethod
                {
                    AstNode = astNode,
                    Name = propertyName+name+"Accessor",
                    DocumentationCommentXML = Model.GetDeclaredSymbol(accessor).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash,
                    ParentNode = parent,
                    MethodType = MethodTypeEnum.Accessor
                };

                foreach (VariableDeclarationSyntax variableDeclaration in accessor.DescendantNodes().OfType<VariableDeclarationSyntax>())
                {
                    VisitVariableDecl(variableDeclaration, astNode);
                }

                DbContext.CsharpMethods.Add(method);
            } 
        }

        public override void VisitEnumDeclaration(EnumDeclarationSyntax node)
        {
            //WriteLine($"\n EnumDeclaration visited: {node.Identifier.Text}");
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.Enum);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpEnum csharpEnum = new CsharpEnum
            {
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            foreach (EnumMemberDeclarationSyntax enumMemberDeclarationSyntax in node.Members)
            {
                csharpEnum.AddMember(VisitEnumMemberDecl(enumMemberDeclarationSyntax, astNode));
            }
            DbContext.CsharpEnums.Add(csharpEnum);
        }

        private CsharpEnumMember VisitEnumMemberDecl(EnumMemberDeclarationSyntax node, CsharpAstNode parent)
        {
            CsharpAstNode astNode = AstNode(node, AstSymbolTypeEnum.EnumMember);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                FullyParsed = false;
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }
            CsharpEnumMember csharpEnumMember = new CsharpEnumMember
            {
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                EntityHash = astNode.EntityHash,
                ParentNode = parent
            };
            if (node.EqualsValue != null)
            {
                try
                {
                    csharpEnumMember.EqualsValue = int.Parse(node.EqualsValue.Value.ToString());
                }
                catch (FormatException)
                {
                    WriteLine($"Unable to parse '{node.EqualsValue.Value}'");
                }
            }
            DbContext.CsharpEnumMembers.Add(csharpEnumMember);
            return csharpEnumMember;
        }

        public override void VisitInvocationExpression(InvocationExpressionSyntax node)
        {
            if (node.Expression.GetFirstToken().GetNextToken().ToString() == ".") //object 
            {
                var symbol = Model.GetSymbolInfo(node.Expression.GetFirstToken().Parent).Symbol;
                if (symbol != null)
                {
                    foreach (var declaration in symbol.DeclaringSyntaxReferences)
                    {
                        if (declaration.GetSyntax().Kind() == SyntaxKind.VariableDeclarator)
                        {
                            //WriteLine($">>>Used Variable: {node.Expression.GetFirstToken()}");    
                            var info = Model.GetTypeInfo(node).ConvertedType;
                            //WriteLine($">>>Expression type: {info.Name}");  
                            var declaratorNodeId = getAstNodeId(declaration.GetSyntax());
                            var astNode = AstNode(node, AstSymbolTypeEnum.EtcEntity, AstTypeEnum.Usage);
                            CsharpEtcEntity invoc = new CsharpEtcEntity
                            {
                                AstNode = astNode,
                                DocumentationCommentXML = Model
                                    .GetDeclaredSymbol(declaration.GetSyntax())
                                    .GetDocumentationCommentXml(),
                                EntityHash = astNode.EntityHash,
                                //ParentNode = DbContext.CsharpAstNodes.Find(astNode.Id),
                                EtcEntityType = EtcEntityTypeEnum.Invocation,
                                DeclaratorNodeId = declaratorNodeId,
                                Name = node.Expression.GetFirstToken().ToString(),
                                QualifiedType = info.Name
                            };
                            DbContext.CsharpEtcEntitys.Add(invoc);
                        }
                    }
                }                
            }
            else if (node.Expression.GetFirstToken().GetNextToken().ToString() == "(") //function 
            {
                var symbol = Model.GetSymbolInfo(node.Expression.GetFirstToken().Parent).Symbol;
                if (symbol != null)
                {
                    foreach (var declaration in symbol.DeclaringSyntaxReferences)
                    {
                        if (declaration.GetSyntax().Kind() == SyntaxKind.VariableDeclarator)
                        {
                            WriteLine($">>>Expression kind: {declaration.GetSyntax().Kind()} in node: '{declaration.GetSyntax()}'");                            
                        }
                    }
                }
                
            }

            base.VisitInvocationExpression(node);
        }

        public override void VisitForEachStatement(ForEachStatementSyntax node)
        {
            var symbol = Model.GetSymbolInfo(node.Expression.GetFirstToken().Parent).Symbol;
                if (symbol != null)
                {
                    foreach (var declaration in symbol.DeclaringSyntaxReferences)
                    {
                        if (declaration.GetSyntax().Kind() == SyntaxKind.VariableDeclarator)
                        {
                            WriteLine($">>>Used Variable: {node.Expression.GetFirstToken()}");    
                            var info = Model.GetTypeInfo(node.Expression.GetFirstToken().Parent).ConvertedType;
                            WriteLine($">>>Expression type: {info.Name}");  
                            var declaratorNodeId = getAstNodeId(declaration.GetSyntax());
                            var astNode = AstNode(node, AstSymbolTypeEnum.EtcEntity, AstTypeEnum.Usage);
                            CsharpEtcEntity invoc = new CsharpEtcEntity
                            {
                                AstNode = astNode,
                                DocumentationCommentXML = Model
                                    .GetDeclaredSymbol(declaration.GetSyntax())
                                    .GetDocumentationCommentXml(),
                                EntityHash = astNode.EntityHash,
                                //ParentNode = DbContext.CsharpAstNodes.Find(astNode.Id),
                                EtcEntityType = EtcEntityTypeEnum.ForeachExpr,
                                DeclaratorNodeId = declaratorNodeId,
                                Name = node.Expression.GetFirstToken().ToString(),
                                QualifiedType = info.Name
                            };
                            DbContext.CsharpEtcEntitys.Add(invoc);
                        }
                    }
                }       

            base.VisitForEachStatement(node);
        }
    }
}
