package org.yinwang.pysonar;

import cc.parser.PythonPersisterService;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.Map;
import java.util.Set;
import org.yinwang.pysonar.Options;
import org.yinwang.pysonar._;
import org.yinwang.pysonar.db.DataPersister;
import org.apache.thrift.TException;
import org.apache.thrift.TProcessor;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolFactory;
import org.apache.thrift.transport.TIOStreamTransport;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;


/**
 * Python parser extension based on Pysonar2 for CodeCompass project.
 */
public class PythonParser {

    private static Analyzer analyzer;

    public static void main(String[] args) {
        Options options = new Options(args);

        preparations(options.getOptionsMap());

        try {
          PythonPersisterService.Iface persister = createPersister(options);
          try {
            start(options.getOptionsMap(), persister);
          } finally {
            persister.stop();
          }
        } catch (Exception ex) {
          ex.printStackTrace();
          System.err.println("Exception: " + ex.toString());
        }
    }

    /**
     * Star parsing and writing to the database.
     *
     * @param fileOrDir input file or dir for parsing
     * @param outputDb output sqlite database
     * @param options behavior options
     */
    private static void start(Map<String, Object> options, PythonPersisterService.Iface persister) {
        Set<String> inputPaths = (Set) options.get("input");

        _.msg("Loading and analyzing files");
        analyzer = new Analyzer(options, persister);
        analyzer.analyze(inputPaths);
        analyzer.finish();

        _.msg("Saving parse information into database.");

        DataPersister dp = new DataPersister(persister);

        analyzer.close();
        deleteCacheDir();
    }

    /**
     * Preparation command before start parsing and persisting.
     */
    private static void preparations(Map<String, Object> options) {
        if (!options.containsKey("input")) {
            throw new IllegalStateException("There is no input directory or file given.");
        }
        /*if (!options.containsKey("database")) {
            _.die("There is no output database name given.");
        }*/
        if (!options.containsKey("ipcInFd")) {
          throw new IllegalStateException("There is no input file descriptor specified!");
        }
        if (!options.containsKey("ipcOutFd")) {
          throw new IllegalStateException("There is no output file descriptor specified!");
        }
    }

    private static void deleteCacheDir() {
        String temp = _.locateTmp("");
        if(temp == null) return;
        File f = new File(temp);
        if (f.exists()) {
            _.deleteDirectory(f);
        }
    }

    private static PythonPersisterService.Iface createPersister(
      Options options) throws FileNotFoundException {

      TProtocolFactory factory = new TBinaryProtocol.Factory();
      TTransport inTransport = new TIOStreamTransport(
        new FileInputStream("/proc/self/fd/" + options.get("ipcInFd")));
      TTransport outTransport = new TIOStreamTransport(
        new FileOutputStream("/proc/self/fd/" + options.get("ipcOutFd")));

      TProtocol inProtocol = factory.getProtocol(inTransport);
      TProtocol outProtocol = factory.getProtocol(outTransport);

      return new PythonPersisterService.Client(inProtocol, outProtocol);
    }
}
