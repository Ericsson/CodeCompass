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
            int threadNum = 1;
            if (args.Length == 0){
                WriteLine("Missing sourcepath in Csharp parser command-line arguments!");
                return 1;
            } else if (args.Length > 0){
                rootDir = args[0];
            } else if (args.Length > 1){
                bool succes = int.TryParse(args[1], out threadNum);
                if (!succes){
                    WriteLine("Invalid threadnumber argument! Multithreaded parsing disabled!");                    
                }
            }

            IEnumerable<string> allFiles = GetSourceFilesFromDir(rootDir);
            /*
            string programPath = @"/home/borisz/Desktop/Labor/Standalone/files2parse/CentroidBasedClustering.cs"; //dummy for test
            string programText = File.ReadAllText(programPath);
            SyntaxTree tree = CSharpSyntaxTree.ParseText(programText);
            CompilationUnitSyntax root = tree.GetCompilationUnitRoot();
            CSharpCompilation compilation = CSharpCompilation.Create("CSharpCompilation")
                .AddReferences(MetadataReference.CreateFromFile(typeof(object).Assembly.Location))
                .AddSyntaxTrees(tree);

            SemanticModel model = compilation.GetSemanticModel(tree);
            CsharpDbContext dbContext = new CsharpDbContext();
            dbContext.Database.EnsureCreated();
            */

            CsharpDbContext dbContext = new CsharpDbContext();
            dbContext.Database.EnsureCreated();

            IEnumerable<SyntaxTree> trees = new SyntaxTree[]{};
            foreach (string file in allFiles)
            {
                string programText = File.ReadAllText(file);
                SyntaxTree tree = CSharpSyntaxTree.ParseText(programText, null, file);
                trees = trees.Append(tree);
            }
            CSharpCompilation compilation = CSharpCompilation.Create("CSharpCompilation")
                .AddReferences(MetadataReference.CreateFromFile(typeof(object).Assembly.Location))
                .AddSyntaxTrees(trees);

            foreach (SyntaxTree tree in trees)
            {
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(dbContext, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());                
                WriteLine(tree.FilePath);
            }           

            //var visitor = new AstVisitor(dbContext, model, tree);
            //visitor.Visit(root);
            dbContext.SaveChanges();
            return 0;
        }

        public static IEnumerable<string> GetSourceFilesFromDir(string root)
        {
            IEnumerable<string> allFiles = new string[]{};
            // Data structure to hold names of subfolders. 
            //Stack<string> dirs = new Stack<string>(100); //does not work if there are more than x subfolders 
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

                //allFiles = allFiles.Concat(files);

            }

            return allFiles;
        }

    }
}
