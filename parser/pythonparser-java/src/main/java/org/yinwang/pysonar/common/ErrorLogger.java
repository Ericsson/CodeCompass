package org.yinwang.pysonar.common;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.Node;

/**
 *
 * @author eadaujv
 */
public class ErrorLogger {

    private HashMap<Node, String> junkNodes;
    private HashMap<Binding, String> junkBindings;
    private LinkedList<String> failedToParse;
    private LinkedList<String> parentErrors;
    private LinkedList<String> classBaseErrors;
    private LinkedList<String> funcParamErrors;
    private LinkedList<String> otherErrors;

    private LinkedList<PythonParserError> pythonParserErrors;

    public ErrorLogger() {
        junkNodes = new HashMap<>();
        junkBindings = new HashMap<>();
        failedToParse = new LinkedList<>();
        parentErrors = new LinkedList<>();
        pythonParserErrors = new LinkedList<>();
        classBaseErrors = new LinkedList<>();
        funcParamErrors = new LinkedList<>();
        otherErrors = new LinkedList<>();
    }

    public void addJunkNode(Node n, String reason) {
        if (!junkNodes.containsKey(n)) {
            junkNodes.put(n, reason);
        }
    }

    /**
     * UNUSED
     */
    public void addJunkBinding(Binding b, String reason) {
        if (!junkBindings.containsKey(b)) {
            junkBindings.put(b, reason);
        }
    }

    public void addFailedToParse(String filePath) {
        failedToParse.add(filePath);
    }

    public void addParentError(String error) {
        parentErrors.add(error);
    }

    public void addClassBaseError(String error) {
        classBaseErrors.add(error);
    }

    public void addFuncParamError(String error) {
        funcParamErrors.add(error);
    }
    
    public void addOtherError(String error) {
        otherErrors.add(error);
    }

    public void logToFile() {
        for (Map.Entry<Node, String> e : junkNodes.entrySet()) {
            Node n = e.getKey();
            pythonParserErrors.add(new PythonParserError(n.start, n.end, n.file, n.getLineStart(),
                    n.getLineEnd(), n.getOffset(), n.getEndOffset(), n.getName(), n.getAstType()));
        }

        StringBuilder sb = new StringBuilder();
        sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        if (!junkNodes.isEmpty()) {
            sb.append("Junk Nodes: \n");
            for (Map.Entry<Node, String> e : junkNodes.entrySet()) {
                sb.append(e.getKey()).append(" - ").append(e.getValue()).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        if (!failedToParse.isEmpty()) {
            sb.append("Failed to parse: \n");
            for (String s : getFailedToParse()) {
                sb.append(s).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        if (!parentErrors.isEmpty()) {
            sb.append("Parent errors: \n");
            for (String s : parentErrors) {
                sb.append(s).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        if (!classBaseErrors.isEmpty()) {
            sb.append("classBase errors: \n");
            for (String s : classBaseErrors) {
                sb.append(s).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        if (!funcParamErrors.isEmpty()) {
            sb.append("funcParam errors: \n");
            for (String s : funcParamErrors) {
                sb.append(s).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        if (!otherErrors.isEmpty()) {
            sb.append("otherErrors errors: \n");
            for (String s : otherErrors) {
                sb.append(s).append("\n");
            }
            sb.append("\n- - - - - - - - - - - - - - - - - - - - - - - \n");
        }
        ErrorLogger.this.logToFile(sb.toString());
    }

    private void logToFile(String content) {
        FileWriter fw = null;
        try {
            File file = new File("python_parser_log.log");
            if (!file.exists()) {
                file.createNewFile();
            }
            fw = new FileWriter(file.getAbsoluteFile());
            BufferedWriter bw = new BufferedWriter(fw);
            bw.write(content);
            bw.close();
        } catch (IOException ex) {
        } finally {
            try {
                fw.close();
            } catch (IOException ex) {
            }
        }
    }

    public LinkedList<PythonParserError> getPythonParserErrors() {
        return pythonParserErrors;
    }

    /**
     * @return the failedToParse
     */
    public LinkedList<String> getFailedToParse() {
        return failedToParse;
    }

}
