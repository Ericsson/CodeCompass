using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Security;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Thrift;
using Thrift.Protocol;
using Thrift.Server;
using Thrift.Transport;
using Thrift.Transport.Server;
using Thrift.Processor;
using System.Diagnostics;
using language;

namespace Server
{
    public static class LoggingHelper
    {
        public static ILoggerFactory LogFactory { get; } = LoggerFactory.Create(builder => {
            ConfigureLogging(builder);
        });

        public static void ConfigureLogging(ILoggingBuilder logging)
        {
            logging.SetMinimumLevel(LogLevel.Trace);
            logging.AddConsole();
            logging.AddDebug();
        }

        public static ILogger<T> CreateLogger<T>() => LogFactory.CreateLogger<T>();
    }

    public class Program
    {
        private static readonly ILogger Logger = LoggingHelper.CreateLogger<Program>();
        private static readonly TConfiguration Configuration = null;  // new TConfiguration() if  needed

        public static void Main(string[] args)
        {
            using (var source = new CancellationTokenSource())
            {
                RunAsync(source.Token).GetAwaiter().GetResult();

                Logger.LogInformation("Press any key to stop...");

                Console.ReadLine();
                source.Cancel();
            }

            Logger.LogInformation("Server stopped");
        }        

        private static async Task RunAsync(CancellationToken cancellationToken)
        {
            TServerTransport serverTransport = new TServerSocketTransport(9090, Configuration);
            TTransportFactory transportFactory = new TBufferedTransport.Factory();
            TProtocolFactory protocolFactory = new TBinaryProtocol.Factory();

            var handler = new ServiceAsyncHandler();
            ITAsyncProcessor processor = new LanguageService.AsyncProcessor(handler);

            try
            {
                var server = new TSimpleAsyncServer(
                    itProcessorFactory: new TSingletonProcessorFactory(processor),
                    serverTransport: serverTransport,
                    inputTransportFactory: transportFactory,
                    outputTransportFactory: transportFactory,
                    inputProtocolFactory: protocolFactory,
                    outputProtocolFactory: protocolFactory,
                    logger: LoggingHelper.CreateLogger<TSimpleAsyncServer >());

                Logger.LogInformation("Starting the server...");

                await server.ServeAsync(cancellationToken);
            }
            catch (Exception x)
            {
                Logger.LogInformation("{x}",x);
            }
        }

        public class ServiceAsyncHandler : LanguageService.IAsync
        {        
            public ServiceAsyncHandler() {}

            /// <summary>
            /// Return the file types which can be used to associate
            /// the file types with the service
            /// @return File types
            /// </summary>
            public async Task<List<string>> getFileTypesAsync(CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<string>());
            }

            /// <summary>
            /// Returns an AstNodeInfo object for the given AST node ID.
            /// @param astNodeId ID of an AST node.
            /// @return The corresponding AstNodeInfo object.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            public async Task<AstNodeInfo> getAstNodeInfoAsync(string astNodeId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new AstNodeInfo());
            }

            /// <summary>
            /// Returns an AstNodeInfo object for the given source code position.
            /// @param fpos File position in the source file.
            /// @return The AstNodeInfo object at the given position. If more AST nodes are
            /// found at the given position nested in each other (e.g. in a compound
            /// expression) then the innermost is returned.
            /// @exception common.InvalidInput Exception is thrown if no AST node found
            /// at the given position.
            /// </summary>
            /// <param name="fpos"></param>
            public async Task<AstNodeInfo> getAstNodeInfoByPositionAsync(FilePosition fpos, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new AstNodeInfo());
            }

            /// <summary>
            /// Returns the source code text that corresponds to the given AST node.
            /// @param astNodeId ID of an AST node.
            /// @return The source text as a verbatim string.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            public async Task<string> getSourceTextAsync(string astNodeId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns the documentation which belongs to the given AST node if any
            /// (Doxygen, Python doc, etc.).
            /// @param astNodeId ID of an AST node.
            /// @return The documentation of the given node.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            public async Task<string> getDocumentationAsync(string astNodeId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns a set of properties which can be known about the given AST node.
            /// @param astNodeId ID of an AST node.
            /// @return A collection which maps the property name to the property value.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeIds"></param>
            public async Task<Dictionary<string, string>> getPropertiesAsync(string astNodeIds, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new Dictionary<string, string>());
            }

            /// <summary>
            /// Returns the diagram types which can be passed to getDiagram() function for
            /// the given AST node.
            /// @param astNodeId ID of an AST node.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            public async Task<Dictionary<string, int>> getDiagramTypesAsync(string astNodeId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new Dictionary<string, int>());
            }

            /// <summary>
            /// Returns the SVG represenation of a diagram about the AST node identified by
            /// astNodeId and diagarm type identified by diagramId.
            /// @param astNodeId The AST node we want to draw diagram about.
            /// @param diagramId The diagram type we want to draw. The diagram types can be
            /// queried by getDiagramTypes().
            /// @return SVG represenation of the diagram. If the diagram can't be generated
            /// then empty string returns.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// @exception common.Timeout Exception is thrown if the diagram generation
            /// times out.
            /// </summary>
            /// <param name="astNodeId"></param>
            /// <param name="diagramId"></param>
            public async Task<string> getDiagramAsync(string astNodeId, int diagramId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns the SVG represenation of the diagram legend used by getDiagram().
            /// @param diagramId The diagram type. This should be one of the IDs returned
            /// by getDiagramTypes().
            /// @return SVG represenation of the diagram legend or empty string if the
            /// legend can't be generated.
            /// </summary>
            /// <param name="diagramId"></param>
            public async Task<string> getDiagramLegendAsync(int diagramId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns a list of diagram types that can be drawn for the specified file.
            /// @param fileId The file ID we would like to draw the diagram about.
            /// @return List of supported diagram types (such as dependency).
            /// @exception common.InvalidId Exception is thrown if no file belongs to the
            /// given ID.
            /// </summary>
            /// <param name="fileId"></param>
            public async Task<Dictionary<string, int>> getFileDiagramTypesAsync(string fileId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new Dictionary<string, int>());
            }

            /// <summary>
            /// Returns an SVG representation of the required diagram graph.
            /// @param fileId The file ID we would like to draw the diagram aboue.
            /// @param diagramId The diagram type we want to draw. These can be queried by
            /// getFileDiagramTypes().
            /// @return SVG represenation of the diagram.
            /// @exception common.InvalidId Exception is thrown if no ID belongs to the
            /// given fileId.
            /// @exception common.Timeout Exception is thrown if the diagram generation
            /// times out.
            /// </summary>
            /// <param name="fileId"></param>
            /// <param name="diagramId"></param>
            public async Task<string> getFileDiagramAsync(string fileId, int diagramId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns the SVG represenation of the diagram legend used by
            /// getFileDiagram().
            /// @param diagramId The diagram type. This should be one of the IDs returned
            /// by getFileDiagramTypes().
            /// @return SVG represenation of the diagram legend or empty string if the
            /// legend can't be generated.
            /// </summary>
            /// <param name="diagramId"></param>
            public async Task<string> getFileDiagramLegendAsync(int diagramId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult("");
            }

            /// <summary>
            /// Returns the reference types which can be passed to getReferences().
            /// @param astNodeId ID of an AST node.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            public async Task<Dictionary<string, int>> getReferenceTypesAsync(string astNodeId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new Dictionary<string, int>());
            }

            /// <summary>
            /// Returns reference count to the AST node identified by astNodeId.
            /// @param astNodeId The AST node to be queried.
            /// @param referenceId Reference type (such as derivedClasses, definition,
            /// usages etc.). Possible values can be queried by getReferenceTypes().
            /// @return Number of rereferences
            /// </summary>
            /// <param name="astNodeId"></param>
            /// <param name="referenceId"></param>
            public async Task<int> getReferenceCountAsync(string astNodeId, int referenceId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(0);
            }

            /// <summary>
            /// Returns references to the AST node identified by astNodeId.
            /// @param astNodeId The AST node to be queried.
            /// @param referenceId Reference type (such as derivedClasses, definition,
            /// usages etc.). Possible values can be queried by getReferenceTypes().
            /// @param tags Meta-information which can help to filter query results of
            /// the AST node (e.g. public, static)
            /// @return List of references.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            /// <param name="referenceId"></param>
            /// <param name="tags"></param>
            public async Task<List<AstNodeInfo>> getReferencesAsync(string astNodeId, int referenceId, List<string> tags, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<AstNodeInfo>());
            }

            /// <summary>
            /// Returns references to the AST node identified by astNodeId restricted to a
            /// given file. Sometimes (e.g. in a GUI) it is sufficient to list only the
            /// results in a file, and this may make the implementation faster.
            /// @param astNodeId The astNode to be queried.
            /// @param referenceId reference type (such as derivedClasses, definition,
            /// usages etc.).
            /// @param fileId ID of the file in which we search for the references.
            /// @param tags Meta-information which can help to filter query results of
            /// the AST node (e.g. public, static)
            /// @return List of references.
            /// @exception common.InvalidId Exception is thrown if not AST node or file
            /// belongs to the given IDs.
            /// </summary>
            /// <param name="astNodeId"></param>
            /// <param name="referenceId"></param>
            /// <param name="fileId"></param>
            /// <param name="tags"></param>
            public async Task<List<AstNodeInfo>> getReferencesInFileAsync(string astNodeId, int referenceId, string fileId, List<string> tags, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<AstNodeInfo>());
            }

            /// <summary>
            /// Same as getReferences() but only a few results are returned based on the
            /// parameters.
            /// @param astNodeId The AST node to be queried.
            /// @param referenceId Reference type (such as derivedClasses, definition,
            /// usages etc.). Possible values can be queried by getReferenceTypes().
            /// @param pageSize The maximum size of the returned list.
            /// @param pageNo The number of the page to display, starting from 0.
            /// @return List of references.
            /// @exception common.InvalidId Exception is thrown if no AST node belongs to
            /// the given ID.
            /// </summary>
            /// <param name="astNodeId"></param>
            /// <param name="referenceId"></param>
            /// <param name="pageSize"></param>
            /// <param name="pageNo"></param>
            public async Task<List<AstNodeInfo>> getReferencesPageAsync(string astNodeId, int referenceId, int pageSize, int pageNo, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<AstNodeInfo>());
            }

            /// <summary>
            /// Returns a list of reference types that can be listed for the requested file
            /// (such as includes, included by, etc.).
            /// @param fileId The file ID we want to get the references about.
            /// @return List of supported reference types.
            /// @exception common.InvalidId Exception is thrown if no file belongs to the
            /// given ID.
            /// </summary>
            /// <param name="fileId"></param>
            public async Task<Dictionary<string, int>> getFileReferenceTypesAsync(string fileId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new Dictionary<string, int>());
            }

            /// <summary>
            /// Returns references as an answer to the requested search.
            /// @param fileId the file ID we want to get the references about.
            /// @param referenceType Reference type (e.g. includes, provides, etc.).
            /// Possible values can be queried by getFileReferenceTypes().
            /// @return List of references.
            /// @exception common.InvalidId Exception is thrown if no file belongs to the
            /// given ID.
            /// </summary>
            /// <param name="fileId"></param>
            /// <param name="referenceId"></param>
            public async Task<List<AstNodeInfo>> getFileReferencesAsync(string fileId, int referenceId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<AstNodeInfo>());
            }

            /// <summary>
            /// Returns reference count to the File node identified by fileId.
            /// @param fileId The file ID we want to get the references count about.
            /// @param referenceId Reference type (such as includes, functions, macros,
            /// files etc.). Possible values can be queried by getFileReferenceTypes().
            /// @return Number of references.
            /// </summary>
            /// <param name="fileId"></param>
            /// <param name="referenceId"></param>
            public async Task<int> getFileReferenceCountAsync(string fileId, int referenceId, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(0);
            }

            /// <summary>
            /// Returns the syntax highlight elements for a whole file.
            /// @param fileRange The range of the file
            /// @return Elements' position and CSS class name.
            /// @exception common.InvalidId Exception is thrown if no file belongs to the
            /// given ID.
            /// </summary>
            /// <param name="range"></param>
            public async Task<List<SyntaxHighlight>> getSyntaxHighlightAsync(FileRange range, CancellationToken cancellationToken = default(CancellationToken)) {
                return await Task.FromResult(new List<SyntaxHighlight>());
            }
     
        }
    }
}
