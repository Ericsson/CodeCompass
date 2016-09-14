package org.yinwang.pysonar.common;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import org.yinwang.pysonar.Analyzer;

public class PPLogger {

    private static ConsoleHandler fileTxt;
    private static SimpleFormatter formatterTxt;

    private final static Logger LOGGER = Logger.getLogger(PPLogger.class.getName());
    private static boolean init = false;

    public PPLogger() {
    }

    private static void init() {
        LOGGER.setUseParentHandlers(false);
        LOGGER.setLevel(getLogLevel());

        fileTxt = new ConsoleHandler();
        formatterTxt = new SimpleFormatter();
        fileTxt.setFormatter(formatterTxt);
        LOGGER.addHandler(fileTxt);
    }

    public static void exception(Exception e) {
        if (init) {
            StackTraceElement[] st = e.getStackTrace();
            StringBuilder sb = new StringBuilder();
            for (StackTraceElement se : st) {
                sb.append(se).append(" \n");
            }
            LOGGER.severe(e.getMessage() + " | \n" + sb.toString());
        } else {
            init();
            init = true;
            exception(e);
        }
    }

    public static void severe(String msg) {
        if (init) {
            LOGGER.severe(msg);
        } else {
            init();
            LOGGER.severe(msg);
            init = true;
        }
    }

    public static void warning(String msg) {
        if (init) {
            LOGGER.warning(msg);
        } else {
            init();
            LOGGER.warning(msg);
            init = true;
        }
    }

    public static void info(String msg) {
        if (init) {
            LOGGER.info(msg);
        } else {
            init();
            LOGGER.info(msg);
            init = true;
        }
    }

    private static Level getLogLevel() {
        String input = (String) Analyzer.self.options.get("loglevel");
        if(input == null){
            input = "default";
        }
        input = input.toLowerCase();
        Level level = null;
        switch (input) {
            case "severe":
                level = Level.SEVERE;
                break;
            case "warning":
                level = Level.WARNING;
                break;
            case "info":
                level = Level.INFO;
                break;
            case "all":
                level = Level.ALL;
                break;
            case "off":
                level = Level.OFF;
                break;
            case "fine":
                level = Level.FINE;
                break;
            case "finer":
                level = Level.FINER;
                break;
            case "finest":
                level = Level.FINEST;
                break;
            case "config":
                level = Level.CONFIG;
                break;
            default:
                level = Level.INFO;
                System.out.println("\nDefault loglevel setting => Level.INFO");
        }
        return level;
    }
}
