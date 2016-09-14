package org.yinwang.pysonar.ast;

import org.jetbrains.annotations.NotNull;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar.types.Type;

public class Str extends Node {

    public String value;

    public Str(@NotNull Object value, String file, int start, int end) {
        super(file, start, end);
        this.value = value.toString();
    }

    @NotNull
    @Override
    public Type transform(State s) {
        return Type.STR;
    }

    @NotNull
    @Override
    public String toString() {
        if (value.length() > 2) {
            if (value.charAt(0) == '\'' || value.charAt(0) == '\"') {
                if ((value.charAt(1) == '\'' || value.charAt(1) == '\"') 
                        && value.length() > 6) {
                    return value.substring(3, value.length() - 3);
                } else {
                    return value.substring(1, value.length() - 1);
                }
            }
        }
        return value;
    }

}
