package org.yinwang.pysonar.common;

public class PythonParserError {

    public String file;
    public int lineStart;
    public int lineEnd;
    public int offset;
    public int endOffset;

    public String msg;

    public PythonParserError(int start, int end, String file, int lineStart, int lineEnd,
            int offset, int endOffset, String name, String astType) {
        this.file = file;
        this.lineStart = lineStart;
        this.lineEnd = lineEnd;
        this.offset = offset;
        this.endOffset = endOffset;
        msg = createMsg(start, end, file, lineStart, lineEnd, offset, endOffset, name, astType);
    }
    
    public String createMsg(int start, int end, String file, int lineStart, int lineEnd,
            int offset, int endOffset, String name, String astType){
        return /*file line here*/"||start:" + start + "end:" + end + 
                "name:" + name + "asttype:" + astType;
    }
}
