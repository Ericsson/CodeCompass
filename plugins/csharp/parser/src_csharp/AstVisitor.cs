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
        private readonly List<CsharpNamespace> CsharpNamespaces = new List<CsharpNamespace>();

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

        private CsharpAstNode AstNode(SyntaxNode node)
        {
            CsharpAstNode astNode = new CsharpAstNode
            {
                AstValue = node.ToString(),
                RawKind = node.RawKind,
                EntityHash = node.GetHashCode()
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
            CsharpAstNode astNode = AstNode(node);
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

            CsharpNamespaces.Add(csharpNamespace);
            DbContext.CsharpNamespaces.Add(csharpNamespace);
            base.VisitNamespaceDeclaration(node);
        }

        public override void VisitInterfaceDeclaration(InterfaceDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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

            var nameSpaces = CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpClass csharpClass = new CsharpClass
            {
                IsInterface = true,
                CsharpNamespace = csharpNamespace,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };            

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                csharpClass.AddVariable(VisitPropertyDecl(propertyDeclaration));
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                csharpClass.AddMethod(VisitMethodDecl(methodDeclaration));
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                csharpClass.AddMethod(VisitOperatorDecl(operatorDeclaration));
            }            

            DbContext.CsharpClasses.Add(csharpClass);
        }

        public override void VisitStructDeclaration(StructDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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

            var nameSpaces = CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
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
                csharpStruct.AddVariables(VisitVariableDecl(variableDeclaration));
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                csharpStruct.AddVariable(VisitPropertyDecl(propertyDeclaration));
                csharpStruct.AddMethods(VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text));
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                csharpStruct.AddMethod(VisitMethodDecl(methodDeclaration));
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                csharpStruct.AddMethod(VisitOperatorDecl(operatorDeclaration));
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                csharpStruct.AddMethod(VisitDelegateDecl(delegateDeclaration));
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                csharpStruct.AddConstructor(VisitConstructorDecl(constructorDeclaration));
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                csharpStruct.AddDestructor(VisitDestructorDecl(destructorDeclaration));
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration);
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
                    EntityHash = astNode.EntityHash
                };
                csharpStruct.AddEvent(csharpEntity);
            }

            DbContext.CsharpStructs.Add(csharpStruct);
        }

        public override void VisitClassDeclaration(ClassDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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

            var nameSpaces = CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
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
                csharpClass.AddVariables(VisitVariableDecl(variableDeclaration));
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                csharpClass.AddVariable(VisitPropertyDecl(propertyDeclaration));
                csharpClass.AddMethods(VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text));
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                csharpClass.AddMethod(VisitMethodDecl(methodDeclaration));
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                csharpClass.AddMethod(VisitOperatorDecl(operatorDeclaration));
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                csharpClass.AddMethod(VisitDelegateDecl(delegateDeclaration));
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                csharpClass.AddConstructor(VisitConstructorDecl(constructorDeclaration));
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                csharpClass.AddDestructor(VisitDestructorDecl(destructorDeclaration));
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration);
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
                    IsEvent = true,
                    Name = eventDeclaration.Identifier.Text,
                    QualifiedName = qName,
                    DocumentationCommentXML = Model.GetDeclaredSymbol(eventDeclaration).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash
                };
                csharpClass.AddEvent(csharpEntity);
            }

            DbContext.CsharpClasses.Add(csharpClass);
        }

        public override void VisitRecordDeclaration(RecordDeclarationSyntax node) {
            CsharpAstNode astNode = AstNode(node);
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

            var nameSpaces = CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
            CsharpNamespace csharpNamespace = null;
            if (nameSpaces.Count == 1)
            {
                csharpNamespace = nameSpaces.First();
            }

            CsharpClass csharpRecord = new CsharpClass
            {
                IsRecord = true,
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
                csharpRecord.AddVariables(VisitVariableDecl(variableDeclaration));
            }

            foreach (PropertyDeclarationSyntax propertyDeclaration in node.Members.OfType<PropertyDeclarationSyntax>())
            {
                csharpRecord.AddVariable(VisitPropertyDecl(propertyDeclaration));
                csharpRecord.AddMethods(VisitAccessors(propertyDeclaration.AccessorList, propertyDeclaration.Identifier.Text));
            }

            foreach (MethodDeclarationSyntax methodDeclaration in node.Members.OfType<MethodDeclarationSyntax>())
            {
                csharpRecord.AddMethod(VisitMethodDecl(methodDeclaration));
            }

            foreach (OperatorDeclarationSyntax operatorDeclaration in node.Members.OfType<OperatorDeclarationSyntax>())
            {
                csharpRecord.AddMethod(VisitOperatorDecl(operatorDeclaration));
            }

            foreach (DelegateDeclarationSyntax delegateDeclaration in node.Members.OfType<DelegateDeclarationSyntax>())
            {
                csharpRecord.AddMethod(VisitDelegateDecl(delegateDeclaration));
            }

            foreach (ConstructorDeclarationSyntax constructorDeclaration in node.Members.OfType<ConstructorDeclarationSyntax>())
            {
                csharpRecord.AddConstructor(VisitConstructorDecl(constructorDeclaration));
            }

            foreach (DestructorDeclarationSyntax destructorDeclaration in node.Members.OfType<DestructorDeclarationSyntax>())
            {
                csharpRecord.AddDestructor(VisitDestructorDecl(destructorDeclaration));
            }

            foreach (EventDeclarationSyntax eventDeclaration in node.Members.OfType<EventDeclarationSyntax>())
            {
                CsharpAstNode astNode2 = AstNode(eventDeclaration);
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
                    IsEvent = true,
                    Name = eventDeclaration.Identifier.Text,
                    QualifiedName = qName,
                    DocumentationCommentXML = Model.GetDeclaredSymbol(eventDeclaration).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash
                };
                csharpRecord.AddEvent(csharpEntity);
            }

            DbContext.CsharpClasses.Add(csharpRecord);
        }

        private CsharpMethod VisitDelegateDecl(DelegateDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                IsDelegate = true,
                AstNode = astNode,
                Name = node.Identifier.Text,
                QualifiedName = qName,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                method.AddParams(VisitMethodParameters(node.ParameterList.Parameters));
            }           

            return method;
        }

        private CsharpMethod VisitDestructorDecl(DestructorDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                EntityHash = astNode.EntityHash
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                method.AddParams(VisitMethodParameters(node.ParameterList.Parameters));
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                method.AddLocals(VisitVariableDecl(variableDeclaration));
            }

            return method;
        }

        private CsharpMethod VisitConstructorDecl(ConstructorDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                EntityHash = astNode.EntityHash
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                method.AddParams(VisitMethodParameters(node.ParameterList.Parameters));
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                method.AddLocals(VisitVariableDecl(variableDeclaration));
            }

            return method;
        }

        private CsharpMethod VisitMethodDecl(MethodDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                EntityHash = astNode.EntityHash
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                method.AddParams(VisitMethodParameters(node.ParameterList.Parameters));
            }
            
            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                method.AddLocals(VisitVariableDecl(variableDeclaration));
            }         

            return method;
        }

        private CsharpMethod VisitOperatorDecl(OperatorDeclarationSyntax node)
        {
            //WriteLine($"\n OperatorDeclaration visited: {node}");
            CsharpAstNode astNode = AstNode(node);
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
                EntityHash = astNode.EntityHash
            };

            if (node.ParameterList.Parameters.Count > 0)
            {
                csharpOperator.AddParams(VisitMethodParameters(node.ParameterList.Parameters));
            }

            foreach (VariableDeclarationSyntax variableDeclaration in node.DescendantNodes().OfType<VariableDeclarationSyntax>())
            {
                csharpOperator.AddLocals(VisitVariableDecl(variableDeclaration));
            }

            return csharpOperator;
        }

        private HashSet<CsharpVariable> VisitMethodParameters(SeparatedSyntaxList<ParameterSyntax> parameters)
        {
            HashSet<CsharpVariable> ret = new HashSet<CsharpVariable>();
            foreach (var param in parameters)
            {
                // WriteLine($"\t\t{param.Identifier} : {param.Type}");
                CsharpAstNode astNode = AstNode(param);
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
                    EntityHash = astNode.EntityHash
                };
                ret.Add(varibale);
            }
            return ret;
        }

        private HashSet<CsharpVariable> VisitVariableDecl(VariableDeclarationSyntax node)
        {
            HashSet<CsharpVariable> variables = new HashSet<CsharpVariable>();

            foreach (var variable in node.Variables)
            {
                CsharpAstNode astNode = AstNode(variable);
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
                    isLINQ = isLINQvar
                };
                variables.Add(csharpVariable);
            }
            return variables;
        }

        private CsharpVariable VisitPropertyDecl(PropertyDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                IsProperty = true,
                DocumentationCommentXML = Model.GetDeclaredSymbol(node).GetDocumentationCommentXml(),
                EntityHash = astNode.EntityHash
            };
            return variable;
        }

        private HashSet<CsharpMethod> VisitAccessors(AccessorListSyntax node, String propertyName)
        {
            HashSet<CsharpMethod> methods = new HashSet<CsharpMethod>();

            if (node == null) return methods;

            foreach (AccessorDeclarationSyntax accessor in node.Accessors)
            {
                CsharpAstNode astNode = AstNode(accessor);

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
                    IsAccessor = true,
                    Name = propertyName+name+"Accessor",
                    DocumentationCommentXML = Model.GetDeclaredSymbol(accessor).GetDocumentationCommentXml(),
                    EntityHash = astNode.EntityHash
                };

                foreach (VariableDeclarationSyntax variableDeclaration in accessor.DescendantNodes().OfType<VariableDeclarationSyntax>())
                {
                    method.AddLocals(VisitVariableDecl(variableDeclaration));
                }

                methods.Add(method);
            }            

            return methods;
        }

        public override void VisitEnumDeclaration(EnumDeclarationSyntax node)
        {
            //WriteLine($"\n EnumDeclaration visited: {node.Identifier.Text}");
            CsharpAstNode astNode = AstNode(node);
            string qName = "";
            try
            {
                qName = Model.GetDeclaredSymbol(node).ToString();
            }
            catch (Exception)
            {
                WriteLine($"Can not get QualifiedName of this name: {node.Identifier}");
            }

            var nameSpaces = CsharpNamespaces.Where(n => qName.Contains(n.Name)).ToList();
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
                csharpEnum.AddMember(VisitEnumMemberDecl(enumMemberDeclarationSyntax));
            }
            DbContext.CsharpEnums.Add(csharpEnum);
        }

        private CsharpEnumMember VisitEnumMemberDecl(EnumMemberDeclarationSyntax node)
        {
            CsharpAstNode astNode = AstNode(node);
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
                EntityHash = astNode.EntityHash
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
            return csharpEnumMember;
        }

        private void VisitQueryExp(QueryExpressionSyntax node){
            CsharpAstNode astNode = AstNode(node);
        }
    }
}
