package org.yinwang.pysonar;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.yinwang.pysonar.common.Extension;
import org.yinwang.pysonar.common.Modified;
import java.util.StringTokenizer;

public class Options {

    private Map<String, Object> optionsMap = new LinkedHashMap<>();

    /**
     * Separated (still unprocessed) collection of the original command line
     * arguments
     */
    private List<String> args;

    /**
     * Process the arguments.
     *
     * @param args_ original command line arguments
     */
    @Extension
    protected void init(String[] args_) {
        args = new ArrayList(Arrays.asList(args_));

        Set<String> inputFiles = new HashSet<>();
        
        for (int i = 0; i < args_.length; i++) {
            String arg = args_[i];

            //Remove dash(es)
            if (arg.startsWith("-")) {
                arg = (arg.startsWith("--")) ? arg.substring(2) : arg.substring(1);

                switch (arg) {

                    /*case "database":
                    case "d":
                        put("database", getArgValue(arg, ++i));
                        break;*/

                    case "ipcInFd":
                        put("ipcInFd", getArgValue(arg, ++i));
                        break;

                    case "ipcOutFd":
                        put("ipcOutFd", getArgValue(arg, ++i));
                        break;

                    case "debug":
                        put("debug", true);
                        break;

		            case "quiet":
                        put("quiet", true);
                        break;

                    case "report":
                        put("report", true);
                        break;

                    case "deps":
                        put("deps", Arrays.asList(getArgValue(arg, ++i).split(":")));
                        break;

                    case "log": // optional
                        put("loglevel", getArgValue(arg, ++i));
                        break;

                    case "python3":
                        put("python3", true);
                        break;

                    case "python2":
                        put("python2", true);
                        break;

                    default:
                    System.err.println("Unrecognised command line option: " + arg);
                    System.exit(1);
                        // throw new IllegalArgumentException(
                                // "Unrecognised command line option: " + arg);
                }
            } else {
                inputFiles.addAll(tokenizeInput(arg));
            }
        }

        if (get("deps") == null) {
            put("deps", new ArrayList<String>());
        }

        if (!inputFiles.isEmpty()) {
            put("input", inputFiles);
        }
    }

    private Set<String> tokenizeInput(String input){
        String str = input.trim();
        Set<String> ret = new HashSet<>();
        StringTokenizer st = new StringTokenizer(str);
        while(st.hasMoreTokens()){
            ret.add(st.nextToken());
        }
        return ret;
    }

    /**
     * Get the expected value of an option argument if it exits.
     *
     * @param arg needed for error message
     * @param index
     * @return
     */
    @Extension
    private String getArgValue(String arg, int index) {
        try {
            String obj = args.get(index);

            if (obj.startsWith("-")) {
                throw new IllegalArgumentException(
                                "Command line option must have value: " + arg);
            }
            return obj;
        } catch (IndexOutOfBoundsException e) {
            throw new IllegalArgumentException(
                                "Command line option must have value: " + arg);
        }
    }

    @Modified
    public Options(String[] args) {
        init(args);
    }

    public Object get(String key) {
        return optionsMap.get(key);
    }

    public boolean hasOption(String key) {
        Object v = optionsMap.get(key);
        if (v instanceof Boolean) {
            return (boolean) v;
        } else {
            return false;
        }
    }

    public void put(String key, Object value) {
        optionsMap.put(key, value);
    }

    public List<String> getArgs() {
        return args;
    }

    public Map<String, Object> getOptionsMap() {
        return optionsMap;
    }
}
