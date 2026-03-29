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
using System.Xml.Linq;

namespace CSharpParser
{
    class Program
    {
        private static List<string> _rootDir;
        private static string _buildDirBase = "";
        private static string _connectionString = "";

        static int Main(string[] args)
        {
            _rootDir = new List<string>();
            int threadNum = 4;

            try
            {
                _connectionString = args[0].Replace("'", "");
                _buildDirBase = args[1].Replace("'", ""); //indexes
                threadNum = int.Parse(args[2]);

                for (int i = 3; i < args.Length; ++i)
                {
                    _rootDir.Add(args[i].Replace("'", ""));
                }
            }
            catch (Exception e)
            {
                WriteLine("Error in parsing command!");
                return 1;
            }

            //Converting the connectionstring into entiy framwork style connectionstring
            string csharpConnectionString = transformConnectionString();

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                            .UseNpgsql(csharpConnectionString)
                            .Options;

            CsharpDbContext _context = new CsharpDbContext(options);
            _context.Database.Migrate();


            List<string> allFiles = new List<string>();
            // This dictionary will remember which file belongs to which DLL
            Dictionary<string, string> fileToTargetDll = new Dictionary<string, string>();

            foreach (var p in _rootDir)
            {
                // We find all .csproj files
                var csprojFiles = Directory.GetFiles(p, "*.csproj", SearchOption.AllDirectories);
                foreach (var csproj in csprojFiles)
                {
                    string projectDir = Path.GetDirectoryName(csproj);
                    // Default DLL name based on project file name
                    string targetDll = Path.GetFileNameWithoutExtension(csproj) + ".dll";

                    // we try to read the real AssemblyName from the XML
                    try {
                        XDocument doc = XDocument.Load(csproj);
                        var assemblyNameNode = doc.Descendants("AssemblyName").FirstOrDefault();
                        if (assemblyNameNode != null && !string.IsNullOrWhiteSpace(assemblyNameNode.Value))
                        {
                            targetDll = assemblyNameNode.Value + ".dll";
                        }
                    } catch { /* If we cannot read the XML, the default name will remain.*/ }

                    // search for C# files belonging to the project (filtering out the garbage)
                    var csFiles = Directory.GetFiles(projectDir, "*.cs", SearchOption.AllDirectories)
                        .Where(f => !f.Contains("/obj/") && !f.Contains("\\obj\\") && 
                                    !f.Contains("/bin/") && !f.Contains("\\bin\\"));

                    foreach (var cs in csFiles)
                    {
                        // if a file is not already in it (to avoid duplication)
                        if (!fileToTargetDll.ContainsKey(cs))
                        {
                            fileToTargetDll[cs] = targetDll;
                            allFiles.Add(cs);
                        }
                    }
                }
            }
            allFiles = allFiles.Distinct().ToList();

            foreach (var f in allFiles)
            {
                WriteLine(f);
            }


            IEnumerable<string> assemblies_base = GetSourceFilesFromDir(_buildDirBase, ".dll"); //loading basic dlls
            
            List<string> assemblies = new List<string>();
            foreach (var p in _rootDir)
            {
                // We search for all .dll files in all input directories
                assemblies.AddRange(GetSourceFilesFromDir(p, ".dll")); 
            }
            // Let's keep only one of each DLL based on the file name!
            assemblies = assemblies.GroupBy(x => System.IO.Path.GetFileName(x))
                                   .Select(g => g.First())
                                   .ToList();

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

            var runtask = ParalellRun(csharpConnectionString, threadNum, trees, compilation, fileToTargetDll);
            int ret = runtask.Result;
            
            return 0;
        }

        private static async Task<int> ParalellRun(string csharpConnectionString, int threadNum,
            List<SyntaxTree> trees, CSharpCompilation compilation, Dictionary<string, string> fileToTargetDll)
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
                ParsingTasks.Add(ParseTree(contextList[i],trees[i],compilation,i,fileToTargetDll));
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
                        trees[nextTreeIndex],compilation,nextContextIndex, fileToTargetDll));
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
            SyntaxTree tree, CSharpCompilation compilation, int index, 
            Dictionary<string, string> fileToTargetDll)
        {
            var ParsingTask = Task.Run(() =>
            {
                WriteLine("ParallelRun " + tree.FilePath);
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(context, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());      

                // Find the DLL name and append a | to the filename.
                string target = fileToTargetDll.ContainsKey(tree.FilePath) ? fileToTargetDll[tree.FilePath] : "Unknown.dll";
                WriteLine((visitor.FullyParsed ? "+" : "-") + tree.FilePath + "|" + target);

                return index;
            });
            return await ParsingTask;
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
