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
using tutorial;
using shared;
using Thrift.Processor;
using System.Diagnostics;

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
            args ??= Array.Empty<string>();

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
            ITAsyncProcessor processor = new Calculator.AsyncProcessor(handler);

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

        public class ServiceAsyncHandler 
        {        
            public CalculatorAsyncHandler() {}

           
        }
    }
}
