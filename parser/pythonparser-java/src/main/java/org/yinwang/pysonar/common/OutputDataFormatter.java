package org.yinwang.pysonar.common;

import org.yinwang.pysonar.types.Type;
import org.yinwang.pysonar.types.UnionType;

public class OutputDataFormatter {

    public String convertStringToHtml(String text) {
        return "<html><p>" + text.replaceAll("\n", "<br>") + "</p></html>";
    }

    public String typeToString(Type type) {
        String str = type.toString();
        return (type instanceof UnionType && ((UnionType) type).types.size() == 1)
                ? str.substring(1, str.length() - 1)
                : str;
    }
}
