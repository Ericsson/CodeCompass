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
        private static List<string> _rootDir;
        private static string _buildDir = "";
        private static string _buildDirBase = "";
        private static string _connectionString = "";

        static int Main(string[] args)
        {
            _rootDir = new List<string>();
            int threadNum = 4;

            try
            {
                _connectionString = args[0].Replace("'", "");
                _buildDir = args[1].Replace("'", "");
                _buildDirBase = args[2].Replace("'", "");
                threadNum = int.Parse(args[3]);

                for (int i = 4; i < args.Length; ++i)
                {
                    _rootDir.Add(args[i].Replace("'", ""));
                }
            }
            catch (Exception e)
            {
                WriteLine("Error in parsing command!");
                return 1;
            }
            /*if (args.Length < 3)
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
            }*/

            //Converting the connectionstring into entiy framwork style connectionstring
            string csharpConnectionString = ProgramHelper.transformConnectionString(_connectionString);

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                            .UseNpgsql(csharpConnectionString)
                            .Options;

            CsharpDbContext _context = new CsharpDbContext(options);
            _context.Database.Migrate();

            List<string> allFiles = new List<string>();
            foreach (var p in _rootDir)
            {
                Console.WriteLine(p);
                allFiles.AddRange(ProgramHelper.GetSourceFilesFromDir(p, ".cs"));
            }

            foreach (var f in allFiles)
            {
                WriteLine(f);
            }
            IEnumerable<string> assemblies = ProgramHelper.GetSourceFilesFromDir(_buildDir, ".dll");
            IEnumerable<string> assemblies_base = assemblies;
            if (args.Length == 5)
                assemblies_base = ProgramHelper.GetSourceFilesFromDir(_buildDirBase, ".dll");

            List<SyntaxTree> trees = new List<SyntaxTree>();
            foreach (string file in allFiles)
            {
                string programText = File.ReadAllText(file);
                SyntaxTree tree = CSharpSyntaxTree.ParseText(programText, null, file);
                trees.Add(tree);
            }
            Write(trees.Count);

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
            WriteLine(threadNum);
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
            var ParsingTask = Task.Run(() =>
            {
                WriteLine("ParallelRun " + tree.FilePath);
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(context, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());                
                WriteLine((visitor.FullyParsed ? "+" : "-") + tree.FilePath);
                return index;
            });
            return await ParsingTask;
        }   
    }
}
