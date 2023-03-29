using static System.Console;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.EntityFrameworkCore;
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;
using CSharpParser.model;

namespace CSharpParser
{
    class Program
    {
        //private readonly CsharpDbContext _context;
        private static string _rootDir = "";
        private static string _buildDir = "";
        private static string _buildDirBase = "";
        private static string _connectionString = "";

        static int Main(string[] args)
        {
            int threadNum = 4;
            if (args.Length < 3)
            {
                WriteLine("Missing command-line arguments in CSharpParser!");                              
                return 1;
            }
            else if (args.Length == 3)
            {
                _connectionString = args[0].Replace("'", "");
                _rootDir = args[1].Replace("'", "");
                _buildDir = args[2].Replace("'", "");
            }
            else if (args.Length == 4)
            {
                _connectionString = args[0].Replace("'", "");
                _rootDir = args[1].Replace("'", "");
                _buildDir = args[2].Replace("'", "");
                bool success = int.TryParse(args[3], out threadNum);
                if (!success){
                    WriteLine("Invalid threadnumber argument! Multithreaded parsing disabled!");                    
                }
            }
            else if (args.Length == 5)
            {
                _connectionString = args[0].Replace("'", "");
                _rootDir = args[1].Replace("'", "");
                _buildDir = args[2].Replace("'", "");
                _buildDirBase = args[3].Replace("'", "");
                bool success = int.TryParse(args[4], out threadNum);
                if (!success)
                {
                    WriteLine("Invalid threadnumber argument! Multithreaded parsing disabled!");                    
                }            
            }
            else if (args.Length > 5)
            {
                WriteLine("Too many command-line arguments in CSharpParser!");
                return 1;
            }

            //Converting the connectionstring into entiy framwork style connectionstring
            string csharpConnectionString = transformConnectionString();

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                            .UseNpgsql(csharpConnectionString)
                            .Options;

            CsharpDbContext _context = new CsharpDbContext(options);
            _context.Database.Migrate();

            IEnumerable<string> allFiles = GetSourceFilesFromDir(_rootDir, ".cs");
            IEnumerable<string> assemblies = GetSourceFilesFromDir(_buildDir, ".dll");
            IEnumerable<string> assemblies_base = assemblies;
            if (args.Length == 5)
                assemblies_base = GetSourceFilesFromDir(_buildDirBase, ".dll");

            List<SyntaxTree> trees = new List<SyntaxTree>();
            foreach (string file in allFiles)
            {
                string programText = File.ReadAllText(file);
                SyntaxTree tree = CSharpSyntaxTree.ParseText(programText, null, file);
                trees.Add(tree);
            }

            CSharpCompilation compilation = CSharpCompilation.Create("CSharpCompilation")
                .AddReferences(MetadataReference.CreateFromFile(typeof(object).Assembly.Location))
                .AddSyntaxTrees(trees);
            
            foreach (string file in assemblies_base)
            {
                compilation = compilation.AddReferences(MetadataReference.CreateFromFile(file));
            }
            foreach (string file in assemblies)
            {
                compilation = compilation.AddReferences(MetadataReference.CreateFromFile(file));
            }

            var runtask = ParalellRun(csharpConnectionString, threadNum, trees, compilation);
            int ret = runtask.Result;
            
            return 0;
        }

        private static async Task<int> ParalellRun(string csharpConnectionString, int threadNum,
            List<SyntaxTree> trees, CSharpCompilation compilation)
        {
            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                .UseNpgsql(csharpConnectionString)
                .Options;
            CsharpDbContext dbContext = new CsharpDbContext(options);

            var contextList = new List<CsharpDbContext>();
            contextList.Add(dbContext);
            for (int i = 1; i < threadNum; i++)
            {
                CsharpDbContext dbContextInstance = new CsharpDbContext(options);
                contextList.Add(dbContextInstance);
            }

            var ParsingTasks = new List<Task<int>>();
            int maxThread = threadNum < trees.Count() ? threadNum : trees.Count();
            for (int i = 0; i < maxThread; i++)
            {                
                ParsingTasks.Add(ParseTree(contextList[i],trees[i],compilation,i));
            }

            int nextTreeIndex = maxThread;
            while (ParsingTasks.Count > 0)
            {
                var finshedTask = await Task.WhenAny<int>(ParsingTasks);
                int nextContextIndex = await finshedTask;

                ParsingTasks.Remove(finshedTask);
                if (nextTreeIndex < trees.Count)
                {
                    ParsingTasks.Add(ParseTree(contextList[nextContextIndex],
                        trees[nextTreeIndex],compilation,nextContextIndex));
                    ++nextTreeIndex;
                }
            }

            foreach (var ctx in contextList)
            {
                ctx.SaveChanges();
            }

            return 0;
        }

        private static async Task<int> ParseTree(CsharpDbContext context, 
            SyntaxTree tree, CSharpCompilation compilation, int index)
        {
            var ParingTask = Task.Run(() =>
            {
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(context, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());                
                WriteLine((visitor.FullyParsed ? "+" : "-") + tree.FilePath);
                return index;
            });
            return await ParingTask;
        }

        public static IEnumerable<string> GetSourceFilesFromDir(string root, string extension)
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
                        if (fi.Extension == extension) {
                            allFiles = allFiles.Append(file);
                        }
                    }
                    catch (System.IO.FileNotFoundException e)
                    {
                        // If file was deleted by a separate application
                        Console.WriteLine(e.Message);
                    }
                }
            }

            return allFiles;
        }

        private static string transformConnectionString()
        {
            _connectionString = _connectionString.Substring(_connectionString.IndexOf(':')+1);
            _connectionString = _connectionString.Replace("user", "username");
            string [] properties = _connectionString.Split(';');
            string csharpConnectionString = "";
            for (int i = 0; i < properties.Length; ++i)
            {
                csharpConnectionString += properties[i].Substring(0,1).ToUpper()
                    + properties[i].Substring(1);
                if (i < properties.Length-1)
                {
                    csharpConnectionString += ";";
                }
            }

            return csharpConnectionString;
        }
    }
}
