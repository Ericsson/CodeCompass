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
using System.Xml.Linq;
using System.Text.Json;

namespace CSharpParser
{
    class Program
    {
        private static List<string> _rootDir;
        private static string _buildDirBase = "";
        private static string _connectionString = "";

        static async Task<int> Main(string[] args)
        {
            try
            {
                MSBuildLocator.RegisterDefaults();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Failed to register MSBuild: {ex.Message}");
                return 1;
            }

            _rootDir = new List<string>();
            int threadNum = 4;

            try
            {
                _connectionString = args[0].Replace("'", "");
                _buildDirBase = args[1].Replace("'", ""); // indexes
                threadNum = int.Parse(args[2]);

                for (int i = 3; i < args.Length; ++i)
                {
                    _rootDir.Add(args[i].Replace("'", ""));
                }
            }
            catch (Exception)
            {
                Console.WriteLine("Error in parsing command!");
                return 1;
            }

            if (_rootDir.Count == 0)
            {
                Console.WriteLine("Missing input path argument in CSharpParser!");
                return 1;
            }

            string primaryInput = _rootDir[0];

            // Converting the connectionstring into entity framework style connectionstring
            string csharpConnectionString = transformConnectionString();

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                            .UseNpgsql(csharpConnectionString)
                            .Options;

            await using var _context = new CsharpDbContext(options);
            _context.Database.Migrate();

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
                var solutionFiles = Directory.GetFiles(primaryInput, "*.sln", SearchOption.AllDirectories)
                    .Concat(Directory.GetFiles(primaryInput, "*.slnx", SearchOption.AllDirectories))
                    .ToList();

                if (solutionFiles.Count > 0)
                {
                    Console.WriteLine($"Found solution file in directory: {solutionFiles[0]}");
                    await ParseSolutionPathAsync(solutionFiles[0], csharpConnectionString, threadNum);
                    return 0;
                }

                var projectFiles = Directory.GetFiles(primaryInput, "*.csproj", SearchOption.AllDirectories)
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToList();

                if (projectFiles.Count > 0)
                {
                    Console.WriteLine($"Found {projectFiles.Count} .csproj file(s) in directory.");
                    await ParseProjectsByPathAsync(projectFiles, csharpConnectionString, threadNum);
                    return 0;
                }

                await ParseLegacyMode(csharpConnectionString, threadNum);
                return 0;
            }

            if (File.Exists(primaryInput))
            {
                Console.Error.WriteLine($"Unsupported file type: {primaryInput}");
                Console.Error.WriteLine("Supported: .sln, .slnx, .csproj");
                return 1;
            }

            Console.WriteLine("Unsupported input path in CSharpParser!");
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
            // This dictionary will remember which file belongs to which DLL
            Dictionary<string, string> fileToTargetDll = new Dictionary<string, string>();

            foreach (var p in _rootDir)
            {
                if (!Directory.Exists(p))
                {
                    Console.WriteLine("Skipping non-directory input in legacy mode: " + p);
                    continue;
                }

                Console.WriteLine(p);
                allFiles.AddRange(GetSourceFilesFromDir(p, ".cs"));
            }
            allFiles = allFiles.Distinct().ToList();

            foreach (var f in allFiles)
            {
                Console.WriteLine(f);
            }

            IEnumerable<string> assemblies_base = Directory.Exists(_buildDirBase)
                ? GetSourceFilesFromDir(_buildDirBase, ".dll")
                : Enumerable.Empty<string>();

            List<string> assemblies = new List<string>();
            foreach (var p in _rootDir)
            {
                if (Directory.Exists(p))
                {
                    // We search for all .dll files in all input directories
                    assemblies.AddRange(GetSourceFilesFromDir(p, ".dll"));
                }
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
            Console.Write(trees.Count);

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

            await ParalellRun(csharpConnectionString, threadNum, trees, compilation, fileToTargetDll);
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
            Console.WriteLine(threadNum);
            for (int i = 0; i < maxThread; i++)
            {
                ParsingTasks.Add(ParseTree(contextList[i], trees[i], compilation, i, fileToTargetDll));
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
                        trees[nextTreeIndex], compilation, nextContextIndex, fileToTargetDll));
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
                Console.WriteLine("ParallelRun " + tree.FilePath);
                SemanticModel model = compilation.GetSemanticModel(tree);
                var visitor = new AstVisitor(context, model, tree);
                visitor.Visit(tree.GetCompilationUnitRoot());

                context.SaveChanges();
                context.ChangeTracker.Clear();

                string target = fileToTargetDll.ContainsKey(tree.FilePath) ? fileToTargetDll[tree.FilePath] : "";

                var resultData = new
                {
                    fullyParsed = visitor.FullyParsed,
                    filePath = tree.FilePath,
                    targetDll = target
                };

                Console.WriteLine("JSON_READY:" + JsonSerializer.Serialize(resultData));

                return index;
            });
            return await ParsingTask;
        }

        public static IEnumerable<string> GetSourceFilesFromDir(string root, string extension)
        {
            IEnumerable<string> allFiles = new string[] { };
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
                    Console.WriteLine(e.Message);
                    continue;
                }
                catch (System.IO.DirectoryNotFoundException e)
                {
                    Console.WriteLine(e.Message);
                    continue;
                }

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
                        if (fi.Extension == extension)
                        {
                            allFiles = allFiles.Append(file);
                        }
                    }
                    catch (System.IO.FileNotFoundException e)
                    {
                        Console.WriteLine(e.Message);
                    }
                }
            }

            return allFiles;
        }

        private static string transformConnectionString()
        {
            _connectionString = _connectionString.Substring(_connectionString.IndexOf(':') + 1);
            _connectionString = _connectionString.Replace("user", "username");
            string[] properties = _connectionString.Split(';');
            string csharpConnectionString = "";
            for (int i = 0; i < properties.Length; ++i)
            {
                csharpConnectionString += properties[i].Substring(0, 1).ToUpper()
                    + properties[i].Substring(1);
                if (i < properties.Length - 1)
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

        private static async Task<Solution> LoadSolutionAsync(string solutionPath)
        {
            Console.WriteLine($"Loading solution: {solutionPath}");
            using var workspace = CreateMsBuildWorkspace();

            var solution = await workspace.OpenSolutionAsync(solutionPath);
            Console.WriteLine($"Solution loaded: {solution.FilePath}");
            Console.WriteLine($"Projects found: {solution.Projects.Count()}");
            return solution;
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
                Console.WriteLine($"Compilation failed for {project.Name}");
                return;
            }

            var documents = project.Documents
                .Where(d => !string.IsNullOrEmpty(d.FilePath))
                .ToList();
            Console.WriteLine($"Documents in {project.Name}: {documents.Count}");

            string targetDll = project.OutputFilePath ?? $"{project.Name}.dll";

            await ProcessDocumentsInParallel(
                documents,
                compilation,
                csharpConnectionString,
                threadCount,
                project.Name,
                targetDll
            );
        }

        private static async Task ProcessDocumentsInParallel(
            List<Document> documents,
            Compilation compilation,
            string csharpConnectionString,
            int threadCount,
            string projectName,
            string targetDll)
        {
            if (documents.Count == 0)
            {
                return;
            }

            var tasks = new List<Task>();
            int effectiveThreadCount = Math.Max(1, Math.Min(threadCount, documents.Count));
            var documentGroups = documents.Chunk(documents.Count / effectiveThreadCount + 1);

            var options = new DbContextOptionsBuilder<CsharpDbContext>()
                .UseNpgsql(csharpConnectionString)
                .Options;

            foreach (var group in documentGroups)
            {
                tasks.Add(Task.Run(async () =>
                {
                    await using var localDbContext = new CsharpDbContext(options);

                    foreach (var document in group)
                    {
                        await ProcessSingleDocument(
                            document,
                            compilation,
                            localDbContext,
                            projectName,
                            targetDll
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
            string projectName,
            string targetDll)
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

                Console.WriteLine($"[{projectName}] {document.FilePath}");

                var resultData = new
                {
                    fullyParsed = visitor.FullyParsed,
                    filePath = document.FilePath,
                    targetDll = targetDll
                };

                Console.WriteLine("JSON_READY:" + JsonSerializer.Serialize(resultData));
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error parsing {document.FilePath}: {ex.Message}");
                Console.WriteLine($"[{projectName}] {document.FilePath}");

                var resultData = new
                {
                    fullyParsed = false,
                    filePath = document.FilePath ?? "",
                    targetDll = targetDll
                };

                Console.WriteLine("JSON_READY:" + JsonSerializer.Serialize(resultData));
            }
        }
    }
}