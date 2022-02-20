using static System.Console;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using StandAloneCSharpParser.model;

namespace StandAloneCSharpParser
{
    class Program
    {
        
        static int Main(string[] args)
        {
            string rootDir = "";
            string connenctionString = "";
            int threadNum = 1;
            if (args.Length < 2){
                WriteLine("Missing command-line arguments in CSharpParser!");                
                return 1;
            } else if (args.Length == 2){
                connenctionString = args[0];
                rootDir = args[1];
            } else if (args.Length == 3){
                connenctionString = args[0];
                rootDir = args[1];
                bool succes = int.TryParse(args[2], out threadNum);
                if (!succes){
                    WriteLine("Invalid threadnumber argument! Multithreaded parsing disabled!");                    
                }
            } else if (args.Length > 3){
                WriteLine("Too many command-line arguments in CSharpParser!");
                return 1;
            }

            //Converting the connectionstring into entiy framwork style connectionstring
            connenctionString = connenctionString.Substring(connenctionString.IndexOf(':')+1);
            connenctionString = connenctionString.Substring(0,1).ToUpper() + connenctionString.Substring(1);  
            connenctionString = connenctionString.Replace("user", "username");
            char[] connStr = connenctionString.ToCharArray();          
            for(int i = 0; i < connenctionString.Length; ++i){
                if (i > 0 && connStr[i]== ';' && i + 1 < connStr.Length){
                    connStr[i+1] = char.ToUpper(connStr[i+1]);
                }
            }
            connenctionString = new string(connStr);
            //WriteLine($"Converted connectionstring:\n{connenctionString}");

            CsharpDbContext dbContext = new CsharpDbContext(connenctionString);
            dbContext.Database.EnsureCreated();

            IEnumerable<string> allFiles = GetSourceFilesFromDir(rootDir);

            IEnumerable<SyntaxTree> trees = new SyntaxTree[]{};
            foreach (string file in allFiles)
            {
                string programText = File.ReadAllText(file);
                SyntaxTree tree = CSharpSyntaxTree.ParseText(programText, null, file);
                trees = trees.Append(tree);
            }
            CSharpCompilation compilation = CSharpCompilation.Create("CSharpCompilation")
                .AddReferences(MetadataReference.CreateFromFile(typeof(object).Assembly.Location))
                .AddSyntaxTrees(trees)
                .WithOptions(new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary).WithUsings(new []{"Microsoft.EntityFrameworkCore"}));

            foreach (SyntaxTree tree in trees)
            {
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(dbContext, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());                
                WriteLine(tree.FilePath);
            }           

            dbContext.SaveChanges();
            return 0;
        }

        public static IEnumerable<string> GetSourceFilesFromDir(string root)
        {
            IEnumerable<string> allFiles = new string[]{};
            // Data structure to hold names of subfolders. 
            ArrayList dirs = new ArrayList();

            if (!System.IO.Directory.Exists(root))
            {
                throw new ArgumentException();
            }
            dirs.Add(root);

            while (dirs.Count > 0)
            {
                string currentDir = dirs[0].ToString();
                dirs.RemoveAt(0);
                string[] subDirs;
                try
                {
                    subDirs = System.IO.Directory.GetDirectories(currentDir);
                }
                catch (UnauthorizedAccessException e)
                {
                    WriteLine(e.Message);
                    continue;
                }
                catch (System.IO.DirectoryNotFoundException e)
                {
                    WriteLine(e.Message);
                    continue;
                }

                // Add the subdirectories for traversal.
                dirs.AddRange(subDirs);

                string[] files = null;
                try
                {
                    files = System.IO.Directory.GetFiles(currentDir);
                }
                catch (UnauthorizedAccessException e)
                {
                    Console.WriteLine(e.Message);
                    continue;
                }
                catch (System.IO.DirectoryNotFoundException e)
                {
                    Console.WriteLine(e.Message);
                    continue;
                }

                foreach (string file in files)
                {
                    try
                    {
                        System.IO.FileInfo fi = new System.IO.FileInfo(file);
                        if (fi.Extension == ".cs") {
                            allFiles = allFiles.Append(file);
                        }
                    }
                    catch (System.IO.FileNotFoundException e)
                    {
                        // If file was deleted by a separate application
                        Console.WriteLine(e.Message);
                        continue;
                    }
                }
            }

            return allFiles;
        }

    }
}
