using static System.Console;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.EntityFrameworkCore;
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using CSharpParser.model;
using Microsoft.Build.Locator;
using Microsoft.CodeAnalysis.MSBuild;

namespace CSharpParser
{
    class Program
    {
        //private readonly CsharpDbContext _context;
        private static List<string> _rootDir;
        private static string _buildDir = "";
        private static string _buildDirBase = "";
        private static string _connectionString = "";

        static async Task<int> Main(string[] args)
        {
            MSBuildLocator.RegisterDefaults();
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
            catch (Exception)
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
            string csharpConnectionString = transformConnectionString();

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                            .UseNpgsql(csharpConnectionString)
                            .Options;

            CsharpDbContext _context = new CsharpDbContext(options);
            _context.Database.Migrate();

            if (_rootDir.Count == 0)
            {
                WriteLine("Missing input path argument in CSharpParser!");
                return 1;
            }

            string primaryInput = _rootDir[0];

            if (IsSolutionInput(primaryInput))
            {
                await ParseSolutionPathAsync(primaryInput, csharpConnectionString, threadNum);
                return 0;
            }

            if (IsProjectInput(primaryInput))
            {
                var projectInputs = _rootDir
                    .Where(IsProjectInput)
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToList();

                await ParseProjectsByPathAsync(projectInputs, csharpConnectionString, threadNum);
                return 0;
            }

            if (Directory.Exists(primaryInput))
            {
                await ParseLegacyMode(csharpConnectionString, threadNum);
                return 0;
            }

            if (File.Exists(primaryInput))
            {
                Console.Error.WriteLine($"Unsupported file type: {primaryInput}");
                Console.Error.WriteLine("Supported: .sln, .slnx, .csproj");
                return 1;
            }

            WriteLine("Unsupported input path in CSharpParser!");
            return 1;
        }

        private static bool IsSolutionInput(string inputPath)
        {
            return File.Exists(inputPath)
                && (inputPath.EndsWith(".sln", StringComparison.OrdinalIgnoreCase)
                    || inputPath.EndsWith(".slnx", StringComparison.OrdinalIgnoreCase));
        }

        private static bool IsProjectInput(string inputPath)
        {
            return File.Exists(inputPath)
                && inputPath.EndsWith(".csproj", StringComparison.OrdinalIgnoreCase);
        }

        private static async Task ParseLegacyMode(string csharpConnectionString, int threadNum)
        {
            List<string> allFiles = new List<string>();
            foreach (var p in _rootDir)
            {
                if (!Directory.Exists(p))
                {
                    WriteLine("Skipping non-directory input in legacy mode: " + p);
                    continue;
                }

                Console.WriteLine(p);
                allFiles.AddRange(GetSourceFilesFromDir(p, ".cs"));
            }

            foreach (var f in allFiles)
            {
                WriteLine(f);
            }

            IEnumerable<string> assemblies = GetSourceFilesFromDir(_buildDir, ".dll");
            IEnumerable<string> assemblies_base = assemblies;
            if (Directory.Exists(_buildDirBase))
            {
                assemblies_base = GetSourceFilesFromDir(_buildDirBase, ".dll");
            }

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

            await ParalellRun(csharpConnectionString, threadNum, trees, compilation);
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

        private static async Task ParseSolutionPathAsync(
            string solutionPath,
            string csharpConnectionString,
            int threadCount)
        {
            try
            {
                var solution = await LoadSolutionAsync(solutionPath);
                int documentCount = solution.Projects
                    .SelectMany(p => p.Documents)
                    .Count(d => !string.IsNullOrEmpty(d.FilePath));

                if (solution.Projects.Any() && documentCount > 0)
                {
                    await ParseSolutionAsync(solution, csharpConnectionString, threadCount);
                    return;
                }

                Console.WriteLine(
                    "OpenSolutionAsync returned no usable documents, trying manual project discovery.");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"OpenSolutionAsync failed: {ex.Message}");
                Console.WriteLine("Trying manual project discovery from solution file.");
            }

            var projectPaths = ExtractProjectPathsFromSolution(solutionPath);
            if (projectPaths.Count == 0)
            {
                Console.Error.WriteLine($"No C# projects found in solution: {solutionPath}");
                throw new InvalidOperationException(
                    $"No C# projects found in solution: {solutionPath}");
            }

            await ParseProjectsByPathAsync(projectPaths, csharpConnectionString, threadCount);
        }

        private static List<string> ExtractProjectPathsFromSolution(string solutionPath)
        {
            var projectPaths = new List<string>();
            var solutionDir = Path.GetDirectoryName(solutionPath) ?? string.Empty;

            // Works for both classic .sln rows and .slnx quoted project paths.
            var csprojRegex = new Regex(
                "\"([^\"]+\\.csproj)\"",
                RegexOptions.IgnoreCase | RegexOptions.Compiled);

            IEnumerable<string> solutionLines;
            try
            {
                solutionLines = File.ReadLines(solutionPath);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Failed to read solution file '{solutionPath}': {ex.Message}");
                return projectPaths;
            }

            foreach (var line in solutionLines)
            {
                var matches = csprojRegex.Matches(line);
                foreach (Match match in matches)
                {
                    var relativeProjectPath = match.Groups[1].Value.Trim();
                    if (string.IsNullOrEmpty(relativeProjectPath))
                    {
                        continue;
                    }

                    var normalizedProjectPath = relativeProjectPath
                        .Replace('\\', Path.DirectorySeparatorChar)
                        .Replace('/', Path.DirectorySeparatorChar);

                    var fullProjectPath = Path.GetFullPath(
                        Path.Combine(solutionDir, normalizedProjectPath));

                    if (!File.Exists(fullProjectPath))
                    {
                        Console.Error.WriteLine(
                            $"Project path from solution not found: {fullProjectPath}");
                        continue;
                    }

                    projectPaths.Add(fullProjectPath);
                }
            }

            return projectPaths
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .ToList();
        }

        private static async Task ParseProjectsByPathAsync(
            List<string> projectPaths,
            string csharpConnectionString,
            int threadCount)
        {
            Console.WriteLine($"Fallback project count: {projectPaths.Count}");
            foreach (var projectPath in projectPaths)
            {
                try
                {
                    Console.WriteLine($"Fallback loading project: {projectPath}");
                    using var workspace = CreateMsBuildWorkspace();
                    var project = await workspace.OpenProjectAsync(projectPath);
                    await ParseProjectAsync(
                        project,
                        csharpConnectionString,
                        threadCount);
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine(
                        $"Fallback load failed for '{projectPath}': {ex.Message}");
                }
            }
        }

        private static MSBuildWorkspace CreateMsBuildWorkspace()
        {
            var workspace = MSBuildWorkspace.Create();
            workspace.LoadMetadataForReferencedProjects = true;
            workspace.WorkspaceFailed += (sender, args) =>
            {
                Console.Error.WriteLine($"Workspace warning: {args.Diagnostic.Message}");
            };

            return workspace;
        }

        private static async Task<Solution> LoadInputAsync(string inputPath)
        {
            if (File.Exists(inputPath))
            {
                if (inputPath.EndsWith(".sln", StringComparison.OrdinalIgnoreCase) ||
                    inputPath.EndsWith(".slnx", StringComparison.OrdinalIgnoreCase))
                {
                    return await LoadSolutionAsync(inputPath);
                }
                if (inputPath.EndsWith(".csproj", StringComparison.OrdinalIgnoreCase))
                {
                    return await LoadProjectAsync(inputPath);
                }
                else
                {
                    Console.Error.WriteLine($"Unsupported file type: {inputPath}");
                    Console.Error.WriteLine("Supported: .sln, .slnx, .csproj");
                    return null;
                }
            }
            if (Directory.Exists(inputPath))
            {
                return null;
            }
            else
            {
                Console.Error.WriteLine($"Input path not found: {inputPath}");
                return null;
            }
        }

        private static async Task<Solution> LoadSolutionAsync(string solutionPath)
        {
            Console.WriteLine($"Loading solution: {solutionPath}");
            using var workspace = CreateMsBuildWorkspace();
            
            var solution = await workspace.OpenSolutionAsync(solutionPath);
            Console.WriteLine($"Solution loaded: {solution.FilePath}");
            Console.WriteLine($"Projects found: {solution.Projects.Count()}");
            return solution;
        }

        private static async Task<Solution> LoadProjectAsync(string projectPath)
        {
            Console.WriteLine($"Loading project: {projectPath}");
            using var workspace = CreateMsBuildWorkspace();
            
            var project = await workspace.OpenProjectAsync(projectPath);
            Console.WriteLine($"Project loaded: {project.Name}");

            return project.Solution;
        }

        private static async Task ParseSolutionAsync(
            Solution solution,
            string csharpConnectionString,
            int threadCount)
        {
            Console.WriteLine($"Solution loaded: {solution.FilePath}");
            Console.WriteLine($"Projects found: {solution.Projects.Count()}");

            foreach (var project in solution.Projects)
            {
                await ParseProjectAsync(project, csharpConnectionString, threadCount);
            }
        }

        private static async Task ParseProjectAsync(
            Project project,
            string csharpConnectionString,
            int threadCount)
        {
            Console.WriteLine($"Processing project: {project.Name}");

            var compilation = await project.GetCompilationAsync();

            if (compilation == null)
            {
                Console.WriteLine($"- Compilation failed for {project.Name}");
                return;
            }

            var documents = project.Documents
                .Where(d => !string.IsNullOrEmpty(d.FilePath))
                .ToList();
            Console.WriteLine($"- Documents in {project.Name}: {documents.Count}");

            await ProcessDocumentsInParallel(
                documents,
                compilation,
                csharpConnectionString,
                threadCount,
                project.Name
            );
        }

        private static async Task ProcessDocumentsInParallel(
            List<Document> documents,
            Compilation compilation,
            string csharpConnectionString,
            int threadCount,
            string projectName)
        {
            if (documents.Count == 0)
            {
                return;
            }

            var tasks = new List<Task>();
            int effectiveThreadCount = Math.Max(1, Math.Min(threadCount, documents.Count));
            var documentGroups = documents.Chunk(documents.Count / effectiveThreadCount + 1);

            foreach (var group in documentGroups)
            {
                tasks.Add(Task.Run(async () =>
                {
                    var options = new DbContextOptionsBuilder<CsharpDbContext>()
                        .UseNpgsql(csharpConnectionString)
                        .Options;
                    var localDbContext = new CsharpDbContext(options);

                    foreach (var document in group)
                    {
                        await ProcessSingleDocument(
                            document,
                            compilation,
                            localDbContext,
                            projectName
                        );
                    }

                    await localDbContext.SaveChangesAsync();
                }));
            }
            await Task.WhenAll(tasks);
        }

        private static async Task ProcessSingleDocument(
            Document document,
            Compilation compilation,
            CsharpDbContext dbContext,
            string projectName)
        {
            try
            {
                var syntaxTree = await document.GetSyntaxTreeAsync();
                if (syntaxTree == null) return;
                
                var semanticModel = compilation.GetSemanticModel(syntaxTree);
                
                var visitor = new AstVisitor(
                    dbContext,
                    semanticModel,
                    syntaxTree);
                
                var root = await syntaxTree.GetRootAsync();
                visitor.Visit(root);
                
                string status = visitor.FullyParsed ? "+" : "-";
                Console.WriteLine($"{status}{document.FilePath}");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error parsing {document.FilePath}: {ex.Message}");
                Console.WriteLine($"-{document.FilePath}");
            }
        }
    }
}
