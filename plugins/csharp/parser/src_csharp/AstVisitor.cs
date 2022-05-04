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

        private CsharpAstNode AstNode(SyntaxNode node, AstTypeEnum type)
        {
            CsharpAstNode astNode = new CsharpAstNode
            {
                AstValue = node.ToString(),
                RawKind = node.Kind(),
                EntityHash = node.GetHashCode(),
                AstType = type
            };
            astNode.SetLocation(Tree.GetLineSpan(node.Span));
            astNode.Id = createIdentifier(astNode);
            DbContext.CsharpAstNodes.Add(astNode);
            return astNode;
        }

        public override void VisitUsingDirective(UsingDirectiveSyntax node)
        {
            //base.VisitUsingDirective(node);
            //Adatbázisban nem kell feltétlenül tárolni, inkább csak azt kell biztosítani hogy amiket meghívunk vele azok is be legyenek járva
            //WriteLine($" UsingDirective name: {node.Name}");
        }

        public override void VisitNamespaceDeclaration(NamespaceDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Namespace);
            //WriteLine($"\n NamespaceDeclaration visited: {node.Name}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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

            DbContext.CsharpNamespaces.Add(csharpNamespace);
            base.VisitNamespaceDeclaration(node);
        }

        public override void VisitInterfaceDeclaration(InterfaceDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Class);
            base.VisitInterfaceDeclaration(node);
            //WriteLine($"\n InterfaceDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Struct);
            base.VisitStructDeclaration(node);
            //WriteLine($"\n StructDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
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
                WriteLine($"Variable name: {variableDeclaration.Variables.First().Identifier}");
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
                CsharpAstNode astNode2 = AstNode(eventDeclaration, AstTypeEnum.EtcEntity);
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
                    ParentNode = astNode
                };
                DbContext.CsharpEtcEntitys.Add(csharpEntity);
            }

            DbContext.CsharpStructs.Add(csharpStruct);
        }

        public override void VisitClassDeclaration(ClassDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Class);
            base.VisitClassDeclaration(node);
            //WriteLine($"\n ClassDeclaration visited: {node.Identifier.Text}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = DbContext.CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
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
                CsharpAstNode astNode2 = AstNode(eventDeclaration,AstTypeEnum.EtcEntity);
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Class);
            //WriteLine($"\n RecordDeclaration visited: {node.Identifier}");
            base.VisitRecordDeclaration(node);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
                CsharpAstNode astNode2 = AstNode(eventDeclaration, AstTypeEnum.EtcEntity);
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
            CsharpAstNode astNode = AstNode(node,AstTypeEnum.Method);
            //WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Method);
            // WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Method);
            // WriteLine($"\n ConstructorDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Method);
           // WriteLine($"\n MethodDeclaration visited: {node.Identifier}");
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            string qType = "";
            try
            {
                qType = Model.GetSymbolInfo(node.ReturnType).Symbol.ToString();
            }
            catch (Exception)
            {
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Method);
            string qName = "";
            string Name = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
                Name = Model.GetDeclaredSymbol(node).Name;
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node}");
            }
            string qType = "";
            try
            {
                qType = Model.GetSymbolInfo(node.ReturnType).Symbol.ToString();
            }
            catch (Exception)
            {
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
                CsharpAstNode astNode = AstNode(param, AstTypeEnum.Variable);
                string paramQType = "";
                try
                {
                    paramQType = Model.GetSymbolInfo(param.Type).Symbol.ToString();
                }
                catch (Exception)
                {
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
                CsharpAstNode astNode = AstNode(variable, AstTypeEnum.Variable);
                string varQType = "";
                bool isLINQvar = node.DescendantNodes().OfType<QueryExpressionSyntax>().Any();
                try
                {
                    if (node.Type.ToString() == "var"){
                        //varQType = ((ILocalSymbol)Model.GetDeclaredSymbol(variable)).Type.ToString();
                        varQType = Model.GetOperation(variable.Initializer.Value).Type.ToString();
                        //WriteLine($"node: '{node}' QualifiedType: '{varQType}'");
                    } else {
                        varQType = Model.GetSymbolInfo(node.Type).Symbol.ToString();
                    }
                }
                catch (Exception)
                {
                    WriteLine($"Can not get QualifiedType of this Type: {node.Type} at this node: '{node}'");
                }

                foreach (var member in node.DescendantNodes().OfType<MemberAccessExpressionSyntax>())
                {
                    isLINQvar = isLINQvar || member.DescendantNodes().OfType<IdentifierNameSyntax>()
                        .Where(memb => new string[]{"Where", "OfType", "Select", "SelectMany"}
                        .Contains(memb.Identifier.ValueText)).Any();              
                }

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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Variable);
            string varQType = "";
            try
            {
                varQType = Model.GetSymbolInfo(node.Type).Symbol.ToString();
            }
            catch (Exception)
            {
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
                CsharpAstNode astNode = AstNode(accessor, AstTypeEnum.Method);

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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.Enum);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
            CsharpAstNode astNode = AstNode(node, AstTypeEnum.EnumMember);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
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
    }
}
