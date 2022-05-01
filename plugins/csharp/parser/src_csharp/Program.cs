using static System.Console;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using CSharpParser.model;

namespace CSharpParser
{
    class Program
    {
        
        static int Main(string[] args)
        {
            string rootDir = "";
            string buildDir = "";
            string connenctionString = "";
            int threadNum = 1;
            if (args.Length < 3){
                WriteLine("Missing command-line arguments in CSharpParser!");                              
                return 1;
            } else if (args.Length == 3){
                connenctionString = args[0].Replace("'", "");
                rootDir = args[1].Replace("'", "");
                buildDir = args[2].Replace("'", "");
            } else if (args.Length == 4){
                connenctionString = args[0].Replace("'", "");
                rootDir = args[1].Replace("'", "");
                buildDir = args[2].Replace("'", "");
                bool succes = int.TryParse(args[3], out threadNum);
                if (!succes){
                    WriteLine("Invalid threadnumber argument! Multithreaded parsing disabled!");                    
                }
            } else if (args.Length > 4){
                WriteLine("Too many command-line arguments in CSharpParser!");
                return 1;
            }

            //Converting the connectionstring into entiy framwork style connectionstring
            connenctionString = connenctionString.Substring(connenctionString.IndexOf(':')+1);
            connenctionString = connenctionString.Replace("user", "username");
            string[] properties = connenctionString.Split(';');
            string csharpConnenctionString = "";
            for(int i = 0; i<properties.Length; ++i) {
                if (properties[i].Contains("database=")) {
                    csharpConnenctionString += "Database=codecompass_csharp_db";
                } else {
                    csharpConnenctionString += properties[i].Substring(0,1).ToUpper() 
                        + properties[i].Substring(1);
                }
                if (i<properties.Length-1){
                    csharpConnenctionString += ";";
                }
            }
            //WriteLine($"Converted connectionstring:\n{csharpConnenctionString}");

            CsharpDbContext dbContext = new CsharpDbContext(csharpConnenctionString);
            dbContext.Database.EnsureCreated();


            IEnumerable<string> allFiles = GetSourceFilesFromDir(rootDir, ".cs");
            IEnumerable<string> assemblies = GetSourceFilesFromDir(buildDir, ".dll");

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
            
            foreach (string file in assemblies)
            {
                compilation = compilation.AddReferences(MetadataReference.CreateFromFile(file));
            }
                
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

        public static IEnumerable<string> GetSourceFilesFromDir(string root, string extension)
        {
            //WriteLine("GetSourceFilesFromDir:"+root);
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
                        if (fi.Extension == extension) {
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
