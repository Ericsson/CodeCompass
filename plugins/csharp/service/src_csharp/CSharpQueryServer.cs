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
using cc.service.csharp;
using CSharpParser.model;

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

    public class CSharpQueryServer
    {
        private static readonly ILogger Logger = LoggingHelper.CreateLogger<CSharpQueryServer>();
        private static readonly TConfiguration Configuration = new TConfiguration();
        private static int port = 9091;

        /*
        * args[0]: database connection string
        * args[1]: Thrift server port number
        */
        public static void Main(string[] args)
        {
            System.Console.WriteLine("New query server");
            using (var source = new CancellationTokenSource())
            {
                string connenctionString = args[0];
                port = Int32.Parse(args[1]);
                System.Console.WriteLine("[CSharpService] Server started!");
                RunAsync(source.Token, connenctionString).GetAwaiter().GetResult();

                System.Console.WriteLine("[CSharpService] Press any key to stop...");

                Console.ReadLine();
                source.Cancel();
                System.Console.WriteLine("[CSharpService] Server stopped");
            }
        }     

        private static async Task RunAsync(CancellationToken cancellationToken, string connenctionString)
        {
            TServerTransport serverTransport = new TServerSocketTransport(port, Configuration);
            TTransportFactory transportFactory = new TBufferedTransport.Factory();
            TProtocolFactory protocolFactory = new TBinaryProtocol.Factory();

            var handler = new CSharpQueryHandler(connenctionString);
            ITAsyncProcessor processor = new CsharpService.AsyncProcessor(handler);

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

                await server.ServeAsync(cancellationToken);
            }
            catch (Exception x)
            {
                Logger.LogInformation("{x}",x);
            }
        }
    }
}
