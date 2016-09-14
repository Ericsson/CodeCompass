package org.yinwang.pysonar;

import org.jetbrains.annotations.NotNull;
import org.yinwang.pysonar.common.Modified;

public class Diagnostic {

    public enum Category {

        INFO, WARNING, ERROR
    }

    public String file;
    public Category category;
    public int start;
    public int end;
    public String msg;

    public Diagnostic(String file, Category category, int start, int end, String msg) {
        this.category = category;
        this.file = file;
        this.start = start;
        this.end = end;
        this.msg = msg;
    }

    @Modified
    @NotNull
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[").append(category.toString()).append("]" + " in ").append(file).
                append(" at ").append(start).append(" - ").append(end).append(" :\n");
        sb.append("\t").append(msg);
        return sb.toString();
    }
}
